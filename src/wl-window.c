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

#include "util/die.h"
#include "util/errstr.h"
#include "util/macro.h"
#include "util/string-util.h"

#ifdef CONFIG_USE_WAYLAND

static void window_xdg_toplevel_surface_configure(void *data,
                                                  struct xdg_toplevel *toplevel,
                                                  int32_t width,
                                                  int32_t height,
                                                  struct wl_array *states)
{
    struct window *win = data;
    enum xdg_toplevel_state *iter;

    (void) toplevel;

    if (width == 0 || height == 0)
        return;

    wl_array_for_each(iter, states)
    {
        printf("%u\n", *iter);
    }

    printf("xdg_toplevel_surface_configure()\n");
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

static inline int32_t window_buffer_stride(const struct window *win)
{
    return win->width * 4;
}

static void window_init_widget(struct window *window);

static void window_xdg_surface_configure(void *data,
                                         struct xdg_surface *surface,
                                         uint32_t serial)
{
    struct window *win = data;
    struct wl_shm_pool *pool;
    struct wl_buffer *buffer;
    void *mem;
    int fd, err;
    uint32_t stride;
    off_t size;

    (void) surface;

    TIMER_INIT_SIMPLE();

    xdg_surface_ack_configure(win->xdg_surface, serial);

    /* Create sharable memory for drawing */
    fd = memfd_create("crudebox", MFD_CLOEXEC);
    if (fd < 0)
        die("failed to create memfd - %s\n", errstr(errno));

    stride = window_buffer_stride(win);
    size = stride * win->height;

    err = ftruncate(fd, size);
    if (err < 0)
        die("failed to allocate memfd memory - %s\n", errstr(errno));

    mem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mem == MAP_FAILED)
        die("failed to memory map memfd - %s\n", errstr(errno));

    pool = wl_shm_create_pool(win->shm, fd, (int32_t) size);
    if (!pool)
        die("failed to create shared memory pool\n");

    buffer = wl_shm_pool_create_buffer(pool,
                                       0,
                                       win->width,
                                       win->height,
                                       stride,
                                       WL_SHM_FORMAT_XRGB8888);
    if (!buffer)
        die("failed to create wayland buffer\n");

    wl_shm_pool_destroy(pool);
    close(fd);

    win->mem = mem;
    win->size = size;

    printf("initial configure\n");
    window_init_widget(win);
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

    printf("width: %d, height: %d\n", width, height);
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

static void
window_bind_seat(struct window *win, uint32_t name, uint32_t version)
{
    if (win->seat)
        return;

    win->seat =
        wl_registry_bind(win->registry, name, &wl_seat_interface, version);

    if (!win->seat)
        die("failed to create binding\n");
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

    wl_surface_commit(win->wl_surface);
    wl_display_flush(win->display);
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
    (void) win;
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

static void window_init_widget(struct window *win)
{
    cairo_surface_t *surface;
    int32_t stride;

    TIMER_INIT_SIMPLE();

    stride = window_buffer_stride(win);

    surface = cairo_image_surface_create_for_data(win->mem,
                                                  CAIRO_FORMAT_ARGB32,
                                                  win->width,
                                                  win->height,
                                                  stride);

    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
        die("failed to create cairo surface for rendering\n");

    widget_init(&win->widget, surface);

    cairo_surface_destroy(surface);
}

void window_init(struct window *win, const char *display_name)
{
    TIMER_INIT_SIMPLE();

    memset(win, 0, sizeof(*win));

    window_init_wayland(win, display_name);
    window_init_events(win);
}

void window_destroy(struct window *win)
{
#ifdef MEM_NOLEAK
    if (win->buffer)
        wl_buffer_destroy(win->buffer);

    if (win->mem)
        munmap(win->mem, win->size);

    wl_compositor_destroy(win->compositor);
    wl_registry_destroy(win->registry);
    wl_display_disconnect(win->display);
#else
    (void) win;
#endif
}

void window_update_size(struct window *win)
{
    (void) win;
}

void window_show(struct window *win)
{
    (void) win;
}

void window_dispatch_events(struct window *win)
{
    while (1) {
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
