/*
 * Copyright (C) 2020   Steffen Nuessle
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

#ifndef XALLOC_H_
#define XALLOC_H_

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "die.h"
#include "macro.h"

static inline void *xmalloc(size_t n)
{
    void *mem = malloc(n);
    if (unlikely(!mem))
        die("failed to allocate memory\n");

    return mem;
}

static inline void *xcalloc(size_t n, size_t size)
{
    void *mem = calloc(n, size);
    if (unlikely(!mem))
        die("failed to allocate memory\n");

    return mem;
}

static inline void *xrealloc(void *mem, size_t n)
{
    mem = realloc(mem, n);
    if (unlikely(!mem))
        die("failed to allocate memory\n");

    return mem;
}

static inline char *xstrdup(const char *s)
{
    char *dup = strdup(s);
    if (unlikely(!dup))
        die("failed to allocate memory\n");

    return dup;
}

#endif /* XALLOC_H_ */
