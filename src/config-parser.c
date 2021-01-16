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
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config-parser.h"

#include "util/die.h"
#include "util/io-util.h"
#include "util/xalloc.h"

#include "timer.h"

static int config_parser_get_event(struct config_parser *parser)
{
    int begin, end;

    begin = 0;
    end = parser->num_events;

    while (begin < end) {
        int mid, cmp;

        mid = begin + (end - begin) / 2;
        cmp = strcmp(parser->events[mid].section, parser->current.section);
        if (!cmp)
            cmp = strcmp(parser->events[mid].key, parser->current.key);

        if (cmp < 0)
            begin = mid + 1;
        else if (cmp > 0)
            end = mid;
        else
            return mid;
    }

    return -1;
}

static void config_parser_emit(struct config_parser *parser)
{
    int index;

    index = config_parser_get_event(parser);
    if (index < 0)
        return;

    parser->events[index].func(parser->events + index, parser->current.value);
}

__attribute__((noreturn, format(printf, 2, 3))) static void
config_parser_error(const struct config_parser *parser, const char *fmt, ...)
{
    va_list args;
    const char *path;
    int line, column;

    va_start(args, fmt);

    path = parser->path;
    line = parser->num_line;
    column = parser->cursor - parser->line;

    fprintf(stderr, "config-parser: error:\n%s:%d:%d:\n  ", path, line, column);
    vfprintf(stderr, fmt, args);

    va_end(args);

    exit(EXIT_FAILURE);
}

static void config_parser_state_comment(struct config_parser *parser)
{
    while (!parser->done) {
        switch (*parser->cursor++) {
        case '\0':
            parser->done = true;
            return;
        case '\n':
            parser->line = parser->cursor;
            ++parser->num_line;
            return;
        default:
            break;
        }
    }
}

static void config_parser_state_line_end(struct config_parser *parser)
{
    while (!parser->done) {
        switch (*parser->cursor++) {
        case '\0':
            parser->done = true;
            return;
        case '\n':
            parser->line = parser->cursor;
            ++parser->num_line;
            return;
        case ';':
            /* FALLTHROUGH */
        case '#':
            config_parser_state_comment(parser);
            return;
        case '\t':
            /* FALLTHROUGH */
        case ' ':
            break;
        default:
            config_parser_error(parser,
                                "unexpected character '%c' after section "
                                "header\n",
                                parser->cursor[-1]);
            break;
        }
    }
}

static void config_parser_state_section_3(struct config_parser *parser)
{
    while (!parser->done) {
        switch (*parser->cursor++) {
        case '\0':
            config_parser_error(parser,
                                "unexpected end-of-file after section name\n");
            break;
        case '\n':
            config_parser_error(
                parser,
                "unexpected newline character after section name\n");
            break;
        case ']':
            config_parser_state_line_end(parser);
            return;
        case '\t':
            /* FALLTHROUGH */
        case ' ':
            break;
        default:
            config_parser_error(parser,
                                "unexpected non-blank character '%c' after "
                                "section name\n",
                                parser->cursor[-1]);
            break;
        }
    }
}

static void config_parser_state_section_2(struct config_parser *parser)
{
    while (!parser->done) {
        switch (*parser->cursor++) {
        case '\0':
            config_parser_error(parser,
                                "unexpected end-of-file in section name\n");
            break;
        case '\n':
            config_parser_error(
                parser,
                "unexpected newline character in section name\n");
            break;
        case '\t':
            /* FALLTHROUGH */
        case ' ':
            parser->cursor[-1] = '\0';
            config_parser_state_section_3(parser);
            return;
        case ']':
            parser->cursor[-1] = '\0';
            config_parser_state_line_end(parser);
            return;
        case '_':
            /* FALLTHROUGH */
        case '-':
            break;
        default:
            if (!isalnum(parser->cursor[-1])) {
                config_parser_error(
                    parser,
                    "unepexcected character '%c' in section name. "
                    "Section names may consist of the following "
                    "characters [a-zA-Z0-9_]\n",
                    parser->cursor[-1]);
            }
            break;
        }
    }
}

static void config_parser_state_section_1(struct config_parser *parser)
{
    while (!parser->done) {
        switch (*parser->cursor++) {
        case '\0':
            config_parser_error(parser,
                                "unexpected end-of-file in section header\n");
            break;
        case '\n':
            config_parser_error(
                parser,
                "unexpected newline character in section header\n");
            break;
        case '\t':
            /* FALLTHROUGH */
        case ' ':
            break;
        default:
            if (!isalpha(parser->cursor[-1]) && parser->cursor[-1] != '_') {
                config_parser_error(
                    parser,
                    "unexpected non-alphabetic character at start of a section "
                    "name. Section names shall start with [a-zA-Z_].\n");
            }

            parser->current.section = parser->cursor - 1;
            config_parser_state_section_2(parser);
            return;
        }
    }
}

