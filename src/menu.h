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

#ifndef MENU_H_
#define MENU_H_

#include <stdint.h>

#include <cairo.h>

#include "color.h"
#include "item-list.h"

#include "util/xalloc.h"

struct menu {
    cairo_t *cairo;
    cairo_scaled_font_t *scaled_font;
    cairo_glyph_t glyphs[64];
    int n_glyphs;

    struct item_list *items;

    int x;
    int y;
    int width;
    int height;

    const char **entries;
    int selected;
    int n_entries;
    int max_entries;

    int entry_height;

    double glyph_x_offset;
    double glyph_y_offset;

    struct color fg;
    struct color bg[2];
    struct color fg_sel;
    struct color bg_sel;
};

void menu_init(struct menu *menu);

void menu_destroy(struct menu *menu);

static inline void menu_set_item_list(struct menu *menu, struct item_list *list)
{
    menu->items = list;
}

static inline struct item_list *menu_item_list(struct menu *menu)
{
    return menu->items;
}

void menu_size_hint(const struct menu *menu,
                    const cairo_font_extents_t *extents,
                    uint32_t *width,
                    uint32_t *height);

void menu_configure(
    struct menu *menu, cairo_t *cairo, int x, int y, int width, int height);

static inline void menu_set_fg(struct menu *menu, uint32_t rgba)
{
    color_set_u32(&menu->fg, rgba);
}

static inline void
menu_set_bg(struct menu *menu, uint32_t rgba1, uint32_t rgba2)
{
    color_set_u32(menu->bg + 0, rgba1);
    color_set_u32(menu->bg + 1, rgba2);
}

static inline void menu_set_fg_sel(struct menu *menu, uint32_t rgba)
{
    color_set_u32(&menu->fg_sel, rgba);
}

static inline void menu_set_bg_sel(struct menu *menu, uint32_t rgba)
{
    color_set_u32(&menu->bg_sel, rgba);
}

static inline uint32_t menu_x(const struct menu *menu)
{
    return menu->x;
}

static inline uint32_t menu_y(const struct menu *menu)
{
    return menu->y;
}

static inline uint32_t menu_width(const struct menu *menu)
{
    return menu->width;
}

static inline uint32_t menu_height(const struct menu *menu)
{
    return menu->height;
}

static inline void menu_set_max_rows(struct menu *menu, int n)
{
    menu->entries = xrealloc(menu->entries, n * sizeof(*menu->entries));
    menu->max_entries = n;
}

static inline const char *menu_get_entry(struct menu *menu)
{
    if (menu->selected < 0 || menu->selected >= menu->n_entries)
        return "";

    return menu->entries[menu->selected];
}

void menu_up(struct menu *menu);

void menu_down(struct menu *menu);

void menu_lookup_push_back(struct menu *menu, int c);

void menu_lookup_pop_back(struct menu *menu);

void menu_lookup_clear(struct menu *menu);

void menu_draw(struct menu *menu);

#endif /* MENU_H_ */
