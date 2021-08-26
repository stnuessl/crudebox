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

#ifndef XKB_H_
#define XKB_H_

#include <stdbool.h>

#include <xkbcommon/xkbcommon.h>

struct xkb {
    struct xkb_keymap *keymap;
    struct xkb_context *context;
    struct xkb_state *state;
};

static inline void xkb_init(struct xkb *xkb)
{
    xkb->context = NULL;
    xkb->keymap = NULL;
    xkb->state = NULL;
}

static inline void xkb_destroy(struct xkb *xkb)
{
    if (xkb->context)
        xkb_context_unref(xkb->context);

    if (xkb->keymap)
        xkb_keymap_unref(xkb->keymap);

    if (xkb->state)
        xkb_state_unref(xkb->state);
}

static inline bool xkb_ready(const struct xkb *xkb)
{
    return xkb->state != NULL;
}

int xkb_set_keymap(struct xkb *xkb, const char *desc);

static inline xkb_keysym_t xkb_get_sym(struct xkb *xkb, uint32_t key)
{
    return xkb_state_key_get_one_sym(xkb->state, key + 8);
}

static inline bool xkb_mod_active(const struct xkb *xkb, const char *name)
{
    return xkb_state_mod_name_is_active(xkb->state,
                                        name,
                                        XKB_STATE_MODS_EFFECTIVE) > 0;
}

static inline void xkb_state_update(struct xkb *xkb,
                                    uint32_t mods_depressed,
                                    uint32_t mods_latched,
                                    uint32_t mods_locked,
                                    uint32_t group)
{
    (void) xkb_state_update_mask(xkb->state,
                                 mods_depressed,
                                 mods_latched,
                                 mods_locked,
                                 0u,
                                 0u,
                                 group);
}

static inline bool xkb_keysym_is_modifier(xkb_keysym_t symbol)
{
    return symbol >= XKB_KEY_Shift_L && symbol <= XKB_KEY_Hyper_R;
}

#endif /* XKB_H_ */
