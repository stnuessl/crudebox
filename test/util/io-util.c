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

#include <errno.h>
#include <fcntl.h>

#include <criterion/criterion.h>

#include "util/die.h"
#include "util/errstr.h"
#include "util/io-util.h"
#include "util/macro.h"

Test(io_util_read, valid_fd)
{
    char buf[4];
    int fd;

    fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0)
        die("open(): %s\n", errstr(errno));

    cr_assert(io_util_read(fd, buf, ARRAY_SIZE(buf)) == 0);
}

Test(io_util_read, invalid_fd)
{
    cr_assert(io_util_read(-1, NULL, 0) == -EBADF);
}

Test(io_util_read_all, valid_fd)
{
    char *buf;
    size_t size;
    int fd;

    fd = open("/usr/include/stdlib.h", O_RDONLY);
    if (fd < 0)
        die("open(): %s\n", errstr(errno));

    cr_assert(io_util_read_all(fd, &buf, &size) == 0);
}

Test(io_util_read_all, invalid_fd)
{
    cr_assert(io_util_read_all(-1, NULL, 0) == -EBADF);
}

Test(io_util_path_read_all, valid_file)
{
    char *buf;
    size_t size;

    cr_assert(io_util_path_read_all("/usr/include/stdlib.h", &buf, &size) == 0);

    free(buf);
}

Test(io_util_path_read_all, invalid_file)
{
    cr_assert(io_util_path_read_all("", NULL, NULL) == -ENOENT);
}

Test(io_util_read_all_str, valid_fd)
{
    char *buf;
    size_t size;
    int fd;

    fd = open("/usr/include/stdlib.h", O_RDONLY);
    if (fd < 0)
        die("open(): %s\n", errstr(errno));

    cr_assert(io_util_read_all_str(fd, &buf, &size) == 0);

    free(buf);
}

Test(io_util_read_all_str, invalid_fd)
{
    cr_assert(io_util_read_all_str(-1, NULL, NULL) == -EBADF);
}

Test(io_util_path_read_all_str, valid_file)
{
    char *path, *buf;
    size_t size;

    path = "/usr/include/stdlib.h";

    cr_assert(io_util_path_read_all_str(path, &buf, &size) == 0);

    free(buf);
}

Test(io_util_path_read_all_str, invalid_file)
{
    cr_assert(io_util_path_read_all_str("", NULL, NULL) == -ENOENT);
}
