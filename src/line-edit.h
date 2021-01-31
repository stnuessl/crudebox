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

#ifndef LINE_EDIT_H_
#define LINE_EDIT_H_

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>

#include <cairo.h>

#include "widget-common.h"

struct line_edit {
    cairo_t *cairo;
    cairo_scaled_font_t *font;

    char str[64];
    int strlen;

    cairo_glyph_t glyphs[64];

    uint32_t x1;
    uint32_t y1;
    uint32_t x2;
    uint32_t y2;

    uint32_t glyph_x;
    uint32_t glyph_y;

    struct color fg;
    struct color bg;
};

void line_edit_init(struct line_edit *edit, cairo_t *cairo);

void line_edit_destroy(struct line_edit *edit);

void line_edit_size_hint(const struct line_edit *edit,
                         const cairo_font_extents_t *ext,
                         uint32_t *width,
                         uint32_t *height);

void line_edit_configure(struct line_edit *edit,
                         const cairo_font_extents_t *ext,
                         uint32_t x1,
                         uint32_t y1,
                         uint32_t x2,
                         uint32_t y2);

static inline uint32_t line_edit_width(const struct line_edit *edit)
{
    return edit->x2 - edit->x1;
}

static inline uint32_t line_edit_height(const struct line_edit *edit)
{
    return edit->y2 - edit->y1;
}

static inline void line_edit_set_fg(struct line_edit *edit, uint32_t rgba)
{
    color_set_u32(&edit->fg, rgba);
}

static inline void line_edit_set_bg(struct line_edit *edit, uint32_t rgba)
{
    color_set_u32(&edit->bg, rgba);
}

void line_edit_clear(struct line_edit *edit);

void line_edit_push_back(struct line_edit *edit, int c);

void line_edit_pop_back(struct line_edit *edit);

void line_edit_draw(struct line_edit *edit);

#endif /* LINE_EDIT_H_ */
