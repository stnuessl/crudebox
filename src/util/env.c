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

#include "die.h"
#include "macro.h"

const char *env_crudebox_cache(void)
{
    static const char *crudebox_cache;

    if (!crudebox_cache)
        crudebox_cache = getenv("CRUDEBOX_CACHE");

    return crudebox_cache;
}

const char *env_crudebox_config(void)
{
    static const char *crudebox_config;

    if (!crudebox_config)
        crudebox_config = getenv("CRUDEBOX_CONFIG");

    return crudebox_config;
}

const char *env_home(void)
{
    static const char *home;

    if (!home) {
        home = getenv("HOME");
        if (unlikely(!home))
            die("failed to retrieve ${HOME} from the environment\n");
    }

    return home;
}

const char *env_xdg_cache(void)
{
    static const char *xdg_cache;

    if (!xdg_cache)
        xdg_cache = getenv("XDG_CACHE_HOME");

    return xdg_cache;
}

const char *env_xdg_config(void)
{
    static const char *xdg_config;

    if (!xdg_config)
        xdg_config = getenv("XDG_CONFIG_HOME");

    return xdg_config;
}
