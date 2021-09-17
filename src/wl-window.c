/*
 * Copyright (C) 2021   Steffen Nuessle
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include <xdg-shell-client.h>

#include "timer.h"
#include "window.h"
#include "xkb.h"

#include "util/die.h"
#include "util/errstr.h"
#include "util/io-util.h"
#include "util/macro.h"
#include "util/string-util.h"

#ifdef CONFIG_USE_WAYLAND

#define WL_WINDOW_BYTES_PER_PIXEL 4

static void window_xdg_toplevel_surface_configure(void *data,
                                                  struct xdg_toplevel *toplevel,
                                                  int32_t width,
                                                  int32_t height,
                                                  struct wl_array *states)
{
    struct window *win = data;

    (void) win;
    (void) toplevel;
    (void) width;
    (void) height;
    (void) states;
}

static void window_xdg_toplevel_surface_close(void *data,
                                              struct xdg_toplevel *toplevel)
{
    struct window *win = data;

    (void) win;
    (void) toplevel;
}

static const struct xdg_toplevel_listener xdg_toplevel_callbacks = {
    .configure = &window_xdg_toplevel_surface_configure,
    .close = &window_xdg_toplevel_surface_close,
};

static void window_xdg_surface_configure(void *data,
                                         struct xdg_surface *surface,
                                         uint32_t serial)
{
    struct window *win = data;

    (void) surface;

    xdg_surface_ack_configure(win->xdg_surface, serial);
}

static const struct xdg_surface_listener xdg_surface_callbacks = {
    .configure = &window_xdg_surface_configure,
};

static void
window_xdg_wm_ping(void *data, struct xdg_wm_base *xdg_wm, uint32_t serial)
{
    struct window *win = data;

    (void) xdg_wm;

    xdg_wm_base_pong(win->xdg_wm, serial);
}

static const struct xdg_wm_base_listener xdg_wm_callbacks = {
    .ping = &window_xdg_wm_ping,
};

struct window_bind_callback {
    const char *interface;
    void (*function)(struct window *, uint32_t, uint32_t);
};

static void window_output_geometry(void *data,
                                   struct wl_output *output,
                                   int32_t x,
                                   int32_t y,
                                   int32_t physical_width,
                                   int32_t physical_height,
                                   int32_t subpixel,
                                   const char *maker,
                                   const char *model,
                                   int32_t transform)
{
    struct window *win = data;

    (void) win;
    (void) output;
    (void) x;
    (void) y;
    (void) physical_width;
    (void) physical_height;
    (void) subpixel;
    (void) maker;
    (void) model;
    (void) transform;
}

static void window_output_mode(void *data,
                               struct wl_output *output,
                               uint32_t flags,
                               int32_t width,
                               int32_t height,
                               int32_t refresh)
{
    struct window *win = data;

    (void) output;
    (void) flags;
    (void) refresh;

    win->width = width;
    win->height = height;
}

static void window_output_done(void *data, struct wl_output *output)
{
    struct window *win = data;

    (void) win;
    (void) output;
}

static void
window_output_scale(void *data, struct wl_output *output, int32_t factor)
{
    struct window *win = data;

    (void) win;
    (void) output;
    (void) factor;
}

static const struct wl_output_listener window_output_callbacks = {
    .geometry = &window_output_geometry,
    .mode = &window_output_mode,
    .done = &window_output_done,
    .scale = &window_output_scale,
};

static void
window_bind_compositor(struct window *win, uint32_t name, uint32_t version)
{
    if (win->compositor)
        return;

    win->compositor = wl_registry_bind(win->registry,
                                       name,
                                       &wl_compositor_interface,
                                       version);
    if (!win->compositor)
        die("failed to create binding\n");
}

static void
window_bind_subcompositor(struct window *win, uint32_t name, uint32_t version)
{
    (void) win;
    (void) name;
    (void) version;
}

static void
window_bind_shell(struct window *win, uint32_t name, uint32_t version)
{
    if (win->shell)
        return;

    win->shell =
        wl_registry_bind(win->registry, name, &wl_shell_interface, version);

    if (!win->shell)
        die("failed to create binding\n");
}

static void window_bind_shm(struct window *win, uint32_t name, uint32_t version)
{
    if (win->shm)
        return;

    win->shm =
        wl_registry_bind(win->registry, name, &wl_shm_interface, version);

    if (!win->shm)
        die("failed to create binding\n");
}

static void window_keyboard_keymap(void *data,
                                   struct wl_keyboard *keyboard,
                                   uint32_t format,
                                   int fd,
                                   uint32_t size)
{
    struct window *win = data;
    void *mem;
    int err;

    (void) keyboard;

    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)
        goto out;

    mem = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (unlikely(mem == MAP_FAILED))
        die("failed to memory map received keymap data: %s\n", errstr(errno));

    err = xkb_set_keymap(&win->xkb, mem);
    if (unlikely(err < 0))
        die("failed to set received keymap data\n");

    munmap(mem, size);

out:
    close(fd);
}

static void window_keyboard_enter(void *data,
                                  struct wl_keyboard *keyboard,
                                  uint32_t serial,
                                  struct wl_surface *surface,
                                  struct wl_array *keys)
{
    struct window *win = data;

    (void) win;
    (void) keyboard;
    (void) serial;
    (void) surface;
    (void) keys;
}

static void window_keyboard_leave(void *data,
                                  struct wl_keyboard *keyboard,
                                  uint32_t serial,
                                  struct wl_surface *surface)
{
    struct window *win = data;

    (void) win;
    (void) keyboard;
    (void) serial;
    (void) surface;
}

static void window_keyboard_key(void *data,
                                struct wl_keyboard *keyboard,
                                uint32_t serial,
                                uint32_t time,
                                uint32_t key,
                                uint32_t state)
{
    struct window *win = data;
    struct itimerspec ts;
    struct key_event ev;
    xkb_keysym_t symbol;

    (void) keyboard;
    (void) serial;
    (void) time;

    if (!xkb_ready(&win->xkb))
        return;

    if (!win->buffer)
        return;

    symbol = xkb_get_sym(&win->xkb, key);

    if (xkb_keysym_is_modifier(symbol))
        return;

    switch (state) {
    case WL_KEYBOARD_KEY_STATE_PRESSED:

        ts.it_interval.tv_sec = 0;
        ts.it_interval.tv_nsec = win->rate;
        ts.it_value.tv_sec = 0;
        ts.it_value.tv_nsec = win->delay;

        ev.symbol = symbol;
        ev.ctrl = xkb_mod_active(&win->xkb, XKB_MOD_NAME_CTRL);
        ev.shift = xkb_mod_active(&win->xkb, XKB_MOD_NAME_SHIFT);
        ev.mod1 = xkb_mod_active(&win->xkb, XKB_MOD_NAME_ALT);

        win->active = widget_do_key_event(&win->widget, ev);

        win->symbol = symbol;
        break;
    case WL_KEYBOARD_KEY_STATE_RELEASED:

        ts.it_interval.tv_sec = 0;
        ts.it_interval.tv_nsec = 0;
        ts.it_value.tv_sec = 0;
        ts.it_value.tv_nsec = 0;

        (void) timerfd_settime(win->timer_fd, CLOCK_MONOTONIC, &ts, NULL);

        break;
    default:
        break;
    }

    widget_draw(&win->widget);
    wl_surface_attach(win->wl_surface, win->buffer, 0, 0);
    wl_surface_damage_buffer(win->wl_surface, 0, 0, win->width, win->height);
    wl_surface_commit(win->wl_surface);
}

static void window_keyboard_modifiers(void *data,
                                      struct wl_keyboard *keyboard,
                                      uint32_t serial,
                                      uint32_t mods_depressed,
                                      uint32_t mods_latched,
                                      uint32_t mods_locked,
                                      uint32_t group)
{
    struct window *win = data;

    (void) keyboard;
    (void) serial;

    xkb_state_update(&win->xkb,
                     mods_depressed,
                     mods_latched,
                     mods_locked,
                     group);
}

static void window_keyboard_repeat_info(void *data,
                                        struct wl_keyboard *keyboard,
                                        int32_t rate,
                                        int32_t delay)
{
    struct window *win = data;

    (void) keyboard;

    /* Convert to nanoseconds for easy use with the timerspec objects */
    win->rate = 1000 * 1000 * 1000 / rate;
    win->delay = 1000 * 1000 * delay;
}

