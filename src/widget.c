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

#include <stdint.h>
#include <unistd.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MODULE_H

#include <cairo-ft.h>
#include <cairo-xcb.h>

#include "timer.h"
#include "widget.h"

#include "util/die.h"
#include "util/macro.h"

static void widget_configure_line_edit(struct widget *widget,
                                       const cairo_font_extents_t *extents)
{
    uint32_t x1, y1, x2, y2, w, h;

    line_edit_size_hint(&widget->line_edit, extents, &w, &h);

    /* Calculate geometry of the widget without the frame */
    x1 = widget->line_width;
    y1 = x1;
    x2 = widget->width - widget->line_width;
    y2 = widget->height - widget->line_width;

    x2 = MIN(x2, x1 + w);
    y2 = MIN(y2, y1 + h);

    line_edit_configure(&widget->line_edit, extents, x1, y1, x2, y2);
}

static void widget_configure_list_view(struct widget *widget,
                                       const cairo_font_extents_t *extents)
{
    uint32_t x1, y1, x2, y2;

    x1 = widget->line_width;
    y1 = 3 * widget->line_width + line_edit_height(&widget->line_edit);
    x2 = x1 + line_edit_width(&widget->line_edit);
    y2 = widget->height - widget->line_width;

    list_view_configure(&widget->list_view, extents, x1, y1, x2, y2);
}

static void widget_clear(struct widget *widget)
{
    TIMER_INIT_SIMPLE();

    line_edit_clear(&widget->line_edit);
    list_view_lookup_clear(&widget->list_view);
}

static void widget_event_add_char(struct widget *widget, int c)
{
    TIMER_INIT_SIMPLE();

    cairo_push_group(widget->cairo);

    line_edit_push_back(&widget->line_edit, c);
    list_view_lookup_push_back(&widget->list_view, c);

    cairo_pop_group_to_source(widget->cairo);
    cairo_paint(widget->cairo);
}

static void widget_event_remove_char(struct widget *widget)
{
    TIMER_INIT_SIMPLE();

    cairo_push_group(widget->cairo);

    line_edit_pop_back(&widget->line_edit);
    list_view_lookup_pop_back(&widget->list_view);

    cairo_pop_group_to_source(widget->cairo);
    cairo_paint(widget->cairo);
}

__attribute__((noreturn)) static void widget_exec_item(struct widget *widget)
{
    const char *file;
    char *argv[2];

    file = list_view_get_entry(&widget->list_view);
    if (!file)
        exit(EXIT_SUCCESS);

    if (widget->print) {
        fprintf(stdout, "%s\n", file);
        exit(EXIT_SUCCESS);
    }

    /* Values in 'argv' cannot be assigned to a 'const' pointer. */
    argv[0] = strdupa(file);
    argv[1] = NULL;

    execvp(file, argv);

    die("failed to execute \"%s\"\n", file);
}

void widget_init(struct widget *widget, cairo_surface_t *surface)
{
    FT_Error error;
    double x1, y1, x2, y2;

    TIMER_INIT_SIMPLE();

    error = FT_Init_FreeType(&widget->freetype);
    if (unlikely(error))
        die("failed to initialize font library\n");

    widget->surface = surface;

    widget->cairo = cairo_create(surface);
    if (unlikely(cairo_status(widget->cairo) != CAIRO_STATUS_SUCCESS))
        die("failed to initialize cairo graphics library\n");

    cairo_clip_extents(widget->cairo, &x1, &y1, &x2, &y2);

    widget->width = (uint32_t)(x2 - x1);
    widget->height = (uint32_t)(y2 - y1);

    line_edit_init(&widget->line_edit, widget->cairo);
    list_view_init(&widget->list_view, widget->cairo);
}

void widget_destroy(struct widget *widget)
{
#ifdef MEM_NOLEAK
    list_view_destroy(&widget->list_view);
    line_edit_destroy(&widget->line_edit);
    cairo_destroy(widget->cairo);
    FT_Done_FreeType(widget->freetype);
#else
    (void) widget;
#endif
}

void widget_get_size_hint(const struct widget *widget,
                          uint32_t *width,
                          uint32_t *height)
{
    cairo_font_extents_t extents;
    uint32_t w1, h1, w2, h2, w3, h3;

    cairo_font_extents(widget->cairo, &extents);

    w1 = 2 * widget->line_width;
    h1 = 4 * widget->line_width;

    line_edit_size_hint(&widget->line_edit, &extents, &w2, &h2);
    list_view_size_hint(&widget->list_view, &extents, &w3, &h3);

    /* Ensure both sub-widgets have enough width */
    *width = w1 + MAX(w2, w3);

    /* Ensure both sub-widgets have enough height */
    *height = h1 + h2 + h3;

    printf("font height: %lf\n", extents.height);

    printf("Hint: (%u, %u)\n", *width, *height);
}

