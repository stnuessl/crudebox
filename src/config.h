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

#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdbool.h>
#include <stdint.h>

#include "config-parser.h"

struct config {
    struct config_parser parser;

    struct {
        uint32_t frame;
        uint32_t line_width;
    } widget;

    struct {
        const char *path;
        uint32_t size;
    } font;

    struct {
        uint32_t size;

        uint32_t fg;
        uint32_t bg1;
        uint32_t bg2;
        uint32_t fg_sel;
        uint32_t bg1_sel;
        uint32_t bg2_sel;
        uint32_t lines;
    } list_view;

    struct {
        uint32_t fg;
        uint32_t bg;
    } line_edit;
};

void config_init(struct config *config);

void config_destroy(struct config *config);

#endif /* CONFIG_H_ */
