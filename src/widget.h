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
#include "list-view.h"
#include "widget-common.h"

struct key_event {
    int symbol;

    uint8_t shift : 1;
    uint8_t ctrl : 1;
    uint8_t mod1 : 1;
};

struct widget {
    FT_Library freetype;
    FT_Face face;

    cairo_t *cairo;

    uint32_t width;
    uint32_t height;

    bool print;

    struct color frame;
    uint32_t line_width;

    struct line_edit line_edit;
    struct list_view list_view;
};

void widget_init(struct widget *widget);

void widget_destroy(struct widget *widget);

void widget_set_surface(struct widget *widget, cairo_surface_t *surface);

static inline struct line_edit *widget_line_edit(struct widget *widget)
{
    return &widget->line_edit;
}

static inline struct list_view *widget_list_view(struct widget *widget)
{
    return &widget->list_view;
}

void widget_get_size_hint(const struct widget *widget,
                          uint32_t *width,
                          uint32_t *height);

void widget_set_font(struct widget *widget, const char *path);

static inline void widget_set_font_size(struct widget *widget, uint32_t size)
{
    cairo_set_font_size(widget->cairo, (double) size);
}

static inline void widget_set_frame_color(struct widget *widget, uint32_t value)
{
    color_set_u32(&widget->frame, value);
}

static inline void widget_set_line_width(struct widget *widget, uint32_t width)
{
    widget->line_width = width;
}

static inline void widget_set_print(struct widget *widget, bool print)
{
    widget->print = print;
}

static inline void widget_set_max_rows(struct widget *widget, int num)
{
    list_view_set_max_rows(&widget->list_view, num);
}

static inline void widget_set_item_list(struct widget *widget,
                                        struct item_list *list)
{
    list_view_set_item_list(&widget->list_view, list);
}

void widget_set_size(struct widget *widget, uint32_t width, uint32_t height);

void widget_draw(struct widget *widget);

bool widget_do_key_event(struct widget *widget, struct key_event ev);

#endif /* WIDGET_H_ */