void widget_set_font(struct widget *widget, const char *path)
{
    FT_Face ft_face;
    FT_Error error;
    cairo_font_face_t *cr_face;

    error = FT_New_Face(widget->freetype, path, 0, &ft_face);
    if (error)
        die("failed to create freetype font face for \"%s\"\n", path);

    cr_face = cairo_ft_font_face_create_for_ft_face(ft_face, FT_LOAD_DEFAULT);
    if (cairo_font_face_status(cr_face) != CAIRO_STATUS_SUCCESS)
        die("failed to create cairo font face for \"%s\"\n", path);

    if (widget->face)
        FT_Done_Face(widget->face);

    widget->face = ft_face;

    cairo_set_font_face(widget->cairo, cr_face);
    cairo_font_face_destroy(cr_face);
}

void widget_set_max_rows(struct widget *widget, int num)
{
    list_view_set_max_rows(&widget->list_view, num);
}

void widget_set_item_list(struct widget *widget, struct item_list *list)
{
    list_view_set_item_list(&widget->list_view, list);
}

void widget_draw(struct widget *widget)
{
    uint32_t x, y, w, h;

    TIMER_INIT_SIMPLE();

    /* Draw the frame surrounding the widget. */
    x = widget->line_width / 2;
    y = x;
    w = widget->width - widget->line_width;
    h = widget->height - widget->line_width;

    cairo_set_line_width(widget->cairo, widget->line_width);
    cairo_rectangle(widget->cairo, x, y, w, h);

    /* Draw frame between the line-edit and list-view elements. */
    y += widget->line_width + line_edit_height(&widget->line_edit);
    h = widget->line_width;

    cairo_rectangle(widget->cairo, x, y, w, h);
    cairo_set_source_rgba(widget->cairo,
                          widget->frame.red,
                          widget->frame.green,
                          widget->frame.blue,
                          widget->frame.alpha);

    cairo_stroke(widget->cairo);

    /* Draw individual widgets */
    line_edit_draw(&widget->line_edit);
    list_view_draw(&widget->list_view);
}

void widget_set_size(struct widget *widget, uint32_t width, uint32_t height)
{
    cairo_font_extents_t extents;

    cairo_xcb_surface_set_size(widget->surface, (int) width, (int) height);

    widget->width = width;
    widget->height = height;

    cairo_font_extents(widget->cairo, &extents);

    widget_configure_line_edit(widget, &extents);
    widget_configure_list_view(widget, &extents);
}

void widget_do_key_event(struct widget *widget, struct key_event ev)
{
    TIMER_INIT_SIMPLE();

    if (ev.shift) {
        switch (ev.symbol) {
        case XKB_KEY_Tab:
            list_view_up(&widget->list_view);
            break;
        default:
            break;
        }

        return;
    }

    if (ev.ctrl) {
        switch (ev.symbol) {
        case XKB_KEY_w:
            widget_clear(widget);
            break;
        case XKB_KEY_c:
            exit(EXIT_SUCCESS);
            break;
        default:
            break;
        }

        return;
    }

    switch (ev.symbol) {
    case XKB_KEY_Escape:
        exit(EXIT_SUCCESS);
        break;
    case XKB_KEY_Control_L:
        break;
    case XKB_KEY_Home:
        /* FALLTHROUGH */
    case XKB_KEY_Page_Up:
        list_view_select_first(&widget->list_view);
        break;
    case XKB_KEY_End:
        /* FALLTHROUGH */
    case XKB_KEY_Page_Down:
        list_view_select_last(&widget->list_view);
        break;
    case XKB_KEY_ISO_Left_Tab:
    case XKB_KEY_Up:
        list_view_up(&widget->list_view);
        break;
    case XKB_KEY_Tab:
    case XKB_KEY_Down:
        list_view_down(&widget->list_view);
        break;
    case XKB_KEY_Return:
        widget_exec_item(widget);
        break;
    case XKB_KEY_BackSpace:
        widget_event_remove_char(widget);
        break;
    case XKB_KEY_NoSymbol:
        break;
    default:
        if (isascii(ev.symbol))
            widget_event_add_char(widget, ev.symbol);

        break;
    }
}
