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

#ifdef CONFIG_USE_WAYLAND

#include <sys/mman.h>

#include "xkb.h"

#include "util/macro.h"

int xkb_set_keymap(struct xkb *xkb, const char *desc)
{
    struct xkb_context *context;
    struct xkb_keymap *keymap;
    struct xkb_state *state;

    context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (unlikely(!context))
        return -1;

    /* clang-format off */
    keymap = xkb_keymap_new_from_string(context, 
                                        desc, 
                                        XKB_KEYMAP_FORMAT_TEXT_V1, 
                                        XKB_KEYMAP_COMPILE_NO_FLAGS);
    /* clang-format on */
    if (unlikely(!keymap))
        goto fail1;

    state = xkb_state_new(keymap);
    if (unlikely(!state))
        goto fail2;

    xkb_destroy(xkb);

    xkb->context = context;
    xkb->keymap = keymap;
    xkb->state = state;

    return 0;

fail2:
    xkb_keymap_unref(keymap);
fail1:
    xkb_context_unref(context);

    return -1;
}

#endif /* CONFIG_USE_WAYLAND */