static const struct wl_keyboard_listener window_keyboard_callbacks = {
    .keymap = &window_keyboard_keymap,
    .enter = &window_keyboard_enter,
    .leave = &window_keyboard_leave,
    .key = &window_keyboard_key,
    .modifiers = &window_keyboard_modifiers,
    .repeat_info = &window_keyboard_repeat_info,
};

static void
window_bind_seat(struct window *win, uint32_t name, uint32_t version)
{
    if (win->seat)
        return;

    win->seat =
        wl_registry_bind(win->registry, name, &wl_seat_interface, version);

    if (!win->seat)
        die("failed to create binding\n");

    win->keyboard = wl_seat_get_keyboard(win->seat);
    if (!win->keyboard)
        die("failed to initialize keyboard\n");

    wl_keyboard_add_listener(win->keyboard, &window_keyboard_callbacks, win);
}

static void
window_bind_output(struct window *win, uint32_t name, uint32_t version)
{
    if (win->output)
        return;

    win->output =
        wl_registry_bind(win->registry, name, &wl_output_interface, version);

    if (!win->output)
        die("failed to create binding\n");

    wl_output_add_listener(win->output, &window_output_callbacks, win);
}

static void
window_bind_xdg_wm(struct window *win, uint32_t name, uint32_t version)
{
    if (win->xdg_wm)
        return;

    win->xdg_wm =
        wl_registry_bind(win->registry, name, &xdg_wm_base_interface, version);

    if (!win->xdg_wm)
        die("failed to create binding");
}

