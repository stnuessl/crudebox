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

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <cairo-xcb.h>
#include <xkbcommon/xkbcommon.h>

#include "timer.h"
#include "widget.h"
#include "window.h"

#include "util/array.h"
#include "util/die.h"

#define WINDOW_NAME "crudebox"

static const char *atoms[] = {
    "_NET_WM_WINDOW_TYPE",
    "_NET_WM_WINDOW_TYPE_UTILITY",
    "_MOTIF_WM_HINTS",
};

static void window_set_root_visual(struct window *win)
{
    xcb_depth_iterator_t depth_iter;
    xcb_visualtype_iterator_t visual_iter;

    depth_iter = xcb_screen_allowed_depths_iterator(win->screen);
    while (depth_iter.rem) {
        visual_iter = xcb_depth_visuals_iterator(depth_iter.data);

        while (visual_iter.rem) {
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

static xcb_intern_atom_cookie_t
cookie(struct window *win, bool only_if_exist, const char *name)
{
    return xcb_intern_atom(win->conn, only_if_exist, strlen(name), name);
}

static xcb_intern_atom_reply_t *reply(struct window *win,
                                      xcb_intern_atom_cookie_t xcb_cookie,
                                      xcb_generic_error_t **error)
{
    return xcb_intern_atom_reply(win->conn, xcb_cookie, error);
}

static bool window_is_configured(const struct window *win)
{
    return win->replies[0] != NULL;
}

static void window_configure(struct window *win)
{
    cairo_surface_t *surface;
    xcb_generic_error_t *error = NULL;
    int16_t x = 0, y = 0;
    uint32_t hints[] = {0x02, 0x00, 0x00, 0x00, 0x00};
    uint32_t values[] = {
        /* clang-format off */
        XCB_NONE, 
        XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS, 
        XCB_EVENT_MASK_KEY_RELEASE,
        /* clang-format on */
    };

    TIMER_INIT_SIMPLE(CLOCK_MONOTONIC);

    widget_get_size_hint(&win->widget, &win->width, &win->height);

    /* Do all the X11 window configuration */
    (void) xcb_create_window(win->conn,
                             win->screen->root_depth,
                             win->xid,
                             win->screen->root,
                             x,
                             y,
                             (uint16_t) win->width,
                             (uint16_t) win->height,
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

    /* We need to gather all atom responses */
    for (int i = 0; i < ARRAY_SIZE(atoms); ++i) {
        win->replies[i] = reply(win, win->cookies[i], &error);
        if (error)
            die("failed to get intern atom \"%s\"\n", atoms[i]);
    }

    /* Set window type to 'utility' */
    (void) xcb_change_property(win->conn,
                               XCB_PROP_MODE_REPLACE,
                               win->xid,
                               win->replies[0]->atom,
                               XCB_ATOM_ATOM,
                               32, /* format */
                               1,  /* length */
                               &win->replies[1]->atom);

    /* Remove window decorations */
    (void) xcb_change_property(win->conn,
                               XCB_PROP_MODE_REPLACE,
                               win->xid,
                               win->replies[2]->atom,
                               XCB_ATOM_INTEGER,
                               32, /* format */
                               ARRAY_SIZE(hints),
                               hints);

    /* Initialize the cairo surface to be used by the widget */
    surface = cairo_xcb_surface_create(win->conn,
                                       win->xid,
                                       win->visual,
                                       win->width,
                                       win->height);

    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
        die("failed to create cairo xcb surface\n");

    widget_configure(&win->widget, surface);
    cairo_surface_destroy(surface);
}

void window_init(struct window *win, const char *display_name)
{
    win->conn = xcb_connect(display_name, NULL);
    if (xcb_connection_has_error(win->conn))
        die("failed to establish xcb connection\n");

    win->setup = xcb_get_setup(win->conn);
    win->screen = xcb_setup_roots_iterator(win->setup).data;

    window_set_root_visual(win);

    win->symbols = xcb_key_symbols_alloc(win->conn);
    if (!win->symbols)
        die("failed to initialize key symbols\n");

    win->xid = xcb_generate_id(win->conn);

    /* Request cookies for internal atoms */
    for (int i = 0; i < ARRAY_SIZE(atoms); ++i) {
        win->cookies[i] = cookie(win, false, atoms[i]);
        win->replies[i] = NULL;
    }

    win->width = 0;
    win->height = 0;

    widget_init(&win->widget);
}

void window_destroy(struct window *win)
{
    (void) win;
}

void window_show(struct window *win)
{
    if (!window_is_configured(win))
        window_configure(win);

    (void) xcb_map_window(win->conn, win->xid);
}

void window_dispatch_events(struct window *win)
{
    xcb_generic_event_t *ev;
    xcb_keycode_t key;
    xcb_keysym_t symbol;
    struct key_event key_event;
    uint16_t mods;

#if 0
    xcb_grab_keyboard_cookie_t x = xcb_grab_keyboard(win->conn, 
                                                     1,
                                                     win->xid,
                                                     XCB_CURRENT_TIME,
                                                     XCB_GRAB_MODE_ASYNC,
                                                     XCB_GRAB_MODE_ASYNC);

    xcb_grab_keyboard_reply_t *r = xcb_grab_keyboard_reply(win->conn, x, NULL);
#endif

    (void) xcb_set_input_focus(win->conn,
                               XCB_INPUT_FOCUS_POINTER_ROOT,
                               win->xid,
                               XCB_CURRENT_TIME);

    while (1) {

        (void) xcb_flush(win->conn);

        ev = xcb_wait_for_event(win->conn);
        if (!ev)
            die("lost x11 connection to the display manager\n");

        TIMER_INIT("X11-Event", CLOCK_MONOTONIC);

        switch (ev->response_type & 0x7f) {
        case XCB_EXPOSE:
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
            key = ((xcb_key_release_event_t *) ev)->detail;

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
