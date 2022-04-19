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

#include <errno.h>
#include <limits.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <cmocka.h>

#include <util/macro.h>
#include <util/string-util.h>

static void T001_streq(void **state)
{
    int (*func)(const char *, const char *) = &streq;

    (void) state;

    assert_true(func("aa", "aa"));
    assert_true(func("bbb", "bbb"));

    assert_false(func("aa", "bb"));
    assert_false(func("aa", "aaa"));
}

static void T001_strneq(void **state)
{
    int (*func)(const char *, const char *, size_t) = &strneq;

    (void) state;

    assert_true(func("aa", "aa", 1));
    assert_true(func("bb", "bb", 2));
    assert_true(func("ccc", "ccc", 3));

    assert_false(func("aa", "bb", 1));
    assert_false(func("aa", "bb", 2));
    assert_false(func("aa", "aaa", 3));
}

static void T001_strlower(void **state)
{
    char str[] = "AA";

    (void) state;

    assert_string_equal("aa", strlower(str));
    assert_string_equal("aa", str);
}

static void T001_strupper(void **state)
{
    char str[] = "aa";

    (void) state;

    assert_string_equal("AA", strupper(str));
    assert_string_equal("AA", str);
}

static void T001_strprefix(void **state)
{
    (void) state;

    assert_int_equal(0, strprefix("aabb", ""));
    assert_int_equal(0, strprefix("aabb", "a"));
    assert_int_equal(0, strprefix("aabb", "aa"));
    assert_int_equal(0, strprefix("aabb", "aab"));
    assert_int_equal(0, strprefix("aabb", "aabb"));

    assert_int_not_equal(0, strprefix("aabb", "x"));
    assert_int_not_equal(0, strprefix("aabb", "ax"));
    assert_int_not_equal(0, strprefix("aabb", "aax"));
    assert_int_not_equal(0, strprefix("aabb", "aabbx"));
}

static void T001_strsuffix(void **state)
{
    (void) state;

    assert_int_equal(0, strsuffix("aabb", ""));
    assert_int_equal(0, strsuffix("aabb", "b"));
    assert_int_equal(0, strsuffix("aabb", "bb"));
    assert_int_equal(0, strsuffix("aabb", "abb"));
    assert_int_equal(0, strsuffix("aabb", "aabb"));

    assert_int_not_equal(0, strsuffix("aabb", "x"));
    assert_int_not_equal(0, strsuffix("aabb", "bx"));
    assert_int_not_equal(0, strsuffix("aabb", "bbx"));
    assert_int_not_equal(0, strsuffix("aabb", "abbx"));
    assert_int_not_equal(0, strsuffix("aabb", "aabbx"));
}

static void T001_strconcat2a(void **state)
{
    char *result;

    (void) state;

    strconcat2a(&result, "aa", "bb");
    assert_string_equal("aabb", result);

    strconcat2a(&result, "bb", "aa");
    assert_string_equal("bbaa", result);

    strconcat2a(&result, "", "");
    assert_string_equal("", result);

    strconcat2a(&result, "aa", "bb");
    assert_string_not_equal("aab", result);

    strconcat2a(&result, "bb", "aa");
    assert_string_not_equal("baa", result);
}

static void T001_strconcat(void **state)
{
    const char *strings[] = {"aa", "bb", "cc", "dd"};
    char *str;

    (void) state;

    str = strconcat(strings, ARRAY_SIZE(strings));

    assert_string_equal("aabbccdd", str);

    free(str);
}

static void T002_strconcat(void **state)
{
    const char *strings[] = {"", "bb", "", ""};
    char *str;

    (void) state;

    str = strconcat(strings, ARRAY_SIZE(strings));

    assert_string_equal("bb", str);

    free(str);
}

static void T003_strconcat(void **state)
{
    char *str;

    (void) state;

    str = strconcat(NULL, 0);

    assert_string_equal("", str);

    free(str);
}

static void T001_strconcat2(void **state)
{
    char *str;

    (void) state;

    str = strconcat2("aabb", "cc");
    assert_string_equal("aabbcc", str);
    free(str);

    str = strconcat2("", "");
    assert_string_equal("", str);
    free(str);
}

static void T001_strnconcat(void **state)
{
    const char *strings[] = {"aa", "bb", "cc", "dd"};
    char buf[16], *str;

    (void) state;

    str = strnconcat(buf, ARRAY_SIZE(buf), strings, ARRAY_SIZE(strings));

    assert_ptr_equal(str, buf);
    assert_string_equal("aabbccdd", str);
}

static void T002_strnconcat(void **state)
{
    const char *strings[] = {"ww", "xx", "yy", "zz"};
    char buf[8], *str;

    (void) state;

    str = strnconcat(buf, ARRAY_SIZE(buf), strings, ARRAY_SIZE(strings));

    assert_ptr_equal(str, buf);
    assert_string_equal("wwxxyyz", str);
}

static void T003_strnconcat(void **state)
{
    const char *strings[] = {"aa", "", "cc", "dd"};
    char buf[5], *str;

    (void) state;

    str = strnconcat(buf, ARRAY_SIZE(buf), strings, ARRAY_SIZE(strings));

    assert_ptr_equal(str, buf);
    assert_string_equal("aacc", str);
}

static void T004_strnconcat(void **state)
{
    const char *strings[] = {"", "", "", ""};
    char buf[2], *str;

    (void) state;

    str = strnconcat(buf, ARRAY_SIZE(buf), strings, ARRAY_SIZE(strings));

    assert_ptr_equal(str, buf);
    assert_string_equal("", str);
}

static void T001_strnconcat2(void **state)
{
    char buf[8], *str;

    (void) state;

    str = strnconcat2(buf, ARRAY_SIZE(buf), "aa", "bb");

    assert_ptr_equal(str, buf);
    assert_string_equal("aabb", str);
}

static void T002_strnconcat2(void **state)
{
    char buf[2], *str;

    (void) state;

    str = strnconcat2(buf, ARRAY_SIZE(buf), "yy", "zz");

    assert_ptr_equal(str, buf);
    assert_string_equal("y", str);
}

static const struct CMUnitTest tests[] = {
    cmocka_unit_test(T001_streq),
    cmocka_unit_test(T001_strneq),
    cmocka_unit_test(T001_strlower),
    cmocka_unit_test(T001_strupper),
    cmocka_unit_test(T001_strprefix),
    cmocka_unit_test(T001_strsuffix),
    cmocka_unit_test(T001_strconcat2a),
    cmocka_unit_test(T001_strconcat),
    cmocka_unit_test(T002_strconcat),
    cmocka_unit_test(T003_strconcat),
    cmocka_unit_test(T001_strconcat2),
    cmocka_unit_test(T001_strnconcat),
    cmocka_unit_test(T002_strnconcat),
    cmocka_unit_test(T003_strnconcat),
    cmocka_unit_test(T004_strnconcat),
    cmocka_unit_test(T001_strnconcat2),
    cmocka_unit_test(T002_strnconcat2),
};

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    return cmocka_run_group_tests_name(__FILE__, tests, NULL, NULL);
}
