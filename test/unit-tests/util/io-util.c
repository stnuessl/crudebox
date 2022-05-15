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

#include <fcntl.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

#include <cmocka.h>

#include <util/io-util.h>
#include <util/macro.h>

static int enable_mocks;

int __real_open(const char *path, int flags, mode_t mode);

ssize_t __real_read(int fd, void *buf, size_t size);

int __wrap_open(const char *path, int flags, mode_t mode)
{
    if (!enable_mocks)
        return __real_open(path, flags, mode);

    (void) path;
    (void) flags;
    (void) mode;

    return 0;
}

ssize_t __wrap_read(int fd, void *buf, size_t size)
{
    if (!enable_mocks)
        return __real_read(fd, buf, size);

    enable_mocks = 0;

    check_expected(fd);
    check_expected(buf);
    check_expected(size);

    enable_mocks = 1;

    return mock_type(ssize_t);
}

static void T001_io_util_read(void **state)
{
    char buf[16];

    (void) state;

    enable_mocks = true;

    expect_value(__wrap_read, fd, 4);
    expect_value(__wrap_read, buf, buf);
    expect_value(__wrap_read, size, ARRAY_SIZE(buf));

    will_return(__wrap_read, ARRAY_SIZE(buf));

    int val = io_util_read(4, buf, ARRAY_SIZE(buf));

    enable_mocks = false;

    assert_int_equal(0, val);
}

static const struct CMUnitTest tests[] = {
    cmocka_unit_test(T001_io_util_read),
};

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    return cmocka_run_group_tests_name(__FILE__, tests, NULL, NULL);
}

#if 0
ssize_t __real_read(int fd, void *buf, size_t size);

static ssize_t (*read_mock)(int, void *, size_t);

ssize_t __wrap_read(int fd, void *buf, size_t size)
{
    if (read_mock)
        return read_mock(fd, buf, size);

    return __real_read(fd, buf, size);
}

#if 0
static int (*open_mock)(const char *, int, mode_t);

int __wrap_open(const char *path, int flags, mode_t mode)
{
    return open_mock(path, flags, mode);
}
#endif

static ssize_t read_OK(int fd, void *buf, size_t size)
{
    cr_assert(fd >= 0);
    cr_assert(buf != NULL);
    cr_assert(size > 0);

    return size;
}

static ssize_t read_EBADF(int fd, void *buf, size_t size)
{
    cr_assert(fd < 0);
    (void) buf;
    (void) size;

    errno = EBADF;
    return -1;
}

static ssize_t read_EINTR(int fd, void *buf, size_t size)
{
    cr_assert(fd >= 0);
    cr_assert(buf != NULL);
    cr_assert(size > 0);

    read_mock = read_OK;

    errno = EINTR;
    return -1;
}

Test(io_util, 001_io_util_read)
{
    char buf[4];

    read_mock = &read_OK;
    cr_assert(io_util_read(4, buf, ARRAY_SIZE(buf)) == 0);

    read_mock = &read_EBADF;
    cr_assert(io_util_read(-1, NULL, 0) == -EBADF);

    read_mock = &read_EINTR;
    cr_assert(io_util_read(5, buf, ARRAY_SIZE(buf)) == 0);
}

Test(io_util, 001_io_util_path_read_all_str)
{
    char *path, *buf;
    size_t size;

    path = "/usr/include/stdlib.h";

    read_mock = NULL;

    cr_assert(io_util_path_read_all_str(path, &buf, &size) == 0);
    cr_assert(buf[size] == '\0');

    free(buf);
}

Test(io_util, 002_io_util_path_read_all_str)
{
    read_mock = NULL;

    cr_assert(io_util_path_read_all_str("", NULL, NULL) == -ENOENT);
}
#endif
