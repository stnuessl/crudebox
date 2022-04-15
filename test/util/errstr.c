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
#include <limits.h>
#include <string.h>

#include <criterion/criterion.h>

#include "util/errstr.h"

Test(errstr, 001_errstr)
{
    cr_assert(strcmp(errstr(0), "OK") == 0);
    cr_assert(strcmp(errstr(EPERM), "EPERM") == 0);
    cr_assert(strcmp(errstr(-ENOENT), "ENOENT") == 0);

    cr_assert(errstr(INT_MIN) == NULL);
    cr_assert(errstr(INT_MAX) == NULL);
}
