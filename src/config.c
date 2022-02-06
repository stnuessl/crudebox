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
#include <stdlib.h>

#include "config.h"
#include "timer.h"

#include "util/die.h"
#include "util/env.h"
#include "util/errstr.h"
#include "util/macro.h"
#include "util/string-util.h"

static void mem_set_u32(struct config_parser_event *event, char *value)
{
    char *p;
    long x;

    errno = 0;
    x = strtol(value, &p, 0);

    if (unlikely(errno != 0)) {
        if (errno == ERANGE) {
            die("config: %s.%s: cannot convert \"%s\" - value out of range\n",
                event->section,
                event->key,
                value);
        }

        die("config: %s.%s: cannot convert \"%s\n",
            event->section,
            event->key,
            value);
    }

    /* The entire string has to be converted */
    if (*p != '\0') {
        die("config: %s.%s: invalid conversion to uint32_t for value \"%s\"\n",
            event->section,
            event->key,
            value);
    }

    if (x < 0 || x > UINT32_MAX) {
        die("config: %s.%s: invalid uint32_t value \"%ld\"\n",
            event->section,
            event->key,
            x);
    }

    *(uint32_t *) event->mem = x;
}

static void mem_set_color(struct config_parser_event *event, char *value)
{
    if (unlikely(strprefix(value, "0x"))) {
        die("%s.%s: value \"%s\" must be hexadecimal and start with \"0x\"\n",
            event->section,
            event->key,
            value);
    }

    mem_set_u32(event, value);

    if (strlen(value) <= 8) {
        uint32_t x;

        x = *(uint32_t *) event->mem;
        x = (x << 8) | 0xff;

        *(uint32_t *) event->mem = x;
    }
}

#if 0
static void mem_set_double(struct config_parser_event *event, char *value)
{
    char *p;
    double x;

    errno = 0;
    x = strtod(value, &p);

    if (unlikely(errno != 0)) {
        if (errno == ERANGE) {
            die("config: %s.%s: cannot convert \"%s\" - value out of range\n",
                event->section,
                event->key,
                value);
        }

        die("config: %s.%s: cannot convert \"%s\"\n",
            event->section,
            event->key,
            value);
    }

    if (unlikely(*p != '\0')) {
        die("config: %s.%s: invalid conversion to double for value \"%s\"\n",
            event->section,
            event->key,
            value);
    }

    *(double *) event->mem = x;
}
#endif

static void mem_set_str(struct config_parser_event *event, char *value)
{
    *(char **) event->mem = value;
}

static int
config_run_parser(struct config *config, const char *prefix, const char *path)
{
    char *buf;

    strconcat2a(&buf, prefix, path);

    return config_parser_run(&config->parser, buf);
}

void config_init(struct config *config)
{
    TIMER_INIT_SIMPLE();

    /*
     * The elements in this array have to be sorted according to their
     * first and second data field.
     */
    struct config_parser_event events[] = {
        /* clang-format off */
        { "font", "path", &config->font.path, &mem_set_str },
        { "font", "size", &config->font.size, &mem_set_u32 },
        { "line-edit", "bg", &config->line_edit.bg, &mem_set_color },
        { "line-edit", "fg", &config->line_edit.fg, &mem_set_color },
        { "list-view", "bg1", &config->list_view.bg1, &mem_set_color },
        { "list-view", "bg1-sel", &config->list_view.bg1_sel, &mem_set_color },
        { "list-view", "bg2", &config->list_view.bg2, &mem_set_color },
        { "list-view", "bg2-sel", &config->list_view.bg2_sel, &mem_set_color },
        { "list-view", "fg", &config->list_view.fg, &mem_set_color },
        { "list-view", "fg-sel", &config->list_view.fg_sel, &mem_set_color },
        { "list-view", "lines", &config->list_view.lines, &mem_set_color },
        { "list-view", "size", &config->list_view.size, &mem_set_u32 },
        { "widget", "frame", &config->widget.frame, &mem_set_color },
        { "widget", "line-width", &config->widget.line_width, &mem_set_u32 },
        /* clang-format on */
    };
    const char *crudebox_config, *home, *xdg_config;
    int err;

    config_parser_init(&config->parser, events, ARRAY_SIZE(events));

    crudebox_config = env_crudebox_config();
    if (crudebox_config) {
        err = config_parser_run(&config->parser, crudebox_config);
        if (err < 0)
            die("failed to load configuration file \"%s\"\n", crudebox_config);
    }

    home = env_home();
    xdg_config = env_xdg_config();
    if (!xdg_config) {
        strconcat2a(&xdg_config, home, "/.config");
    }

    const char *twines[] = {
        xdg_config,
        "/crudebox/config",
        xdg_config,
        "/crudebox/crudebox.conf",
        home,
        "/.crudebox",
        home,
        "/.crudebox.conf",
        "/etc/crudebox/config",
        "",
        "/etc/crudebox/crudebox.conf",
        "",
        "/etc/crudebox.conf",
        "",
    };

    for (int i = 0; i < ARRAY_SIZE(twines); i += 2) {
        const char *a, *b;

        a = twines[i];
        b = twines[i + 1];

        err = config_run_parser(config, a, b);
        if (err == 0)
            return;

        if (unlikely(err != -ENOENT))
            die("failed to load configuration file \"%s%s\"\n", a, b);
    }

    die("failed to auto-detect the configuration file\n");
}

void config_destroy(struct config *config)
{
#ifdef MEM_NOLEAK
    config_parser_destroy(&config->parser);
#else
    (void) config;
#endif
}
