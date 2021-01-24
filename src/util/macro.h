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

#ifndef MACRO_H_
#define MACRO_H_

#if __clang__ || __GNUC__
#define likely(x_) (__builtin_expect(!!(x_), 1))
#define unlikely(x_) (__builtin_expect(!!(x_), 0))
#else
#define likely(x_) (x_)
#define unlikely(x_) (x_)
#endif

#define ARRAY_SIZE(x_) ((int) (sizeof((x_)) / sizeof(*(x_))))

#define MIN(a_, b_) (((a_) < (b_)) ? (a_) : (b_))

#define MAX(a_, b_) (((a_) > (b_)) ? (a_) : (b_))

#endif /* MACRO_H_ */
