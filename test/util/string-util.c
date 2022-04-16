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

#include <string.h>

#include <criterion/criterion.h>

#include <util/macro.h>
#include <util/string-util.h>

Test(string_util, 001_streq)
{
    int (*func)(const char *, const char *) = &streq;

    cr_assert(func("aa", "aa"));
    cr_assert(func("bbb", "bbb"));

    cr_assert(!func("aa", "bb"));
    cr_assert(!func("aa", "aaa"));
}

Test(string_util, 001_strneq)
{
    int (*func)(const char *, const char *, size_t) = &strneq;

    cr_assert(func("aa", "aa", 1));
    cr_assert(func("bb", "bb", 2));
    cr_assert(func("ccc", "ccc", 3));

    cr_assert(!func("aa", "bb", 1));
    cr_assert(!func("aa", "bb", 2));
    cr_assert(!func("aa", "aaa", 3));
}

Test(string_util, 001_strlower)
{
    char str[] = "AA";

    cr_assert(strcmp(strlower(str), "aa") == 0);
    cr_assert(strcmp(str, "aa") == 0);
}

Test(string_util, 001_strupper)
{
    char str[] = "aa";

    cr_assert(strcmp(strupper(str), "AA") == 0);
    cr_assert(strcmp(str, "AA") == 0);
}

Test(string_util, 001_strprefix)
{
    cr_assert(strprefix("aabb", "") == 0);
    cr_assert(strprefix("aabb", "a") == 0);
    cr_assert(strprefix("aabb", "aa") == 0);
    cr_assert(strprefix("aabb", "aab") == 0);
    cr_assert(strprefix("aabb", "aabb") == 0);

    cr_assert(strprefix("aabb", "x") != 0);
    cr_assert(strprefix("aabb", "ax") != 0);
    cr_assert(strprefix("aabb", "aax") != 0);
    cr_assert(strprefix("aabb", "aabbx") != 0);
}

Test(string_util, 001_strsuffix)
{
    cr_assert(strsuffix("aabb", "") == 0);
    cr_assert(strsuffix("aabb", "b") == 0);
    cr_assert(strsuffix("aabb", "bb") == 0);
    cr_assert(strsuffix("aabb", "abb") == 0);
    cr_assert(strsuffix("aabb", "aabb") == 0);

    cr_assert(strsuffix("aabb", "x") != 0);
    cr_assert(strsuffix("aabb", "bx") != 0);
    cr_assert(strsuffix("aabb", "bbx") != 0);
    cr_assert(strsuffix("aabb", "abbx") != 0);
    cr_assert(strsuffix("aabb", "aabbx") != 0);
}

Test(string_util, 001_strconcat2a)
{
    char *result;

    strconcat2a(&result, "aa", "bb");
    cr_assert(strcmp(result, "aabb") == 0);

    strconcat2a(&result, "bb", "aa");
    cr_assert(strcmp(result, "bbaa") == 0);

    strconcat2a(&result, "", "");
    cr_assert(strcmp(result, "") == 0);

    strconcat2a(&result, "aa", "bb");
    cr_assert(strcmp(result, "aab") != 0);

    strconcat2a(&result, "bb", "aa");
    cr_assert(strcmp(result, "baa") != 0);
}

Test(string_util, 001_strconcat)
{
    const char *strings[] = {"aa", "bb", "cc", "dd"};
    char *str;

    str = strconcat(strings, ARRAY_SIZE(strings));

    cr_assert(strcmp(str, "aabbccdd") == 0);

    free(str);
}

Test(string_util, 002_strconcat)
{
    const char *strings[] = {"", "bb", "", ""};
    char *str;

    str = strconcat(strings, ARRAY_SIZE(strings));

    cr_assert(strcmp(str, "bb") == 0);

    free(str);
}

Test(string_util, 003_strconcat)
{
    char *str = strconcat(NULL, 0);

    cr_assert(strcmp(str, "") == 0);

    free(str);
}

Test(string_util, 001_strconcat2)
{
    char *str;

    str = strconcat2("aabb", "cc");
    cr_assert(strcmp(str, "aabbcc") == 0);
    free(str);

    str = strconcat2("", "");
    cr_assert(strcmp(str, "") == 0);
    free(str);
}

Test(string_util, 001_strnconcat)
{
    const char *strings[] = {"aa", "bb", "cc", "dd"};
    char buf[16], *str;

    str = strnconcat(buf, ARRAY_SIZE(buf), strings, ARRAY_SIZE(strings));

    cr_assert(str == buf);
    cr_assert(strcmp(str, "aabbccdd") == 0);
}

Test(string_util, 002_strnconcat)
{
    const char *strings[] = {"ww", "xx", "yy", "zz"};
    char buf[8], *str;

    str = strnconcat(buf, ARRAY_SIZE(buf), strings, ARRAY_SIZE(strings));

    cr_assert(str == buf);
    cr_assert(strcmp(str, "wwxxyyz") == 0);
}

Test(string_util, 003_strnconcat)
{
    const char *strings[] = {"aa", "", "cc", "dd"};
    char buf[5], *str;

    str = strnconcat(buf, ARRAY_SIZE(buf), strings, ARRAY_SIZE(strings));

    cr_assert(str == buf);
    cr_assert(strcmp(str, "aacc") == 0);
}

Test(string_util, 004_strnconcat)
{
    const char *strings[] = {"", "", "", ""};
    char buf[2], *str;

    str = strnconcat(buf, ARRAY_SIZE(buf), strings, ARRAY_SIZE(strings));

    cr_assert(str == buf);
    cr_assert(strcmp(str, "") == 0);
}

Test(string_util, 001_strnconcat2)
{
    char buf[8], *str;

    str = strnconcat2(buf, ARRAY_SIZE(buf), "aa", "bb");

    cr_assert(str == buf);
    cr_assert(strcmp(str, "aabb") == 0);
}

Test(string_util, 002_strnconcat2)
{
    char buf[2], *str;

    str = strnconcat2(buf, ARRAY_SIZE(buf), "yy", "zz");

    cr_assert(str == buf);
    cr_assert(strcmp(str, "y") == 0);
}
