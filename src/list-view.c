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

#include "list-view.h"
#include "timer.h"

#include "util/macro.h"

static inline const struct color *list_view_get_fg(const struct list_view *view,
                                                   int index)
{
    if (view->selected == index)
        return &view->fg_sel;

    return &view->fg;
}

static inline const struct color *list_view_get_bg(const struct list_view *view,
                                                   int index)
{
    if (view->selected == index)
        return view->bg_sel + (index & 0x01);

    return view->bg + (index & 0x01);
}

static void list_view_update_entry_list(struct list_view *view)
{
    const struct item *item, *end;
    int n, score;

    item = item_list_cbegin(view->items);
    end = item_list_cend(view->items);
    n = 0;
    score = item_list_lookup_score(view->items);

    while (n < view->max_entries && item != end) {
        if (item->score == score)
            view->entries[n++] = item->name;

        ++item;
    }

    view->n_entries = n;

    /*
     * Ensure that the selected entry is always within the allowed
     * range. If there can be no selected entry because the list_view does not
     * display any items at all, the value of the selected entry has to
     * be brought back to zero as soon as entries are available again.
     */
    if (view->selected >= view->n_entries)
        view->selected = view->n_entries - 1;
    else if (view->selected < 0 && view->n_entries > 0)
        view->selected = 0;
}

static void list_view_draw_lines(struct list_view *view)
{
    cairo_antialias_t antialias;
    uint32_t h;

    antialias = cairo_get_antialias(view->cairo);

    cairo_set_antialias(view->cairo, CAIRO_ANTIALIAS_NONE);
    cairo_set_line_width(view->cairo, 1.0);

    h = 1 + (view->y2 - view->y1) / view->max_entries;

    for (uint32_t y = view->y1 + h; y < view->y2; y += h) {
        cairo_move_to(view->cairo, view->x1, y);
        cairo_line_to(view->cairo, view->x2, y);
    }

    cairo_set_source_rgba(view->cairo,
                          view->lines.red,
                          view->lines.green,
                          view->lines.blue,
                          view->lines.alpha);

    cairo_stroke(view->cairo);

    cairo_set_antialias(view->cairo, antialias);
}

static void list_view_update_entry_bg(struct list_view *view, int index)
{
    const struct color *bg = list_view_get_bg(view, index);
    uint32_t x, y, w, h;

    w = view->x2 - view->x1;
    h = (view->y2 - view->y1) / view->max_entries;

    x = view->x1;
    y = view->y1 + index * (h + 1);

    cairo_rectangle(view->cairo, x, y, w, h);
    cairo_set_source_rgba(view->cairo, bg->red, bg->green, bg->blue, bg->alpha);
    cairo_fill(view->cairo);
}

static void list_view_update_entry_fg(struct list_view *view, int index)
{
    const struct color *fg = list_view_get_fg(view, index);
    cairo_glyph_t *glyphs = view->glyphs;
    int n_glyphs = ARRAY_SIZE(view->glyphs);
    uint32_t x, y;
    cairo_status_t status;

    x = view->glyph_x;
    y = view->glyph_y + index * (1 + (view->y2 - view->y1) / view->max_entries);

    status = cairo_scaled_font_text_to_glyphs(view->font,
                                              x,
                                              y,
                                              view->entries[index],
                                              strlen(view->entries[index]),
                                              &glyphs,
                                              &n_glyphs,
                                              NULL,
                                              NULL,
                                              NULL);

    if (unlikely(status != CAIRO_STATUS_SUCCESS))
        die("failed to update text glyphs\n");

    cairo_set_source_rgba(view->cairo, fg->red, fg->green, fg->blue, fg->alpha);

    cairo_show_glyphs(view->cairo, glyphs, n_glyphs);

    if (glyphs != view->glyphs)
        cairo_glyph_free(glyphs);
}

static void list_view_update_entry(struct list_view *view, int index)
{
    if (index < 0)
        return;

    list_view_update_entry_bg(view, index);

    if (index >= view->n_entries)
        return;

    list_view_update_entry_fg(view, index);
}

static void list_view_update(struct list_view *view)
{
    int num;

    TIMER_INIT_SIMPLE();

    /* Save number of currently displayed items. */
    num = view->n_entries;

    list_view_update_entry_list(view);

    /*
     * The number of items which have to be displayed may have changed.
     * Ensure that _all_ items are updated accordingly.
     */
    num = MAX(num, view->n_entries);

    for (int i = 0; i < num; ++i)
        list_view_update_entry(view, i);
}

void list_view_init(struct list_view *view)
{
    memset(view, 0, sizeof(*view));
}

void list_view_destroy(struct list_view *view)
{
#ifdef MEM_NOLEAK
    free(view->entries);
#else
    (void) view;
#endif
}

void list_view_size_hint(const struct list_view *view,
                         const cairo_font_extents_t *extents,
                         uint32_t *width,
                         uint32_t *height)
{
    uint32_t w, h;

    w = (uint32_t) (ARRAY_SIZE(view->glyphs) * extents->max_x_advance);
    h = view->max_entries * (uint32_t) (1.25 * extents->height);

    /* Account for a separation line between all entries. */
    h += view->max_entries - 1;

    *width = w;
    *height = h;
}

void list_view_configure(struct list_view *view,
                         const cairo_font_extents_t *ext,
                         uint32_t x1,
                         uint32_t y1,
                         uint32_t x2,
                         uint32_t y2)
{
    double val;

    view->font = cairo_get_scaled_font(view->cairo);

    view->x1 = x1;
    view->y1 = y1;
    view->x2 = x2;
    view->y2 = y2;

    val = ext->ascent - ext->descent;
    val += (view->y2 - view->y1) / view->max_entries;

    view->glyph_x = view->x1 + (uint32_t) ext->max_x_advance;
    view->glyph_y = view->y1 + (uint32_t) (1 + val / 2.0);
}

void list_view_up(struct list_view *view)
{
    int prev;

    if (view->selected <= 0)
        return;

    prev = view->selected--;

    list_view_update_entry(view, prev);
    list_view_update_entry(view, view->selected);
}

void list_view_down(struct list_view *view)
{
    int prev;

    if (view->selected >= view->n_entries - 1)
        return;

    prev = view->selected++;

    list_view_update_entry(view, prev);
    list_view_update_entry(view, view->selected);
}

void list_view_select_first(struct list_view *view)
{
    int prev;

    if (view->selected <= 0)
        return;

    prev = view->selected;
    view->selected = 0;

    list_view_update_entry(view, prev);
    list_view_update_entry(view, view->selected);
}

void list_view_select_last(struct list_view *view)
{
    int prev;

    if (view->selected >= view->n_entries - 1)
        return;

    prev = view->selected;
    view->selected = view->n_entries - 1;

    list_view_update_entry(view, prev);
    list_view_update_entry(view, view->selected);
}

void list_view_lookup_push_back(struct list_view *view, int c)
{
    TIMER_INIT_SIMPLE();

    item_list_lookup_push_back(view->items, c);

    list_view_update(view);
}

void list_view_lookup_pop_back(struct list_view *view)
{
    TIMER_INIT_SIMPLE();

    item_list_lookup_pop_back(view->items);

    list_view_update(view);
}

void list_view_lookup_clear(struct list_view *view)
{
    item_list_lookup_clear(view->items);

    list_view_update(view);
}

void list_view_draw(struct list_view *view)
{
    TIMER_INIT_SIMPLE();

    list_view_draw_lines(view);
    list_view_update_entry_list(view);

    for (int i = 0; i < view->max_entries; ++i)
        list_view_update_entry(view, i);
}
