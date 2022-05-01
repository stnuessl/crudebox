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

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "item-list.h"
#include "timer.h"

#include "util/env.h"
#include "util/io-util.h"
#include "util/macro.h"
#include "util/string-util.h"
#include "util/xalloc.h"

static void merge(struct item *dst,
                  struct item *s1,
                  const struct item *e1,
                  struct item *s2,
                  const struct item *e2)
{
    while (s1 < e1 && s2 < e2) {
        if (strverscmp(s1->name, s2->name) <= 0)
            dst++->name = s1++->name;
        else
            dst++->name = s2++->name;
    }

    while (s1 < e1)
        dst++->name = s1++->name;

    while (s2 < e2)
        dst++->name = s2++->name;
}

static void item_list_sort(struct item_list *list)
{
    struct item *buf, *items = list->items;
    int n = 16, size = list->n;
    bool use_heap;

    use_heap = size > list->n_max / 2;
    if (use_heap)
        buf = xmalloc(size * sizeof(*buf));
    else
        buf = items + size;

    /* Sort small batches of size 'n' with insertion sort */
    for (int i = 0; i < size; i += n) {
        int end = i + n;

        if (end > size)
            end = size;

        for (int j = i + 1; j < end; ++j) {
            char *name = items[j].name;
            int k = j;

            while (k > i && strverscmp(items[k - 1].name, name) > 0) {
                items[k].name = items[k - 1].name;
                --k;
            }

            items[k].name = name;
        }
    }

    /*
     * Sort the small batches into bigger batches until the complete array
     * is sorted.
     */
    while (n < size) {
        struct item *dst = buf;
        struct item *src = items;

        for (int i = 0; i < size; i += (n + n)) {
            int j = i + n;
            int k = j + n;

            if (j > size)
                j = size;

            if (k > size)
                k = size;

            merge(dst, src + i, src + j, src + j, src + k);
            dst += k - i;
        }

        n *= 2;

        dst = items;
        src = buf;

        for (int i = 0; i < size; i += (n + n)) {
            int j = i + n;
            int k = j + n;

            if (j > size)
                j = size;

            if (k > size)
                k = size;

            merge(dst, src + i, src + j, src + j, src + k);
            dst += k - i;
        }

        n *= 2;
    }

#ifdef MEM_NOLEAK
    if (use_heap)
        free(buf);
#endif
}

static void item_list_dedup(struct item_list *list)
{
    int i = 0;

    for (int j = 1; j < list->n; ++j) {
        if (strcmp(list->items[i].name, list->items[j].name) != 0) {
            list->items[++i].name = list->items[j].name;
            continue;
        }

#ifdef MEM_NOLEAK
        free(list->items[j].name);
#endif
    }

    /* Last index containing a unique item is 'i' */
    list->n = i + 1;
}

static int
item_list_do_cache_read(struct item_list *list, int fd, const char *dirs)
{
    struct item *items;
    char *ptr, *str;
    size_t n = 0, n_max;
    int err;

    free(list->mem);

    err = io_util_read_all_str(fd, (char **) &list->mem, NULL);
    if (err < 0)
        return -1;

    ptr = list->mem;

    /* Does 'dirs' match with the information stored in the cache? */
    str = ptr;
    ptr = strchr(ptr, '\n');

    if (!ptr)
        return -1;

    *ptr++ = '\0';

    if (strcmp(str, dirs) != 0)
        return -1;

    /* Retrieve the number of items stored in the cache */
    if (!isdigit(ptr[0]))
        return -1;

    n_max = strtoul(ptr, &ptr, 10);

    /* Retrieve all items from the cache */
    items = xmalloc(n_max * sizeof(*items));

    while (ptr) {
        str = ptr;
        ptr = strchr(ptr, '\n');

        /* Convert the newline to a terminating null byte */
        if (ptr)
            *ptr++ = '\0';

        /*
         * If at the beginning of this iteration 'ptr' pointed to a newline
         * character or the end of the string itself, then 'str' will be empty
         * and can therefore be skipped.
         */
        if (*str == '\0')
            continue;

        if (n >= n_max)
            return -1;

        items[n].name = str;
        items[n].score = 0;

        ++n;
    }

    /* Move data to app list structure */
    list->items = items;
    list->n = n;
    list->n_max = n_max;

    return 0;
}

