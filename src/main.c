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
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>

#include "util/die.h"
#include "util/macro.h"
#include "util/string-util.h"

#include "config.h"
#include "item-list.h"
#include "timer.h"
#include "window.h"

#ifndef CRUDEBOX_VERSION_MAJOR
#error Preprocessor macro "CRUDEBOX_VERSION_MAJOR" not defined.
#endif

#ifndef CRUDEBOX_VERSION_MINOR
#error Preprocessor macro "CRUDEBOX_VERSION_MINOR" not defined.
#endif

#ifndef CRUDEBOX_VERSION_PATCH
#error Preprocessor macro "CRUDEBOX_VERSION_PATCH" not defined.
#endif

#define CRUDEBOX_VERSION                                                       \
    CRUDEBOX_VERSION_MAJOR "." CRUDEBOX_VERSION_MINOR "." CRUDEBOX_VERSION_PATCH

#ifndef COPYRIGHT_YEAR
#error Preprocessor macro "COPYRIGHT_YEAR" not defined.
#endif

static struct config conf;
static struct window win;
static struct item_list items;

static void help(void)
{
    fprintf(stdout,
            "Usage:\n"
            "  crudebox [options]\n"
            "\n"
            "crudebox options:\n"
            "\n"
            "  --dry-run      Do not execute the selected entry. Instead,\n"
            "                 print it to standard output.\n"
            "  --help,    -h  Print this help message and exit.\n"
            "  --version, -v  Print version information and exit.\n"
            "\n"
            "The list of available programs displayed by crudebox can be\n"
            "influenced by changing the value of the PATH environment\n"
            "variable.\n"
            "\n"
            "The program also supports reading in a list if newline separated\n"
            "values from standard input. If data is available on stdin,\n"
            "no other items will be displayed by crudebox.\n"
            "\n"
            "Extensive help for crudebox can be found here:\n"
            "<https://github.com/stnuessl/crudebox/blob/master/README.md>.\n"
            "\n");
}

static void version(void)
{
    fprintf(stdout,
            "crudebox version " CRUDEBOX_VERSION "\n"
            "\n"
            "Copyright (C) " COPYRIGHT_YEAR " Free Software Foundation, Inc.\n"
            "License GPLv3+: GNU GPL version 3 or later "
            "<https://gnu.org/licenses/gpl.html>.\n"
            "This is free software: you are free to change and redistribute "
            "it.\n"
            "There is NO WARRANTY, to the extent permitted by law.\n"
            "\n"
            "This program is distributed in the hope that it will be useful,\n"
            "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
            "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
            "GNU General Public License for more details.\n"
            "\n"
            "You should have received a copy of the GNU General Public "
            "License\n"
            "along with this program. If not, see "
            "http://www.gnu.org/licenses/.\n"
            "\n");
}

static void *thread1_run(void *arg)
{
    TIMER_INIT_SIMPLE();

    (void) arg;

    item_list_init(&items, NULL);

    config_init(&conf);

    return NULL;
}

static void *thread2_run(void *arg)
{
    TIMER_INIT_SIMPLE();

    (void) arg;

    window_init(&win, NULL);

    return NULL;
}

__attribute__((used)) static int crudebox(int argc, char *argv[])
{
    struct widget *widget;
    struct line_edit *edit;
    struct list_view *view;
    pthread_t thread1, thread2;
    int err1, err2;
    bool dry_run;

    err1 = pthread_create(&thread1, NULL, &thread1_run, NULL);
    err2 = pthread_create(&thread2, NULL, &thread2_run, NULL);

    if (err1 != 0)
        (void) thread1_run(NULL);

    if (err2 != 0)
        (void) thread2_run(NULL);

    dry_run = false;

    for (int i = 1; i < argc; ++i) {
        if (streq("-h", argv[i]) || streq("--help", argv[i])) {
            help();
            exit(EXIT_SUCCESS);
        } else if (streq("--version", argv[i])) {
            version();
            exit(EXIT_SUCCESS);
        } else if (streq("--dry-run", argv[i])) {
            dry_run = true;
        } else {
            die("invalid option \"%s\"\n", argv[i]);
        }
    }

    (void) pthread_join(thread1, NULL);
    (void) pthread_join(thread2, NULL);

    widget = window_get_widget(&win);
    edit = widget_line_edit(widget);
    view = widget_list_view(widget);

    /* Apply configuration to the elements */
    widget_set_font(widget, conf.font.path);
    widget_set_font_size(widget, conf.font.size);
    widget_set_frame_color(widget, conf.widget.frame);
    widget_set_line_width(widget, conf.widget.line_width);
    widget_set_dry_run(widget, dry_run);

    line_edit_set_fg(edit, conf.line_edit.fg);
    line_edit_set_bg(edit, conf.line_edit.bg);

    list_view_set_fg(view, conf.list_view.fg);
    list_view_set_bg(view, conf.list_view.bg1, conf.list_view.bg2);
    list_view_set_fg_sel(view, conf.list_view.fg_sel);
    list_view_set_bg_sel(view, conf.list_view.bg1_sel, conf.list_view.bg2_sel);
    list_view_set_max_rows(view, conf.list_view.size);
    list_view_set_lines(view, conf.list_view.lines);

    widget_set_item_list(widget, &items);

    window_show(&win);

    window_dispatch_events(&win);

    config_destroy(&conf);
    item_list_destroy(&items);
    window_destroy(&win);

    return EXIT_SUCCESS;
}

#ifndef UNIT_TESTS_ENABLED

int main(int argc, char *argv[])
{
    return crudebox(argc, argv);
}

#endif
