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

#ifndef ITEM_LIST_H_
#define ITEM_LIST_H_

#include <stdbool.h>

#define APP_LIST_SEARCH_SUBSTRING 0
#define APP_LIST_SEARCH_PREFIX 1

struct item {
    char *name;
    char *ptr;
    int score;
};

struct item_list {
    struct item *items;
    int n;
    int n_max;

    char lookup[64];
    int strlen;

    void *mem;
};

void item_list_init(struct item_list *list, const char *dirs);

void item_list_destroy(struct item_list *list);

static inline bool item_list_empty(const struct item_list *list)
{
    return list->n == 0;
}

void item_list_lookup_clear(struct item_list *list);

void item_list_lookup_push_back(struct item_list *list, int c);

void item_list_lookup_pop_back(struct item_list *list);

static inline const struct item *item_list_cbegin(const struct item_list *list)
{
    return list->items;
}

static inline const struct item *item_list_cend(const struct item_list *list)
{
    return list->items + list->n;
}

static inline int item_list_lookup_score(const struct item_list *list)
{
    return list->strlen;
}

#endif /* ITEM_LIST_H_ */
