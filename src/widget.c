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

static inline uint32_t widget_area_x(const struct widget *widget)
{
    return (uint32_t) widget->line;
}

static inline uint32_t widget_area_y(const struct widget *widget)
{
    return (uint32_t) widget->line;
}

static inline uint32_t widget_area_width(const struct widget *widget)
{
    return widget->width - (uint32_t)(2.0 * widget->line);
}

static inline uint32_t widget_area_height(const struct widget *widget)
{
    return widget->height - (uint32_t)(3.0 * widget->line);
}

static void widget_init_cairo(struct widget *widget, cairo_surface_t *surface)
{
    double x1, y1, x2, y2;

    TIMER_INIT_SIMPLE();

    widget->surface = surface;

    widget->cairo = cairo_create(surface);
    if (unlikely(cairo_status(widget->cairo) != CAIRO_STATUS_SUCCESS))
        die("failed to initialize cairo graphics library\n");

    cairo_clip_extents(widget->cairo, &x1, &y1, &x2, &y2);

    widget->width = (uint32_t)(x2 - x1);
    widget->height = (uint32_t)(y2 - y1);
}

static void widget_configure_line_edit(struct widget *widget,
                                       const cairo_font_extents_t *extents)
{
    uint32_t x, y, width, height;

    line_edit_size_hint(&widget->line_edit, extents, &width, &height);

    x = widget_area_x(widget);
    y = widget_area_y(widget);
    width = MIN(width, widget_area_width(widget));
    height = MIN(height, widget_area_height(widget));

    line_edit_configure(&widget->line_edit, extents, x, y, width, height);

    printf("%s: P(%u|%u) Size(%u|%u)\n", __func__, x, y, width, height);
}

static void widget_configure_menu(struct widget *widget,
                                  const cairo_font_extents_t *extents)
{
    uint32_t x, y, width, height, rem;

    menu_size_hint(&widget->menu, extents, &width, &height);

    x = widget_area_x(widget);

    y = widget_area_y(widget);
    y += line_edit_height(&widget->line_edit);
    y += (uint32_t) widget->line;

    /* Width has to be the same as for the line edit element */
    width = line_edit_width(&widget->line_edit);

    rem = widget_area_height(widget) - line_edit_height(&widget->line_edit);
    height = MIN(height, rem);

    menu_configure(&widget->menu, extents, x, y, width, height);

    printf("%s: P(%u|%u) Size(%u|%u)\n", __func__, x, y, width, height);
}

static void widget_clear(struct widget *widget)
{
    TIMER_INIT_SIMPLE();

    line_edit_clear(&widget->line_edit);
    menu_lookup_clear(&widget->menu);
}

static void widget_event_add_char(struct widget *widget, int c)
{
    TIMER_INIT_SIMPLE();

    cairo_push_group(widget->cairo);

    line_edit_push_back(&widget->line_edit, c);
    menu_lookup_push_back(&widget->menu, c);

    cairo_pop_group_to_source(widget->cairo);
    cairo_paint(widget->cairo);
}

static void widget_event_remove_char(struct widget *widget)
{
    TIMER_INIT_SIMPLE();

    cairo_push_group(widget->cairo);

    line_edit_pop_back(&widget->line_edit);
    menu_lookup_pop_back(&widget->menu);

    cairo_pop_group_to_source(widget->cairo);
    cairo_paint(widget->cairo);
}

__attribute__((noreturn)) static void widget_exec_item(struct widget *widget)
{
    const char *file;
    char *argv[2];

    file = menu_get_entry(&widget->menu);

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

    TIMER_INIT_SIMPLE();

    error = FT_Init_FreeType(&widget->freetype);
    if (unlikely(error))
        die("failed to initialize font library\n");

    widget->face = NULL;

    widget_init_cairo(widget, surface);

    line_edit_init(&widget->line_edit, widget->cairo);
    menu_init(&widget->menu, widget->cairo);
}

void widget_destroy(struct widget *widget)
{
#ifdef MEM_NOLEAK
    menu_destroy(&widget->menu);
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
    uint32_t w1, h1, w2, h2;
    double line;

    cairo_font_extents(widget->cairo, &extents);

    line_edit_size_hint(&widget->line_edit, &extents, &w1, &h1);
    menu_size_hint(&widget->menu, &extents, &w2, &h2);

    line = cairo_get_line_width(widget->cairo);

    /* Ensure both sub-widgets have enough width */
    *width = line + MAX(w1, w2) + line;

    /* Ensure both sub-widgets have enough height */
    *height = line + h1 + line + h2 + line;

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

    if (widget->face != NULL)
        FT_Done_Face(widget->face);

    widget->face = ft_face;

    cairo_set_font_face(widget->cairo, cr_face);
    cairo_font_face_destroy(cr_face);
}

void widget_set_font_size(struct widget *widget, double font_size)
{
    cairo_set_font_size(widget->cairo, font_size);
}

void widget_set_line_width(struct widget *widget, double line_width)
{
    widget->line = line_width;
    cairo_set_line_width(widget->cairo, line_width);
}

void widget_set_max_rows(struct widget *widget, int num)
{
    menu_set_max_rows(&widget->menu, num);
}

void widget_set_item_list(struct widget *widget, struct item_list *list)
{
    menu_set_item_list(&widget->menu, list);
}

void widget_draw(struct widget *widget)
{
    uint32_t x, y, width, height;

    TIMER_INIT_SIMPLE();

    /*
     * Calculate geometry of the widget frame.
     * Make sure that the line is centered correctly.
     */
    x = (uint32_t)(widget->line / 2.0);
    y = x;

    /*
     * We have to move the start point of the frame by half a line width
     * into the middle. We also have to move the end point of the frame by
     * half a line width into the middle. The frame width / height is thus
     * the original widget width / height minus two times half the line
     * width.
     */
    width = widget->width - widget->line;
    height = widget->height - widget->line;

    cairo_set_source_rgba(widget->cairo,
                          widget->frame.red,
                          widget->frame.green,
                          widget->frame.blue,
                          widget->frame.alpha);

    cairo_rectangle(widget->cairo, x, y, width, height);

    /* Draw line delimiter between line_edit and menu. */
    y += line_edit_height(&widget->line_edit) + widget->line;
    cairo_move_to(widget->cairo, x, y);

    x += width;
    cairo_line_to(widget->cairo, x, y);

    cairo_stroke(widget->cairo);

    /* Draw individual widgets */
    line_edit_draw(&widget->line_edit);
    menu_draw(&widget->menu);
}

void widget_set_size(struct widget *widget, uint32_t width, uint32_t height)
{
    cairo_font_extents_t extents;

    cairo_xcb_surface_set_size(widget->surface, (int) width, (int) height);

    widget->width = width;
    widget->height = height;

    cairo_font_extents(widget->cairo, &extents);

    widget_configure_line_edit(widget, &extents);
    widget_configure_menu(widget, &extents);
}

void widget_do_key_event(struct widget *widget, struct key_event ev)
{
    TIMER_INIT_SIMPLE();

    if (ev.shift) {
        switch (ev.symbol) {
        case XKB_KEY_Tab:
            menu_up(&widget->menu);
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
    case XKB_KEY_Control_L:
        break;
    case XKB_KEY_ISO_Left_Tab:
    case XKB_KEY_Up:
        menu_up(&widget->menu);
        break;
    case XKB_KEY_Tab:
    case XKB_KEY_Down:
        menu_down(&widget->menu);
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
        widget_event_add_char(widget, ev.symbol);
        break;
    }
}