static const struct window_bind_callback window_bind_callbacks[] = {
    {"wl_compositor", &window_bind_compositor},
    {"wl_output", &window_bind_output},
    {"wl_seat", &window_bind_seat},
    {"wl_shell", &window_bind_shell},
    {"wl_shm", &window_bind_shm},
    {"wl_subcompositor", &window_bind_subcompositor},
    {"xdg_wm_base", &window_bind_xdg_wm},
};

static void window_registry_add(void *data,
                                struct wl_registry *registry,
                                uint32_t name,
                                const char *interface,
                                uint32_t version)
{
    struct window *win = data;
    const struct window_bind_callback *begin, *end;

    (void) registry;

    begin = window_bind_callbacks;
    end = begin + ARRAY_SIZE(window_bind_callbacks);

    while (begin < end) {
        const struct window_bind_callback *mid = begin + (end - begin) / 2;
        int cmp = strcmp(mid->interface, interface);

        if (cmp < 0) {
            begin = mid + 1;
        } else if (cmp > 0) {
            end = mid;
        } else {
            mid->function(win, name, version);
            break;
        }
    }
}

static void
window_registry_remove(void *data, struct wl_registry *registry, uint32_t name)
{
    struct window *win = data;

    (void) registry;
    (void) name;

    (void) win;
}

static const struct wl_registry_listener window_registry_callbacks = {
    .global = &window_registry_add,
    .global_remove = &window_registry_remove,
};

static void window_init_wayland(struct window *win, const char *display_name)
{
    TIMER_INIT_SIMPLE();

    win->display = wl_display_connect(display_name);
    if (!win->display)
        die("failed to connect to wayland compositor: %s\n", errstr(errno));

    win->registry = wl_display_get_registry(win->display);
    if (!win->registry)
        die("failed to establish wayland registry\n");

    wl_registry_add_listener(win->registry, &window_registry_callbacks, win);

    /* Wait for the registry to collect and bind all interfaces. */
    wl_display_roundtrip(win->display);

    if (!win->xdg_wm)
        die("missing interface \"xdg_base_wm\"\n");

    xdg_wm_base_add_listener(win->xdg_wm, &xdg_wm_callbacks, win);

    win->wl_surface = wl_compositor_create_surface(win->compositor);

    win->xdg_surface =
        xdg_wm_base_get_xdg_surface(win->xdg_wm, win->wl_surface);
    xdg_surface_add_listener(win->xdg_surface, &xdg_surface_callbacks, win);

    win->xdg_toplevel = xdg_surface_get_toplevel(win->xdg_surface);
    xdg_toplevel_add_listener(win->xdg_toplevel, &xdg_toplevel_callbacks, win);

    xdg_toplevel_set_title(win->xdg_toplevel, WINDOW_NAME);
    xdg_toplevel_set_app_id(win->xdg_toplevel, WINDOW_NAME);

    wl_surface_commit(win->wl_surface);

    /* Dispatch server events to initialize surfaces */
    wl_display_roundtrip(win->display);
}

struct window_event {
    void (*dispatch)(struct window *);
};

static void window_dispatch_wayland_event(struct window *win)
{
    int err;

    err = wl_display_prepare_read(win->display);
    if (err < 0)
        die("failed to prepare reading events - %s\n", errstr(errno));

    err = wl_display_read_events(win->display);
    if (err < 0)
        die("failed to read events - %s\n", errstr(errno));

    err = wl_display_dispatch_pending(win->display);
    if (err < 0)
        die("failed to dispatch events - %s\n", errstr(errno));
}

static void window_dispatch_timer_event(struct window *win)
{
    struct key_event ev;
    uint64_t value;
    int err;

    /* Clear all timeouts */
    err = io_util_read(win->timer_fd, &value, sizeof(value));
    if (err < 0)
        die("io_util_read: %s\n", errstr(errno));

    ev.symbol = win->symbol;
    ev.ctrl = xkb_mod_active(&win->xkb, XKB_MOD_NAME_CTRL);
    ev.shift = xkb_mod_active(&win->xkb, XKB_MOD_NAME_SHIFT);
    ev.mod1 = xkb_mod_active(&win->xkb, XKB_MOD_NAME_ALT);

    (void) widget_do_key_event(&win->widget, ev);
}

