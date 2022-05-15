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

#include <item-list.h>

int __wrap_io_util_read_all_str(int fd, char **buf, size_t *size)
{
    (void) fd;

    *buf = "";
    *size = 1;

    return 0;
}

const char *__wrap_env_crudebox_cache(void)
{
    return "";
}

const char *__wrap_env_xdg_cache(void)
{
    return mock_type(const char *);
}

const char *__wrap_env_home(void)
{
    return mock_type(const char *);
}

char *__wrap_getenv(const char *name)
{
    check_expected(name);

    return mock_type(char *);
}

static void T001_item_list_init(void **state)
{
    struct item_list list;

    (void) state;

    expect_value(__wrap_getenv, name, "PATH");
    will_return(__wrap_getenv, "");

    item_list_init(&list, NULL);

    //    assert_true(item_list_empty(&list));
}

static void T001_item_list_destroy(void **state)
{
    struct item_list list;

    (void) state;

    expect_value(__wrap_getenv, name, "PATH");
    will_return(__wrap_getenv, "");

    item_list_init(&list, NULL);
    item_list_destroy(&list);
}

static void T001_item_list_lookup_clear(void **state)
{
    struct item_list list;

    (void) state;

    expect_value(__wrap_getenv, name, "PATH");
    will_return(__wrap_getenv, "");

    item_list_init(&list, NULL);
}

static void T001_item_list_lookup_push_back(void **state)
{
    (void) state;
}

static const struct CMUnitTest tests[] = {
    cmocka_unit_test(T001_item_list_init),
    cmocka_unit_test(T001_item_list_destroy),
    cmocka_unit_test(T001_item_list_lookup_clear),
    cmocka_unit_test(T001_item_list_lookup_push_back),
};

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    return cmocka_run_group_tests_name(__FILE__, tests, NULL, NULL);
}
