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

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <cmocka.h>

#include <util/env.h>
#include <util/macro.h>

static void T001_crudebox_env(void **state)
{
    const char *(*table[])(void) = {
        &env_crudebox_cache,
        &env_crudebox_config,
        &env_home,
        &env_xdg_cache,
        &env_xdg_config,
    };

    (void) state;

    for (int i = 0; i < ARRAY_SIZE(table); ++i) {
        const char *val = table[i]();

        assert_ptr_equal(val, table[i]());
    }
}

static void T002_env_crudebox_cache(void **state)
{
    (void) state;
}

static const struct CMUnitTest tests[] = {
    cmocka_unit_test(T001_crudebox_env),
    cmocka_unit_test(T002_env_crudebox_cache),
};

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    return cmocka_run_group_tests_name(__FILE__, tests, NULL, NULL);
}
