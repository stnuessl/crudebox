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

#ifndef TIMER_H_
#define TIMER_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef TIME_PERF

struct timer {
    const char *name;
    struct timespec time;
    clockid_t clock;
};

static inline void timer_destroy(const struct timer *timer)
{
    struct timespec now;
    uint64_t time;
    char *unit;

    (void) clock_gettime(timer->clock, &now);

    now.tv_sec -= timer->time.tv_sec;
    now.tv_nsec -= timer->time.tv_nsec;

    time = 1000 * 1000 * 1000 * now.tv_sec + now.tv_nsec;
    unit = "ns";

    if (time >= 20ull * 1000 * 1000 * 1000) {
        time /= (1ull * 1000 * 1000 * 1000);
        unit = "s";
    } else if (time >= 20ull * 1000 * 1000) {
        time /= (1ull * 1000 * 1000);
        unit = "ms";
    } else if (time >= 20ull * 1000) {
        time /= (1ull * 1000);
        unit = "us";
    }

    printf("Timer \"%s\": %ld %s\n", timer->name, time, unit);
}

#define TIMER_INIT(name_, clock_)                                              \
    __attribute__(                                                             \
        (__cleanup__(timer_destroy))) struct timer __FILE__##__LINE__ = {      \
        .name = (name_),                                                       \
        .clock = (clock_),                                                     \
    };                                                                         \
                                                                               \
    (void) clock_gettime(clock_, &(__FILE__##__LINE__).time)

#define TIMER_INIT_SIMPLE(clock_) TIMER_INIT(__func__, (clock_))

#else

#define TIMER_INIT(name_, clock_) while (0)

#define TIMER_INIT_SIMPLE(clock_) while (0)

#endif

#endif /* TIMER_H_ */
