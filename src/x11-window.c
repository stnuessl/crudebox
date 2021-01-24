/*
 * Copyright (C) 2020   Steffen Nuessle
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

#ifdef USE_X11

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <cairo-xcb.h>
#include <xkbcommon/xkbcommon.h>

#include "timer.h"
#include "widget.h"
#include "window.h"

#include "util/die.h"
#include "util/macro.h"

#define WINDOW_NAME "crudebox"

struct cookies {
    xcb_intern_atom_cookie_t net_wm_window_type;
    xcb_intern_atom_cookie_t net_wm_window_type_utility;
    xcb_intern_atom_cookie_t motif_wm_hints;
    xcb_grab_keyboard_cookie_t grab_keyboard;
};

static void window_set_root_visual(struct window *win)
{
    xcb_depth_iterator_t depth_iter;
    xcb_visualtype_iterator_t visual_iter;

    depth_iter = xcb_screen_allowed_depths_iterator(win->screen);
    while (likely(depth_iter.rem)) {
        visual_iter = xcb_depth_visuals_iterator(depth_iter.data);

        while (likely(visual_iter.rem)) {
            if (visual_iter.data->visual_id == win->screen->root_visual) {
                win->visual = visual_iter.data;
                return;
            }

            xcb_visualtype_next(&visual_iter);
        }

        xcb_depth_next(&depth_iter);
    }

    die("failed to find the screen's root visual\n");
}

static void window_set_size(struct window *win, uint32_t width, uint32_t height)
{
    uint32_t mask = XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT;
    uint32_t values[] = {width, height};

    if (win->width == width && win->height == height)
        return;

    win->width = width;
    win->height = height;

    (void) xcb_configure_window(win->conn, win->xid, mask, values);
}

static void window_init_xcb(struct window *win, const char *display_name)
{
    int err;

    win->conn = xcb_connect(display_name, NULL);

    err = xcb_connection_has_error(win->conn);
    if (unlikely(err))
        die("failed to establish xcb connection\n");

    win->setup = xcb_get_setup(win->conn);
    win->screen = xcb_setup_roots_iterator(win->setup).data;

    window_set_root_visual(win);

    win->symbols = xcb_key_symbols_alloc(win->conn);
    if (unlikely(!win->symbols))
        die("failed to initialize key symbols\n");
}

static void window_init_cookies(struct window *win, struct cookies *cookies)
{
    static const char *names[] = {
        "_NET_WM_WINDOW_TYPE",
        "_NET_WM_WINDOW_TYPE_UTILITY",
        "_MOTIF_WM_HINTS",
    };

    cookies->net_wm_window_type =
        xcb_intern_atom(win->conn, false, strlen(names[0]), names[0]);

    cookies->net_wm_window_type_utility =
        xcb_intern_atom(win->conn, false, strlen(names[1]), names[1]);

    cookies->motif_wm_hints =
        xcb_intern_atom(win->conn, false, strlen(names[2]), names[2]);

    cookies->grab_keyboard = xcb_grab_keyboard(win->conn,
                                               true,
                                               win->screen->root,
                                               XCB_CURRENT_TIME,
                                               XCB_GRAB_MODE_ASYNC,
                                               XCB_GRAB_MODE_ASYNC);
}

static void window_init_frame(struct window *win)
{
    uint32_t values[2];

    win->xid = xcb_generate_id(win->conn);

    win->width = 1920;
    win->height = 1080;

    values[0] = XCB_NONE;
    values[1] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS;

    /* Do all the X11 window configuration */
    (void) xcb_create_window(win->conn,
                             win->screen->root_depth,
                             win->xid,
                             win->screen->root,
                             0, /* x */
                             0, /* y */
                             win->width,
                             win->height,
                             0, /* border_width */
                             XCB_WINDOW_CLASS_INPUT_OUTPUT,
                             win->screen->root_visual,
                             XCB_CW_BACK_PIXMAP | XCB_CW_EVENT_MASK,
                             values);

    /* Set window name */
    (void) xcb_change_property(win->conn,
                               XCB_PROP_MODE_REPLACE,
                               win->xid,
                               XCB_ATOM_WM_NAME,
                               XCB_ATOM_STRING,
                               8, /* format */
                               strlen(WINDOW_NAME),
                               WINDOW_NAME);

    /* Set this window transient to the root window */
    (void) xcb_change_property(win->conn,
                               XCB_PROP_MODE_REPLACE,
                               win->xid,
                               XCB_ATOM_WM_TRANSIENT_FOR,
                               XCB_ATOM_WINDOW,
                               32, /* format */
                               1,  /* length */
                               &win->screen->root);
}

static void window_init_widget(struct window *win)
{
    cairo_surface_t *surface;

    /* Initialize the cairo surface to be used by the widget */
    surface = cairo_xcb_surface_create(win->conn,
                                       win->xid,
                                       win->visual,
                                       win->width,
                                       win->height);

    if (unlikely(cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS))
        die("failed to create cairo xcb surface\n");

    widget_init(&win->widget, surface);

    /* The widget itself is responsible to keep a handle on the surface */
    cairo_surface_destroy(surface);
}