static struct window_event window_wayland_event = {
    .dispatch = &window_dispatch_wayland_event};

static struct window_event window_timer_event = {
    .dispatch = &window_dispatch_timer_event};

static void window_init_events(struct window *win)
{
    TIMER_INIT_SIMPLE();

    struct epoll_event ev;
    int fd, err;

    win->epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (win->epoll_fd < 0)
        die("epoll_create1: %s\n", errstr(errno));

    fd = wl_display_get_fd(win->display);

    ev.events = EPOLLIN;
    ev.data.ptr = &window_wayland_event;

    err = epoll_ctl(win->epoll_fd, EPOLL_CTL_ADD, fd, &ev);
    if (err < 0)
        die("epoll_ctl: failed to add wayland events: %s\n", errstr(errno));

    win->timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
    if (win->timer_fd < 0)
        die("timerfd_create: %s\n", errstr(errno));

    ev.events = EPOLLIN;
    ev.data.ptr = &window_timer_event;

    err = epoll_ctl(win->epoll_fd, EPOLL_CTL_ADD, win->timer_fd, &ev);
    if (err < 0)
        die("epoll_ctl: failed to add timer events: %s\n", errstr(errno));
}
static bool window_widget_surface_initialized(const struct window *win)
{
    return win->mem != NULL;
}

static void window_set_widget_surface(struct window *win)
{
    struct wl_shm_pool *pool;
    cairo_surface_t *surface;
    int fd, err;
    int32_t stride;

    TIMER_INIT_SIMPLE();

    widget_get_size_hint(&win->widget, &win->width, &win->height);

    stride = win->width * WL_WINDOW_BYTES_PER_PIXEL;
    win->size = stride * win->height;

    /* Create sharable memory for drawing */
    fd = memfd_create("crudebox", MFD_CLOEXEC);
    if (fd < 0)
        die("failed to create memfd - %s\n", errstr(errno));

    err = ftruncate(fd, (off_t) win->size);
    if (err < 0)
        die("failed to allocate memfd memory - %s\n", errstr(errno));

    win->mem = mmap(NULL, win->size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (win->mem == MAP_FAILED)
        die("failed to memory map memfd - %s\n", errstr(errno));

    pool = wl_shm_create_pool(win->shm, fd, (int32_t) win->size);
    if (!pool)
        die("failed to create shared memory pool\n");

    win->buffer = wl_shm_pool_create_buffer(pool,
                                            0,
                                            win->width,
                                            win->height,
                                            stride,
                                            WL_SHM_FORMAT_XRGB8888);
    if (!win->buffer)
        die("failed to create wayland buffer\n");

    /* Release temporary allocated ressources */
    wl_shm_pool_destroy(pool);
    close(fd);

    /* Create cairo surface for rendering the widget */
    surface = cairo_image_surface_create_for_data(win->mem,
                                                  CAIRO_FORMAT_ARGB32,
                                                  win->width,
                                                  win->height,
                                                  stride);

    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
        die("failed to create cairo surface for rendering\n");

    widget_set_surface(&win->widget, surface);

    cairo_surface_destroy(surface);
}

void window_init(struct window *win, const char *display_name)
{
    TIMER_INIT_SIMPLE();

    memset(win, 0, sizeof(*win));

    xkb_init(&win->xkb);
    window_init_wayland(win, display_name);
    window_init_events(win);
    widget_init(&win->widget);
}

void window_destroy(struct window *win)
{
#ifdef MEM_NOLEAK
    if (win->buffer)
        wl_buffer_destroy(win->buffer);

    if (win->mem)
        munmap(win->mem, win->size);

    close(win->timer_fd);
    close(win->epoll_fd);

    widget_destroy(&win->widget);
    wl_compositor_destroy(win->compositor);
    wl_registry_destroy(win->registry);
    wl_display_disconnect(win->display);
    xkb_destroy(&win->xkb);
#else
    (void) win;
#endif
}

void window_show(struct window *win)
{
    if (!window_widget_surface_initialized(win))
        window_set_widget_surface(win);

    widget_draw(&win->widget);
    wl_surface_attach(win->wl_surface, win->buffer, 0, 0);
    wl_surface_commit(win->wl_surface);
}

void window_dispatch_events(struct window *win)
{
    win->active = true;

    while (win->active) {
        struct epoll_event events[2];
        int n;

        wl_display_flush(win->display);

        n = epoll_wait(win->epoll_fd, events, ARRAY_SIZE(events), -1);
        if (n < 0) {
            if (errno == EINTR)
                continue;

            die("epoll_wait: %s\n", errstr(errno));
        }

        while (n--)
            ((struct window_event *) events[n].data.ptr)->dispatch(win);
    }
}

#endif /* CONFIG_USE_WAYLAND */
