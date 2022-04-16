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

#ifndef STRING_UTIL_H_
#define STRING_UTIL_H_

#include <alloca.h>
#include <ctype.h>
#include <string.h>

#define strconcat2a(dst_, s1_, s2_)                                            \
    do {                                                                       \
        const char *s1 = (s1_);                                                \
        const char *s2 = (s2_);                                                \
        size_t len1 = strlen(s1);                                              \
        size_t len2 = strlen(s2) + 1;                                          \
        char *dst = alloca(len1 + len2);                                       \
                                                                               \
        memcpy(dst, s1, len1);                                                 \
        memcpy(dst + len1, s2, len2);                                          \
                                                                               \
        *(dst_) = dst;                                                         \
    } while (0)

static inline int streq(const char *s1, const char *s2)
{
    return strcmp(s1, s2) == 0;
}

static inline int strneq(const char *s1, const char *s2, size_t n)
{
    return strncmp(s1, s2, n) == 0;
}

static inline char *strlower(char *s)
{
    for (char *p = s; *p != '\0'; ++p)
        *p = tolower(*p);

    return s;
}

static inline char *strupper(char *s)
{
    for (char *p = s; *p != '\0'; ++p)
        *p = toupper(*p);

    return s;
}

static inline int strprefix(const char *s, const char *prefix)
{
    return strncmp(s, prefix, strlen(prefix));
}

static inline int strsuffix(const char *s, const char *suffix)
{
    size_t n1, n2;

    n1 = strlen(s);
    n2 = strlen(suffix);

    if (n1 < n2)
        return '\0' - suffix[n1];

    return strcmp(s + n1 - n2, suffix);
}

char *strconcat(const char **array, int size);

char *strconcat2(const char *s1, const char *s2);

char *strnconcat(char *buf, size_t buf_size, const char **array, int size);

char *strnconcat2(char *buf, size_t buf_size, const char *s1, const char *s2);

#endif /* STRING_UTIL_H_ */
