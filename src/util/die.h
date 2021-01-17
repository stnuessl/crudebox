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

#ifndef DIE_H_
#define DIE_H_

#include <stdlib.h>
#include <stdio.h>

#define die(...)                                                               \
    do {                                                                       \
        fprintf(stderr, "crudebox: %s: ", __func__);                           \
        fprintf(stderr, __VA_ARGS__);                                          \
                                                                               \
        exit(EXIT_FAILURE);                                                    \
    } while (0)

#endif /* DIE_H_ */
