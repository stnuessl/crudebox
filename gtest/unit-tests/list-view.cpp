/*
 * Copyright (C) 2022   Steffen Nuessle
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fcntl.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <util/macro.h>

#include <list-view.h>

class mock {
public:
    MOCK_METHOD(cairo_antialias_t, cairo_get_antialias, (cairo_t *) );
    MOCK_METHOD(cairo_scaled_font_t *, cairo_get_scaled_font, (cairo_t *) );
    MOCK_METHOD(cairo_status_t,
                cairo_scaled_font_text_to_glyphs,
                (cairo_scaled_font_t *,
                 double,
                 double,
                 const char *,
                 int,
                 cairo_glyph_t **,
                 int *,
                 cairo_text_cluster_t **,
                 int *,
                 cairo_text_cluster_flags_t *) );
    MOCK_METHOD(void, cairo_fill, (cairo_t *) );
    MOCK_METHOD(void, cairo_glyph_free, (cairo_glyph_t *) );
    MOCK_METHOD(void, cairo_line_to, (cairo_t *, double, double) );
    MOCK_METHOD(void, cairo_move_to, (cairo_t *, double, double) );
    MOCK_METHOD(void,
                cairo_rectangle,
                (cairo_t *, double x, double y, double w, double h));
    MOCK_METHOD(void, cairo_set_antialias, (cairo_t *, cairo_antialias_t));
    MOCK_METHOD(void, cairo_set_line_width, (cairo_t *, double) );
    MOCK_METHOD(void,
                cairo_set_source_rgba,
                (cairo_t *, double, double, double, double) );
    MOCK_METHOD(void,
                cairo_show_glyphs,
                (cairo_t *, const cairo_glyph_t *, int) );
    MOCK_METHOD(void, cairo_stroke, (cairo_t *) );

    MOCK_METHOD(void, item_list_lookup_clear, (struct item_list *) );
    MOCK_METHOD(void, item_list_lookup_push_back, (struct item_list *, int) );
    MOCK_METHOD(void, item_list_lookup_pop_back, (struct item_list *) );
};

static testing::StrictMock<mock> mock;

cairo_antialias_t cairo_get_antialias(cairo_t *cairo)
{
    return mock.cairo_get_antialias(cairo);
}

cairo_status_t
cairo_scaled_font_text_to_glyphs(cairo_scaled_font_t *font,
                                 double x,
                                 double y,
                                 const char *utf8,
                                 int utf8_len,
                                 cairo_glyph_t **glyphs,
                                 int *num_glyphs,
                                 cairo_text_cluster_t **clusters,
                                 int *num_clusters,
                                 cairo_text_cluster_flags_t *cluster_flags)
{
    return mock.cairo_scaled_font_text_to_glyphs(font,
                                                 x,
                                                 y,
                                                 utf8,
                                                 utf8_len,
                                                 glyphs,
                                                 num_glyphs,
                                                 clusters,
                                                 num_clusters,
                                                 cluster_flags);
}

cairo_scaled_font_t *cairo_get_scaled_font(cairo_t *cairo)
{
    return mock.cairo_get_scaled_font(cairo);
}

void cairo_fill(cairo_t *cairo)
{
    return mock.cairo_fill(cairo);
}

void cairo_glyph_free(cairo_glyph_t *glyphs)
{
    return mock.cairo_glyph_free(glyphs);
}

void cairo_move_to(cairo_t *cairo, double x, double y)
{
    mock.cairo_move_to(cairo, x, y);
}

void cairo_line_to(cairo_t *cairo, double x, double y)
{
    mock.cairo_line_to(cairo, x, y);
}

void cairo_rectangle(cairo_t *cairo, double x, double y, double w, double h)
{
    mock.cairo_rectangle(cairo, x, y, w, h);
}

void cairo_set_antialias(cairo_t *cairo, cairo_antialias_t antialias)
{
    return mock.cairo_set_antialias(cairo, antialias);
}

void cairo_set_line_width(cairo_t *cairo, double val)
{
    mock.cairo_set_line_width(cairo, val);
}

void cairo_set_source_rgba(cairo_t *cairo,
                           double red,
                           double green,
                           double blue,
                           double alpha)
{
    mock.cairo_set_source_rgba(cairo, red, green, blue, alpha);
}

void cairo_show_glyphs(cairo_t *cairo,
                       const cairo_glyph_t *glyphs,
                       int num_glyphs)
{
    return mock.cairo_show_glyphs(cairo, glyphs, num_glyphs);
}

void cairo_stroke(cairo_t *cairo)
{
    mock.cairo_stroke(cairo);
}

void item_list_lookup_clear(struct item_list *list)
{
    mock.item_list_lookup_clear(list);
}

void item_list_lookup_push_back(struct item_list *list, int c)
{
    mock.item_list_lookup_push_back(list, c);
}

void item_list_lookup_pop_back(struct item_list *list)
{
    mock.item_list_lookup_pop_back(list);
}

#ifdef __cplusplus
}
#endif

TEST(list_view_init, 001)
{
    struct list_view view;

    list_view_init(&view);
}

TEST(list_view_destroy, 001)
{
    struct list_view view;

    list_view_init(&view);
    list_view_destroy(&view);
}

TEST(list_view_size_hint, 001)
{
    struct list_view view;
    cairo_font_extents_t ext;
    uint32_t width, height;

    ext.max_x_advance = 10.0;
    ext.height = 8.0;

    list_view_init(&view);
    list_view_set_max_rows(&view, 1);
    list_view_size_hint(&view, &ext, &width, &height);

    ASSERT_GE(width, ext.max_x_advance);
    ASSERT_GE(height, ext.height);
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