static bool cache_dirty(const struct stat *st_cache, char *dirs)
{
    /*
     * Iterate over all directories contained in 'dirs' and check if they
     * were modified after the current cache file was modified.
     * If this is the case, the cache is considered dirty.
     */
    while (dirs) {
        struct stat st;
        char *path;
        int err;

        path = dirs;
        dirs = strchr(dirs, ':');

        if (dirs)
            *dirs = '\0';

        err = stat(path, &st);

        if (dirs)
            *dirs++ = ':';

        if (err < 0) {
            if (errno == EEXIST)
                continue;

            return true;
        }

        /* Check if last cache update is newer than last update to 'path' */
        if (st_cache->st_mtim.tv_sec > st.st_mtim.tv_sec)
            continue;

        /* Check if last cache update is older than last update to 'path' */
        if (st_cache->st_mtim.tv_sec < st.st_mtim.tv_sec)
            return true;

        /* Timestamps need to be compared on a finer granularity... */
        if (st_cache->st_mtim.tv_nsec <= st.st_mtim.tv_nsec)
            return true;
    }

    return false;
}

static int
item_list_read_cache(struct item_list *list, const char *cache, char *dirs)
{
    struct stat st;
    int fd, err;

    fd = open(cache, O_RDONLY);
    if (fd < 0)
        return -1;

    err = fstat(fd, &st);
    if (err < 0)
        goto out;

    if (cache_dirty(&st, dirs))
        goto out;

    err = item_list_do_cache_read(list, fd, dirs);

    close(fd);

    return err;

out:
    close(fd);
    return -1;
}

static void item_list_write_cache(const struct item_list *list,
                                  const char *cache,
                                  const char *dirs)
{
    FILE *file;

    file = fopen(cache, "w");
    if (!file)
        return;

    fprintf(file, "%s\n%d\n\n", dirs, list->n);

    for (int i = 0; i < list->n; ++i)
        fprintf(file, "%s\n", list->items[i].name);

    fclose(file);
}

static void
item_list_do_load(struct item_list *list, const char *cache, char *dirs)
{
    struct item *items;
    size_t n = 0, n_max = 4096;
    char *it;
    int err;

    /* Check if we can use previously cached data */
    err = item_list_read_cache(list, cache, dirs);
    if (!err)
        return;

    items = xmalloc(n_max * sizeof(*items));

    /*
     * Iterate over all directories in 'dirs' and search for executable
     * programs. Make sure that 'dirs' contains its original values after the
     * loop is finished.
     */
    it = dirs;
    while (it) {
        char *path;
        DIR *dir;
        int fd;

        path = it;
        it = strchr(it, ':');

        if (it)
            *it = '\0';

        dir = opendir(path);

        if (it)
            *it++ = ':';

        if (!dir)
            continue;

        fd = dirfd(dir);

        while (1) {
            struct dirent *entry = readdir(dir);
            struct stat st;

            if (!entry)
                break;

            err = fstatat(fd, entry->d_name, &st, 0);
            if (err < 0)
                continue;

            if (!S_ISREG(st.st_mode) || !(st.st_mode & S_IXUSR))
                continue;

            if (n >= n_max) {
                n_max = n_max * 2 - n_max / 2;

                items = xrealloc(items, n_max * sizeof(*items));
            }

            items[n].name = xstrdup(entry->d_name);
            items[n].score = 0;

            ++n;
        }

        closedir(dir);
    }

    /* Move data to app list structure */
    list->items = items;
    list->n = n;
    list->n_max = n_max;

    item_list_sort(list);
    item_list_dedup(list);

    item_list_write_cache(list, cache, dirs);
}

