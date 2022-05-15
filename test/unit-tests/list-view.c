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

#include <list-view.h>

class mock {
public:
};

#ifdef __cplusplus
}
#endif

static testing::StrictMock<mock> mock;

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
    list_view_size_hint(&view, &ext, &width, &height);

    ASSERT_GE(width, 10.0);
    ASSERT_EQ(height, 0);
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
