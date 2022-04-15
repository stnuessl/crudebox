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
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <criterion/criterion.h>

#include "config-parser.h"
#include "util/die.h"
#include "util/errstr.h"

Test(config_parser, 001_config_parser_run)
{
    struct config_parser parser;

    config_parser_init(&parser, NULL, 0);

    cr_assert(config_parser_run(&parser, "/invalid/path") != 0);

    config_parser_destroy(&parser);
}

Test(config_parser, 002_config_parser_run)
{
    struct config_parser parser;

    config_parser_init(&parser, NULL, 0);

    cr_assert(config_parser_run(&parser, "/dev/null") == 0);

    config_parser_destroy(&parser);
}

/* clang-format off */
static const char valid_config[] = {
    "#\n"
    "# [Comment]\n"
    "#\n"
    "[widget]\n"
    "# Set the widget's frame color\n"
    "frame = 0xffffff\n"
    "# Set the widget's line width\n"
    "line-width = 2\n"
    "\n"
};

static const char invalid_config[] = {
    "#\n"
    "# [Comment]\n"
    "#\n"
    "??????????????????????\n"
    "[widget]\n"
    "# Set the widget's frame color\n"
    "frame = 0xffffff\n"
    "# Set the widget's line width\n"
    "line-width = 2\n"
};
/* clang-format on */

int test_config_parser_run(const char *config)
{
    char *directory, *path;
    struct config_parser parser;
    int fd, err;

    directory = getenv("XDG_RUNTIME_DIR");
    if (!directory)
        directory = "/tmp";

    path = tempnam(directory, "crudebox");
    if (!path)
        die("tempnam(): %s\n", errstr(errno));

    fd = open(path, O_WRONLY | O_EXCL | O_CREAT | O_CLOEXEC, 0600);
    if (fd < 0)
        die("open(): %s\n", errstr(errno));

    (void) dprintf(fd, "%s\n", config);

    close(fd);

    config_parser_init(&parser, NULL, 0);
    err = config_parser_run(&parser, path);
    config_parser_destroy(&parser);

    unlink(path);
    free(path);

    return err;
}

Test(config_parser, 003_config_parser_run)
{
    cr_assert(test_config_parser_run(valid_config) == 0);
}

Test(config_parser, 004_config_parser_run, .exit_code = EXIT_FAILURE)
{
    (void) test_config_parser_run(invalid_config);
}