static void config_parser_state_value_2(struct config_parser *parser)
{
    /*
     * Parse the value associated with the current section and key.
     * Variable 'p' is used to filter out redundant tabs, newlines or spaces
     * by copying only non-redundant data to its position in memory.
     */
    char *p = parser->cursor - 1;
    bool empty = true;

    while (!parser->done) {
        switch (*parser->cursor++) {
        case '\0':
            parser->done = true;
            goto out;
        case '\n':
            parser->line = parser->cursor - 1;
            ++parser->num_line;

            /*
             * Overwrite tabs and spaces by a newline character to avoid
             * having them trailing the value string.
             */
            if (isblank(*p))
                *p = '\n';
            else if (*p != '\n')
                *++p = '\n';

            /* Finish if the next character starts a new key name */
            if (!isblank(*parser->cursor))
                goto out;

            /* Finish if the previous line is empty */
            if (empty)
                goto out;

            empty = true;
            break;
        case ';':
            /* FALLTHROUGH */
        case '#':
            config_parser_state_comment(parser);
            goto out;
        case '\t':
            /* FALLTHROUGH */
        case ' ':
            if (!isblank(*p))
                *++p = ' ';

            break;
        default:
            *++p = parser->cursor[-1];
            empty = false;
            break;
        }
    }
out:
    /*
     * We can only reach this code here if 'p' points to a newline or
     * a null character. In both cases we are fine terminating the
     * value string.
     */
    *p = '\0';

    config_parser_emit(parser);
}

static void config_parser_state_value_1(struct config_parser *parser)
{
    /*
     * Advance the cursor to the beginning of the value string.
     * The value may be put on the next line after the key if it is indented
     * with at least one blank character.
     */
    bool newline = false;

    while (!parser->done) {
        switch (*parser->cursor++) {
        case '\n':
            parser->line = parser->cursor - 1;
            ++parser->num_line;

            if (newline) {
                config_parser_error(
                    parser,
                    "redundant newline character after equals sign. "
                    "Expected value definition.\n");
            }

            newline = true;
            break;
        case '\t':
            /* FALLTHROUGH */
        case ' ':
            break;
        default:
            parser->current.value = parser->cursor - 1;

            /*
             * If we encountered a newline character, the character just
             * before the value has to be blank.
             */
            if (newline && !isblank(parser->current.value[-1])) {
                config_parser_error(
                    parser,
                    "expected indentation of value definition if put "
                    "on a new line\n");
            }

            config_parser_state_value_2(parser);
            return;
        }
    }
}

static void config_parser_state_assignment(struct config_parser *parser)
{
    while (!parser->done) {
        switch (*parser->cursor++) {
        case '\0':
            parser->done = true;
            return;
        case ':':
            /* FALLTHROUGH */
        case '=':
            config_parser_state_value_1(parser);
            return;
        case '\n':
            /* Current key has no value */
            parser->current.value = "";

            config_parser_emit(parser);
            return;
        case '\t':
            /* FALLTHROUGH */
        case ' ':
            break;
        default:
            config_parser_error(
                parser,
                "unexpected non-blank character before assignment");
            break;
        }
    }
}

static void config_parser_state_key_name(struct config_parser *parser)
{
    while (!parser->done) {
        switch (*parser->cursor++) {
        case '\0':
            parser->done = true;
            return;
        case '\n':
            /* FALLTHROUGH */
        case ' ':
            parser->cursor[-1] = '\0';
            config_parser_state_assignment(parser);

            return;
        case ':':
            /* FALLTHROUGH */
        case '=':
            parser->cursor[-1] = '\0';
            config_parser_state_value_1(parser);

            return;
        case '-':
            break;
        case '_':
            break;
        default:
            if (!isalnum(parser->cursor[-1])) {
                config_parser_error(
                    parser,
                    "unexpected character '%c' in key name. Keys must "
                    "consist of the following character set: "
                    "[a-zA-Z0-9_-]\n",
                    parser->cursor[-1]);
            }

            break;
        }
    }
}

static void config_parser_state_main(struct config_parser *parser)
{
    TIMER_INIT_SIMPLE(CLOCK_MONOTONIC);

    while (!parser->done) {
        switch (*parser->cursor++) {
        case '\0':
            parser->done = true;
            return;
        case '[':
            config_parser_state_section_1(parser);
            break;
        case '\n':
            parser->line = parser->cursor;
            ++parser->num_line;
        case '\t':
            /* FALLTHROUGH */
        case '\r':
            /* FALLTHROUGH */
        case ' ':
            break;
        case ';':
            /* FALLTHROUGH */
        case '#':
            config_parser_state_comment(parser);
            break;
        default:
            if (!isalpha(parser->cursor[-1]) && parser->cursor[-1] != '_') {
                config_parser_error(
                    parser,
                    "unexpected non-alphabetic and non-underscore "
                    "character '%c' at the beginning of a key "
                    "name. Key names shall start with [a-zA-Z_].\n",
                    parser->cursor[-1]);
            }
            parser->current.key = parser->cursor - 1;
            config_parser_state_key_name(parser);
        }
    }
}

void config_parser_init(struct config_parser *parser,
                        struct config_parser_event *events,
                        int num_events)
{
    parser->events = events;
    parser->num_events = num_events;

    parser->mem = NULL;
}

void config_parser_destroy(struct config_parser *parser)
{
#ifdef MEM_NOLEAK
    free(parser->mem);
#else
    (void) parser;
#endif
}

int config_parser_run(struct config_parser *parser, const char *path)
{
    char *mem;
    size_t size;
    int err;

    TIMER_INIT_SIMPLE(CLOCK_MONOTONIC);

    err = io_util_path_read_all_str(path, &mem, &size);
    if (err < 0)
        return err;

    /* Initialize the parser structure */
    parser->path = path;
    parser->mem = mem;
    parser->current.section = "";
    parser->current.key = "";
    parser->current.value = "";
    parser->cursor = mem;
    parser->line = mem;
    parser->num_line = 1;
    parser->done = false;

    config_parser_state_main(parser);

    return 0;
}
