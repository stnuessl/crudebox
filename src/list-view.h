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

#ifndef LIST_VIEW_H_
#define LIST_VIEW_H_

#include <stdint.h>

#include <cairo.h>

#include "item-list.h"
#include "widget-common.h"

#include "util/xalloc.h"

struct list_view {
    cairo_t *cairo;
    cairo_scaled_font_t *font;

    cairo_glyph_t glyphs[64];

    struct item_list *items;

    uint32_t x1;
    uint32_t y1;
    uint32_t x2;
    uint32_t y2;

    uint32_t glyph_x;
    uint32_t glyph_y;

    const char **entries;
    int selected;
    int n_entries;
    int max_entries;

    struct color fg;
    struct color bg[2];
    struct color fg_sel;
    struct color bg_sel[2];
    struct color lines;
};

void list_view_init(struct list_view *view, cairo_t *cairo);

void list_view_destroy(struct list_view *view);

static inline void list_view_set_item_list(struct list_view *view,
                                           struct item_list *list)
{
    view->items = list;
}

static inline struct item_list *list_view_item_list(struct list_view *view)
{
    return view->items;
}

void list_view_size_hint(const struct list_view *view,
                         const cairo_font_extents_t *ext,
                         uint32_t *width,
                         uint32_t *height);

void list_view_configure(struct list_view *view,
                         const cairo_font_extents_t *ext,
                         uint32_t x1,
                         uint32_t y1,
                         uint32_t x2,
                         uint32_t y2);

static inline void list_view_set_fg(struct list_view *view, uint32_t rgba)
{
    color_set_u32(&view->fg, rgba);
}

static inline void
list_view_set_bg(struct list_view *view, uint32_t rgba1, uint32_t rgba2)
{
    color_set_u32(view->bg + 0, rgba1);
    color_set_u32(view->bg + 1, rgba2);
}

static inline void list_view_set_fg_sel(struct list_view *view, uint32_t rgba)
{
    color_set_u32(&view->fg_sel, rgba);
}

static inline void
list_view_set_bg_sel(struct list_view *view, uint32_t rgba1, uint32_t rgba2)
{
    color_set_u32(view->bg_sel + 0, rgba1);
    color_set_u32(view->bg_sel + 1, rgba2);
}

static inline void list_view_set_lines(struct list_view *view, uint32_t rgba)
{
    color_set_u32(&view->lines, rgba);
}

static inline void list_view_set_max_rows(struct list_view *view, int n)
{
    view->entries = xrealloc(view->entries, n * sizeof(*view->entries));
    view->max_entries = n;
}

static inline const char *list_view_get_entry(const struct list_view *view)
{
    if (view->selected < 0 || view->selected >= view->n_entries)
        return NULL;

    return view->entries[view->selected];
}

void list_view_up(struct list_view *view);

void list_view_down(struct list_view *view);

void list_view_lookup_push_back(struct list_view *view, int c);

void list_view_lookup_pop_back(struct list_view *view);

void list_view_lookup_clear(struct list_view *view);

void list_view_draw(struct list_view *view);

#endif /* LIST_VIEW_H_ */
