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
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>

#include "io-util.h"

int io_util_read(int fd, void *buf, size_t size)
{
    size_t n = 0;

    do {
        ssize_t m = read(fd, (char *) buf + n, size - n);
        if (m < 0) {
            if (errno == EINTR)
                continue;

            return -errno;
        }

        n += m;
    } while (n < size);

    return 0;
}

int io_util_read_all(int fd, char **buf, size_t *size)
{
    struct stat st;
    void *mem;
    int err;

    err = fstat(fd, &st);
    if (err < 0)
        return -errno;

    mem = malloc((size_t) st.st_size);
    if (!mem)
        return -ENOMEM;

    err = io_util_read(fd, mem, (size_t) st.st_size);
    if (err < 0) {
        free(mem);
        return err;
    }

    *buf = mem;
    *size = (size_t) st.st_size;

    return 0;
}

int io_util_read_all_str(int fd, char **buf, size_t *size)
{
    struct stat st;
    void *mem;
    int err;

    err = fstat(fd, &st);
    if (err < 0)
        return -errno;

    mem = malloc((size_t) st.st_size + 1);
    if (!mem)
        return -ENOMEM;

    err = io_util_read(fd, mem, (size_t) st.st_size);
    if (err < 0) {
        free(mem);
        return err;
    }

    ((char *) mem)[st.st_size] = '\0';

    *buf = mem;

    if (size)
        *size = (size_t) st.st_size;

    return 0;
}

int io_util_path_read_all(const char *path, char **buf, size_t *size)
{
    int fd, err;

    fd = open(path, O_RDONLY);
    if (fd < 0)
        return -errno;

    err = io_util_read_all(fd, buf, size);

    close(fd);

    return err;
}

int io_util_path_read_all_str(const char *path, char **buf, size_t *size)
{
    int fd, err;

    fd = open(path, O_RDONLY);
    if (fd < 0)
        return -errno;

    err = io_util_read_all_str(fd, buf, size);

    close(fd);

    return err;
}
