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

static struct config conf;
static struct window win;
static struct item_list items;

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


int main(int argc, char *argv[])
{
    struct widget *widget;
    struct line_edit *edit;
    struct list_view *view;
    pthread_t thread1, thread2;
    int err1, err2;
    bool print;

    err1 = pthread_create(&thread1, NULL, &thread1_run, NULL);
    err2 = pthread_create(&thread2, NULL, &thread2_run, NULL);

    if (err1 != 0)
        (void) thread1_run(NULL);

    if (err2 != 0)
        (void) thread2_run(NULL);

    print = false;

    for (int i = 1; i < argc; ++i) {
        if (streq("-h", argv[i]) || streq("--help", argv[i])) {
            printf("Help message\n");
        } else if (streq("--version", argv[i])) {
            printf("Version information\n");
        } else if (streq("--print", argv[i]) || streq("-p", argv[i])) {
            print = true;
        } else if (i + 1 >= argc) {
            die("missing argument for option \"%s\"\n", argv[i]);
        } else {
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
    widget_set_print(widget, print);

    line_edit_set_fg(edit, conf.line_edit.fg);
    line_edit_set_bg(edit, conf.line_edit.bg);

    list_view_set_fg(view, conf.list_view.fg);
    list_view_set_bg(view, conf.list_view.bg1, conf.list_view.bg2);
    list_view_set_fg_sel(view, conf.list_view.fg_sel);
    list_view_set_bg_sel(view, conf.list_view.bg1_sel, conf.list_view.bg2_sel);
    list_view_set_max_rows(view, conf.list_view.size);
    list_view_set_lines(view, conf.list_view.lines);

    widget_set_item_list(widget, &items);

    window_update_size(&win);
    window_show(&win);

    window_dispatch_events(&win);

    config_destroy(&conf);
    item_list_destroy(&items);
    window_destroy(&win);

    return EXIT_SUCCESS;
}
