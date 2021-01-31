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

#ifndef WIDGET_COMMON_H_
#define WIDGET_COMMON_H_

#include <stdint.h>

struct color {
    double red;
    double green;
    double blue;
    double alpha;
};

static inline void color_set_red(struct color *c, uint32_t rgba)
{
    c->red = ((double) ((rgba >> 24) & 0xff) / 255.0);
}

static inline void color_set_green(struct color *c, uint32_t rgba)
{
    c->green = ((double) ((rgba >> 16) & 0xff) / 255.0);
}

static inline void color_set_blue(struct color *c, uint32_t rgba)
{
    c->blue = ((double) ((rgba >> 8) & 0xff) / 255.0);
}

static inline void color_set_alpha(struct color *c, uint32_t rgba)
{
    c->alpha = ((double) (rgba & 0xff) / 255.0);
}

static inline void color_set_u32(struct color *c, uint32_t rgba)
{
    color_set_red(c, rgba);
    color_set_green(c, rgba);
    color_set_blue(c, rgba);
    color_set_alpha(c, rgba);
}

#endif /* WIDGET_COMMON_H_ */
