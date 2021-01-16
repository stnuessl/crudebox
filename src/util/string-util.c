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

#include <stdlib.h>

#include "string-util.h"

char *strconcat(const char **array, int size)
{
    char *s;
    size_t n = 1;

    for (int i = 0; i < size; ++i)
        n += strlen(array[i]);

    s = malloc(n);
    if (!s)
        return NULL;

    n = 0;

    for (int i = 0; i < size; ++i) {
        size_t len = strlen(array[i]);

        memcpy(s + n, array[i], len);
        n += len;
    }

    s[n] = '\0';

    return s;
}

char *strconcat2(const char *s1, const char *s2)
{
    const char *args[] = {s1, s2};

    return strconcat(args, 2);
}

char *strnconcat(char *buf, size_t buf_size, const char **array, int size)
{
    size_t n = 0;

    for (int i = 0; i < size && n < buf_size; ++i) {
        size_t len = strlen(array[i]);

        if (n + len >= buf_size)
            len = buf_size - n;

        memcpy(buf + n, array[i], len);
        n += len;
    }

    if (n >= buf_size)
        n = buf_size - 1;

    buf[n] = '\0';

    return buf;
}

char *strnconcat2(char *buf, size_t buf_size, const char *s1, const char *s2)
{
    const char *args[] = {s1, s2};

    return strnconcat(buf, buf_size, args, 2);
}
