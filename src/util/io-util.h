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

#ifndef IO_UTIL_H_
#define IO_UTIL_H_

int io_util_read(int fd, void *buf, size_t size);

int io_util_read_all(int fd, void **buf, size_t *size);

int io_util_read_all_str(int fd, char **buf, size_t *size);

int io_util_path_read_all(const char *path, void **buf, size_t *size);

int io_util_path_read_all_str(const char *path, char **buf, size_t *size);

#endif /* IO_UTIL_H_ */
