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

#ifndef WIDGET_H_
#define WIDGET_H_

#include <stdbool.h>

#include <cairo.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MODULE_H

#include <xkbcommon/xkbcommon.h>

#include "item-list.h"
#include "line-edit.h"
#include "menu.h"

struct key_event {
    int symbol;

    uint8_t shift : 1;
    uint8_t ctrl : 1;
    uint8_t mod1 : 1;
    uint8_t mod2 : 1;
};

struct widget {
    FT_Library freetype;
    FT_Face face;

    cairo_surface_t *surface;
    cairo_t *cairo;

    struct app_list *apps;

    uint32_t width;
    uint32_t height;

    double line;
    bool print;

    struct line_edit line_edit;
    struct menu menu;

    struct color frame;
};

void widget_init(struct widget *widget, cairo_surface_t *surface);

void widget_destroy(struct widget *widget);

void widget_get_size_hint(const struct widget *widget,
                          uint32_t *width,
                          uint32_t *height);

void widget_set_font(struct widget *widget, const char *path);

void widget_set_font_size(struct widget *widget, double size);

void widget_set_line_width(struct widget *widget, double line_width);

static inline void widget_set_frame(struct widget *widget, uint32_t value)
{
    color_set_u32(&widget->frame, value);
}

static inline void widget_set_print(struct widget *widget, bool print)
{
    widget->print = print;
}

void widget_set_max_rows(struct widget *widget, int num);

void widget_set_item_list(struct widget *widget, struct item_list *list);

void widget_set_size(struct widget *widget, uint32_t width, uint32_t height);

void widget_draw(struct widget *widget);

void widget_do_key_event(struct widget *widget, struct key_event ev);

#endif /* WIDGET_H_ */
