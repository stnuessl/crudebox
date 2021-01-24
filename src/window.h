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

#ifndef WINDOW_H_
#define WINDOW_H_

#ifdef USE_X11

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

#include "widget.h"

struct window {
    struct widget widget;

    xcb_connection_t *conn;
    const xcb_setup_t *setup;
    xcb_screen_t *screen;
    xcb_visualtype_t *visual;
    xcb_key_symbols_t *symbols;
    xcb_window_t xid;

    xcb_intern_atom_reply_t *net_wm_window_type;
    xcb_intern_atom_reply_t *net_wm_window_type_utility;
    xcb_intern_atom_reply_t *motif_wm_hints;
    xcb_grab_keyboard_reply_t *grab_keyboard;

    uint32_t width;
    uint32_t height;
};

#elif USE_WAYLAND

#include <wayland-client.h>

struct window {
    struct wl_display *display;
};

#else
#error "Invalid windowing system configured."
#endif

void window_init(struct window *win, const char *display_name);

void window_destroy(struct window *win);

static inline struct widget *window_get_widget(struct window *win)
{
    return &win->widget;
}

void window_update_size(struct window *win);

void window_show(struct window *win);

void window_dispatch_events(struct window *win);

#endif /* WINDOW_H_ */
