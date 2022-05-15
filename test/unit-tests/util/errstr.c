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

#include <cmocka.h>

#include <util/errstr.h>

static void T001_errstr(void **state)
{
    (void) state;

    assert_string_equal("OK", errstr(0));
    assert_string_equal("EPERM", errstr(EPERM));
    assert_string_equal("ENOENT", errstr(-ENOENT));
}

static void T002_errstr(void **state)
{
    (void) state;

    assert_null(errstr(INT_MIN));
    assert_null(errstr(INT_MAX));
}

static const struct CMUnitTest tests[] = {
    cmocka_unit_test(T001_errstr),
    cmocka_unit_test(T002_errstr),
};

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    return cmocka_run_group_tests_name(__FILE__, tests, NULL, NULL);
}
