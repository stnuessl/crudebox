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

#define WINDOW_NAME "crudebox"

#ifdef CONFIG_USE_X11

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

#elif CONFIG_USE_WAYLAND

#include <wayland-client.h>

#include "widget.h"
#include "xkb.h"

struct window {
    struct widget widget;
    struct xkb xkb;

    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_shell *shell;
    struct wl_shm *shm;
    struct wl_buffer *buffer;
    struct wl_seat *seat;
    struct wl_output *output;
    struct wl_keyboard *keyboard;
    struct xdg_wm_base *xdg_wm;

    struct wl_surface *wl_surface;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;

    void *mem;
    size_t size;

    int epoll_fd;
    int timer_fd;

    uint32_t width;
    uint32_t height;

    xkb_keysym_t symbol;
    int32_t rate;
    int32_t delay;
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
