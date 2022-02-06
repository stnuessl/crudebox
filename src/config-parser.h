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

#ifndef CONFIG_PARSER_H_
#define CONFIG_PARSER_H_

#include <stdbool.h>

#define CONFIG_OK 0
#define CONFIG_ENOKEY 1
#define CONFIG_EINVAL 2
#define CONFIG_ERANGE 3

struct config_parser_event {
    const char *section;
    const char *key;
    void *mem;
    void (*func)(struct config_parser_event *, char *);
};

struct config_parser {
    struct config_parser_event *events;
    int num_events;

    struct {
        char *section;
        char *key;
        char *value;
    } current;

    const char *path;
    void *mem;

    char *cursor;
    char *line;
    int num_line;

    bool done;
};

void config_parser_init(struct config_parser *parser,
                        struct config_parser_event *events,
                        int num_events);

void config_parser_destroy(struct config_parser *parser);

int config_parser_run(struct config_parser *parser, const char *path);

#endif /* CONFIG_PARSER_H_ */