static int window_grab_keyboard_fallback(struct window *win)
{
    struct timespec ts;
    int attempts;

    /* 
     * Use force for grabbing the keyboard.
     * We perform a grab every 1500 us. After 100 failed attempts we abort.
     * Worst case is that we wasted 150 ms
     */
    ts.tv_sec = 0;
    ts.tv_nsec = 1500000;

    attempts = 100;

    while (attempts--) {
        xcb_grab_keyboard_cookie_t cookie;
        xcb_generic_error_t *error;

        cookie = xcb_grab_keyboard(win->conn,
                                   true,
                                   win->screen->root,
                                   XCB_CURRENT_TIME,
                                   XCB_GRAB_MODE_ASYNC,
                                   XCB_GRAB_MODE_ASYNC);

#ifdef MEM_NOLEAK
        free(win->grab_keyboard);
#endif
        error = NULL;

        win->grab_keyboard = xcb_grab_keyboard_reply(win->conn, cookie, &error);
        if (!error && win->grab_keyboard->status == XCB_GRAB_STATUS_SUCCESS)
            return 0;

#ifdef MEM_NOLEAK
        free(error);
#endif
        (void) nanosleep(&ts, NULL);
    }

    return -1;
}

static void window_init_attributes(struct window *win,
                                   const struct cookies *cookies)
{
    xcb_generic_error_t *error = NULL;
    uint32_t hints[] = {0x02, 0x00, 0x00, 0x00, 0x00};

    /* Collect replies from the server. */
    win->net_wm_window_type =
        xcb_intern_atom_reply(win->conn, cookies->net_wm_window_type, &error);

    if (unlikely(error))
        die("failed to retrieve intern atom \"_NET_WM_WINDOW_TYPE\"\n");

    win->net_wm_window_type_utility =
        xcb_intern_atom_reply(win->conn,
                              cookies->net_wm_window_type_utility,
                              &error);

    if (unlikely(error))
        die("failed to retrieve intern atom \"_NET_WM_WINDOW_TYPE_UTILITY\"\n");

    win->motif_wm_hints =
        xcb_intern_atom_reply(win->conn, cookies->motif_wm_hints, &error);

    if (unlikely(error))
        die("failed to retrieve intern atom \"_MOTIF_WM_HINTS\"\n");

    /* Set window type to 'utility' */
    (void) xcb_change_property(win->conn,
                               XCB_PROP_MODE_REPLACE,
                               win->xid,
                               win->net_wm_window_type->atom,
                               XCB_ATOM_ATOM,
                               32, /* format */
                               1,  /* length */
                               &win->net_wm_window_type_utility->atom);

    /* Remove window decorations */
    (void) xcb_change_property(win->conn,
                               XCB_PROP_MODE_REPLACE,
                               win->xid,
                               win->motif_wm_hints->atom,
                               XCB_ATOM_INTEGER,
                               32, /* format */
                               ARRAY_SIZE(hints),
                               hints);

    win->grab_keyboard =
        xcb_grab_keyboard_reply(win->conn, cookies->grab_keyboard, &error);

    if (error || win->grab_keyboard->status != XCB_GRAB_STATUS_SUCCESS) {
        int err;
#ifdef MEM_NOLEAK
        free(error);
#endif
        err = window_grab_keyboard_fallback(win);
        if (err < 0)
            die("failed to grab keyboard\n");
    }
}

void window_init(struct window *win, const char *display_name)
{
    struct cookies cookies;

    TIMER_INIT_SIMPLE();

    window_init_xcb(win, display_name);
    window_init_cookies(win, &cookies);
    window_init_frame(win);
    window_init_widget(win);
    window_init_attributes(win, &cookies);
}

void window_destroy(struct window *win)
{
    (void) xcb_ungrab_keyboard(win->conn, XCB_TIME_CURRENT_TIME);

#ifdef MEM_NOLEAK
    free(win->grab_keyboard);
    free(win->motif_wm_hints);
    free(win->net_wm_window_type_utility);
    free(win->net_wm_window_type);
    xcb_key_symbols_free(win->symbols);
    xcb_disconnect(win->conn);
#else
    (void) win;
#endif
}

void window_update_size(struct window *win)
{
    uint32_t width, height;

    TIMER_INIT_SIMPLE();

    widget_get_size_hint(&win->widget, &width, &height);

    window_set_size(win, width, height);
    widget_set_size(&win->widget, width, height);
}

void window_show(struct window *win)
{     
    (void) xcb_map_window(win->conn, win->xid);
}

void window_dispatch_events(struct window *win)
{


    while (1) {
        xcb_generic_event_t *ev;
        xcb_keysym_t symbol;
        struct key_event key_event;
        uint16_t mods;

        (void) xcb_flush(win->conn);

        ev = xcb_wait_for_event(win->conn);
        if (unlikely(!ev))
            die("lost x11 connection to the display manager\n");

        switch (ev->response_type & 0x7f) {
        case XCB_EXPOSE: 
            /* Make sure our window has the input focues */
            (void) xcb_set_input_focus(win->conn,
                                       XCB_INPUT_FOCUS_POINTER_ROOT,
                                       win->xid,
                                       XCB_CURRENT_TIME);


            widget_draw(&win->widget); 

            break;
        case XCB_KEY_PRESS:
            symbol = xcb_key_press_lookup_keysym(win->symbols, (void *) ev, 0);

            if (symbol == XKB_KEY_Escape) {
#ifdef MEM_NOLEAK
                free(ev);
#endif
                return;
            }

            mods = ((xcb_button_press_event_t *) ev)->state;

            key_event.symbol = (int) symbol;
            key_event.shift = !!(mods & XCB_MOD_MASK_SHIFT);
            key_event.ctrl = !!(mods & XCB_MOD_MASK_CONTROL);
            key_event.mod1 = !!(mods & XCB_MOD_MASK_1);
            key_event.mod2 = !!(mods & XCB_MOD_MASK_2);

            widget_do_key_event(&win->widget, key_event);
            break;
        case XCB_KEY_RELEASE:
            break;
        default:
            break;
        }

#ifdef MEM_NOLEAK
        free(ev);
#endif
    }
}

#endif /* USE_X11 */
