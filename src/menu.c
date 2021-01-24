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

#include <string.h>

#include "menu.h"
#include "timer.h"

#include "util/macro.h"

static inline const struct color *menu_get_fg(const struct menu *menu,
                                              int index)
{
    if (menu->selected == index)
        return &menu->fg_sel;

    return &menu->fg;
}

static inline const struct color *menu_get_bg(const struct menu *menu,
                                              int index)
{
    if (menu->selected == index)
        return &menu->bg_sel;

    return menu->bg + !(index & 0x01);
}

static void menu_update_entry_list(struct menu *menu)
{
    const struct item *item, *end;
    int n, score;

    item = item_list_cbegin(menu->items);
    end = item_list_cend(menu->items);
    n = 0;
    score = item_list_lookup_score(menu->items);

    while (n < menu->max_entries && item != end) {
        if (item->score == score)
            menu->entries[n++] = item->name;

        ++item;
    }

    menu->n_entries = n;

    /*
     * Ensure that the selected entry is always within the allowed
     * range. If there can be no selected entry because the menu does not
     * display any items at all, the value of the selected entry has to
     * be brought back to zero as soon as entries are available again.
     */
    if (menu->selected >= menu->n_entries)
        menu->selected = menu->n_entries - 1;
    else if (menu->selected < 0 && menu->n_entries > 0)
        menu->selected = 0;
}

static void menu_update_entry(struct menu *menu, int index)
{
    cairo_glyph_t *glyphs;
    cairo_status_t status;
    const struct color *fg, *bg;
    int x, y, width, height, n_glyphs;

    /* Update background */
    if (index < 0)
        return;

    x = menu->x;
    y = menu->y + index * menu->entry_height;
    width = menu->width;
    height = menu->entry_height;

    bg = menu_get_bg(menu, index);

    cairo_rectangle(menu->cairo, x, y, width, height);
    cairo_set_source_rgba(menu->cairo, bg->red, bg->green, bg->blue, bg->alpha);

    cairo_fill(menu->cairo);

    /* Update glyphs */
    if (index >= menu->n_entries)
        return;

    glyphs = menu->glyphs;
    n_glyphs = ARRAY_SIZE(menu->glyphs);

    x += menu->glyph_x_offset;
    y += menu->glyph_y_offset + height / 2;

    fg = menu_get_fg(menu, index);

    status = cairo_scaled_font_text_to_glyphs(menu->font,
                                              x,
                                              y,
                                              menu->entries[index],
                                              strlen(menu->entries[index]),
                                              &glyphs,
                                              &n_glyphs,
                                              NULL,
                                              NULL,
                                              NULL);

    if (unlikely(status != CAIRO_STATUS_SUCCESS))
        die("failed to retrieve glyphs\n");

    cairo_set_source_rgba(menu->cairo, fg->red, fg->green, fg->blue, fg->alpha);

    cairo_show_glyphs(menu->cairo, glyphs, n_glyphs);

    if (glyphs != menu->glyphs)
        cairo_glyph_free(glyphs);
}

void menu_init(struct menu *menu, cairo_t *cairo)
{
    memset(menu, 0, sizeof(*menu));

    menu->cairo = cairo;
}

void menu_destroy(struct menu *menu)
{
#ifdef MEM_NOLEAK
    free(menu->entries);
#else
    (void) menu;
#endif
}

void menu_size_hint(const struct menu *menu,
                    const cairo_font_extents_t *extents,
                    uint32_t *width,
                    uint32_t *height)
{
    uint32_t entry_height;

    entry_height = (uint32_t)(1.25 * extents->height);

    *width = (uint32_t)(64 * extents->max_x_advance);
    *height = (uint32_t)(menu->max_entries * entry_height);
}

void menu_configure(struct menu *menu,
                    const cairo_font_extents_t *extents,
                    uint32_t x,
                    uint32_t y,
                    uint32_t width,
                    uint32_t height)
{
    menu->font = cairo_get_scaled_font(menu->cairo);

    menu->glyph_x_offset = extents->max_x_advance;
    menu->glyph_y_offset = 1 + (extents->ascent - extents->descent) / 2.0;

    menu->x = x;
    menu->y = y;
    menu->width = width;
    menu->height = height;

    menu->entry_height = (uint32_t)(1.25 * extents->height);

    printf("menu: (%u, %u) / (%u, %u)\n", x, y, width, height);
}

void menu_up(struct menu *menu)
{
    if (menu->selected <= 0)
        return;

    menu_update_entry(menu, menu->selected--);
    menu_update_entry(menu, menu->selected);
}

void menu_down(struct menu *menu)
{
    if (menu->selected >= menu->n_entries - 1)
        return;

    menu_update_entry(menu, menu->selected++);
    menu_update_entry(menu, menu->selected);
}

static void menu_update(struct menu *menu)
{
    int num;

    /* Save number of currently displayed items. */
    num = menu->n_entries;

    menu_update_entry_list(menu);

    /*
     * The number of items which have to be displayed may have changed.
     * Ensure that _all_ items are updated accordingly.
     */
    num = MAX(num, menu->n_entries);

    for (int i = 0; i < num; ++i)
        menu_update_entry(menu, i);
}

void menu_lookup_push_back(struct menu *menu, int c)
{
    TIMER_INIT_SIMPLE();

    item_list_lookup_push_back(menu->items, c);

    menu_update(menu);
}

void menu_lookup_pop_back(struct menu *menu)
{
    TIMER_INIT_SIMPLE();

    item_list_lookup_pop_back(menu->items);

    menu_update(menu);
}

void menu_lookup_clear(struct menu *menu)
{
    item_list_lookup_clear(menu->items);

    menu_update(menu);
}

void menu_draw(struct menu *menu)
{
    TIMER_INIT_SIMPLE();

    menu_update_entry_list(menu);

    for (int i = 0; i < menu->max_entries; ++i)
        menu_update_entry(menu, i);
}