static void item_list_load_from_stdin(struct item_list *list)
{
    size_t n = 0, n_max = 4096;
    struct item *items;
    char *data;

    data = xmalloc(n_max);

    /* Read all available data from stdin */
    while (1) {
        ssize_t m = read(STDIN_FILENO, data + n, n_max - n);
        if (m < 0) {
            if (errno == EINTR)
                continue;

            die("failed to read data from stdin\n");
        }

        if (!m)
            break;

        n += m;

        if (n >= n_max) {
            n_max *= 2;

            data = xrealloc(data, n_max);
        }
    }

    if (!n) {
        /* No data available */
        free(data);
        return;
    }

    /* Ensure that we can treat the data as a string */
    if (n >= n_max)
        data = xrealloc(data, n_max + 1);

    data[n] = '\0';

    /* Get a good initial value for the number of expected items */
    n_max = n / 40 + 10;
    n = 0;

    items = xmalloc(n_max * sizeof(*items));

    /* Extract the names from the data and move them into an array */
    while (data) {
        char *p, *str = data;

        data = strchr(data, '\n');
        if (data)
            *data++ = '\0';

        /* Trim preceding spaces */
        while (*str != '\0' && isspace(*str))
            ++str;

        p = str;

        /* Trim trailing spaces */
        while (*p != '\0' && !isspace(*p))
            ++p;

        *p = '\0';

        /* Skip empty strings */
        if (str == p)
            continue;

        if (n >= n_max) {
            n_max = n_max * 2 - n_max / 2;

            items = xrealloc(items, n_max * sizeof(*items));
        }

        items[n].name = str;
        items[n].score = 0;

        ++n;
    }

    /* Move data to app list structure */
    list->items = items;
    list->n = n;
    list->n_max = n_max;

    free(list->mem);
    list->mem = data;
}

static void item_list_load_from_directories(struct item_list *list,
                                            const char *dirs)
{
    const char *cache, *xdg_cache;
    char *dup;

    /* Create path to the cache file */
    cache = env_crudebox_cache();
    if (!cache) {
        xdg_cache = env_xdg_cache();
        if (xdg_cache) {
            strconcat2a(&cache, xdg_cache, "/crudebox/cache");
        } else {
            const char *home = env_home();

            strconcat2a(&cache, home, "/.cache/crudebox/cache");
        }
    }

    /* Make sure that we can modify the directory list */
    dup = strdupa(dirs);

    item_list_do_load(list, cache, dup);
}

void item_list_init(struct item_list *list, const char *dirs)
{
    TIMER_INIT_SIMPLE();

    memset(list, 0, sizeof(*list));

    if (!dirs) {
        int n_packets, n_bytes;

        n_packets = ioctl(STDIN_FILENO, FIONREAD, &n_bytes);
        if (unlikely(n_packets < 0))
            die("failed to check for data on stdin\n");

        if (n_bytes > 0) {
            item_list_load_from_stdin(list);

            if (!item_list_empty(list))
                return;
        }

        /*
         * No directory paths passed and nothing to read from stdin.
         * Use default.
         */
        dirs = getenv("PATH");
        if (unlikely(!dirs))
            die("failed to retrieve ${PATH} variable from environment\n");
    }

    item_list_load_from_directories(list, dirs);
}

void item_list_destroy(struct item_list *list)
{
#ifdef MEM_NOLEAK
    if (!list->mem) {
        for (int i = 0; i < list->n; ++i)
            free(list->items[i].name);
    }

    free(list->items);
    free(list->mem);
#else
    (void) list;
#endif
}

void item_list_lookup_clear(struct item_list *list)
{
    /* Clear lookup string */
    while (list->strlen)
        list->lookup[list->strlen--] = '\0';

    /* Clear item list */
    for (int i = 0; i < list->n; ++i)
        list->items[i].score = 0;
}

void item_list_lookup_push_back(struct item_list *list, int c)
{
    TIMER_INIT_SIMPLE();

    if (list->strlen < ARRAY_SIZE(list->lookup) - 1 && isascii(c))
        list->lookup[list->strlen++] = (char) c;

    for (int i = 0; i < list->n; ++i) {
        if (list->items[i].score != list->strlen - 1)
            continue;

        if (!strstr(list->items[i].name, list->lookup))
            continue;

        list->items[i].score = list->strlen;
    }
}

void item_list_lookup_pop_back(struct item_list *list)
{
    if (list->strlen)
        list->lookup[--list->strlen] = '\0';

    for (int i = 0; i < list->n; ++i) {
        if (list->items[i].score < list->strlen + 1)
            continue;

        list->items[i].score = list->strlen;
    }
}
