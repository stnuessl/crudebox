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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "line-edit.h"
#include "timer.h"

#include "util/die.h"
#include "util/macro.h"

static void line_edit_update_background(struct line_edit *edit)
{
    uint32_t x, y, w, h;

    x = edit->x1;
    y = edit->y1;
    w = edit->x2 - x;
    h = edit->y2 - y;

    cairo_rectangle(edit->cairo, x, y, w, h);

    cairo_set_source_rgba(edit->cairo,
                          edit->bg.red,
                          edit->bg.green,
                          edit->bg.blue,
                          edit->bg.alpha);

    cairo_fill(edit->cairo);
}

static void line_edit_update_glyphs(struct line_edit *edit)
{
    cairo_glyph_t *glyphs;
    int n_glyphs;
    cairo_status_t status;
    bool cursor;

    TIMER_INIT_SIMPLE();

    cursor = edit->strlen < ARRAY_SIZE(edit->str);
    if (cursor)
        edit->str[edit->strlen++] = '_';

    glyphs = edit->glyphs;
    n_glyphs = ARRAY_SIZE(edit->glyphs);

    status = cairo_scaled_font_text_to_glyphs(edit->font,
                                              edit->glyph_x,
                                              edit->glyph_y,
                                              edit->str,
                                              edit->strlen,
                                              &glyphs,
                                              &n_glyphs,
                                              NULL,
                                              NULL,
                                              NULL);

    if (unlikely(status != CAIRO_STATUS_SUCCESS))
        die("failed to retrieve glyphs from text\n");

    cairo_set_source_rgba(edit->cairo,
                          edit->fg.red,
                          edit->fg.green,
                          edit->fg.blue,
                          edit->fg.alpha);

    cairo_show_glyphs(edit->cairo, glyphs, n_glyphs);

    if (cursor)
        --edit->strlen;

    if (glyphs != edit->glyphs)
        cairo_glyph_free(glyphs);
}

void line_edit_init(struct line_edit *edit)
{
    memset(edit, 0, sizeof(*edit));
}

void line_edit_destroy(struct line_edit *edit)
{
    (void) edit;
}

void line_edit_size_hint(const struct line_edit *edit,
                         const cairo_font_extents_t *ext,
                         uint32_t *width,
                         uint32_t *height)
{
    uint32_t w, h;

    /*
     * We want to have same space between the glyphs and the window borders.
     * This means we have to reserve space for all the glyphs in the text
     * buffer plus two additional non-visible characters for space padding.
     */
    w = (uint32_t) ((ARRAY_SIZE(edit->str) + 2) * ext->max_x_advance);
    h = 1 + (uint32_t) (1.5 * ext->height);

    *width = w;
    *height = h;
}

void line_edit_configure(struct line_edit *edit,
                         const cairo_font_extents_t *ext,
                         uint32_t x1,
                         uint32_t y1,
                         uint32_t x2,
                         uint32_t y2)
{
    uint32_t mid;

    edit->x1 = x1;
    edit->y1 = y1;
    edit->x2 = x2;
    edit->y2 = y2;

    edit->font = cairo_get_scaled_font(edit->cairo);

    mid = (edit->y2 - edit->y1 + ext->ascent - ext->descent) / 2.0;

    edit->glyph_x = edit->x1 + (uint32_t) ext->max_x_advance;
    edit->glyph_y = edit->y1 + mid;
}

void line_edit_clear(struct line_edit *edit)
{
    edit->strlen = 0;

    line_edit_update_background(edit);
    line_edit_update_glyphs(edit);
}

void line_edit_push_back(struct line_edit *edit, int c)
{
    if (edit->strlen >= ARRAY_SIZE(edit->str) || !isascii(c))
        return;

    edit->str[edit->strlen++] = (char) c;

    line_edit_update_background(edit);
    line_edit_update_glyphs(edit);
}

void line_edit_pop_back(struct line_edit *edit)
{
    if (!edit->strlen)
        return;

    --edit->strlen;

    line_edit_update_background(edit);
    line_edit_update_glyphs(edit);
}

void line_edit_draw(struct line_edit *edit)
{
    TIMER_INIT_SIMPLE();

    line_edit_update_background(edit);
    line_edit_update_glyphs(edit);
}
