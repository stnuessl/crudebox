/*
 * Copyright (C) 2022   Steffen Nuessle
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fcntl.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <util/macro.h>
//#include <util/env.h>

#include <item-list.h>

class mock {
public:
    MOCK_METHOD(const char *, env_crudebox_cache, ());
    MOCK_METHOD(const char *, env_xdg_cache, ());
    MOCK_METHOD(const char *, env_home, ());

    MOCK_METHOD(int, io_util_read_all_str, (int, char **, size_t *) );

    MOCK_METHOD(int, stat, (const char *, struct stat *) );
    MOCK_METHOD(int, ioctl, (int, int, int *) );
    MOCK_METHOD(int, fstat, (int, struct stat *) );
    MOCK_METHOD(ssize_t, read, (int, void *, size_t));
};

static testing::StrictMock<mock> mock;

const char *env_crudebox_cache(void)
{
    return mock.env_crudebox_cache();
}

const char *env_xdg_cache(void)
{
    return mock.env_xdg_cache();
}

const char *env_home(void)
{
    return mock.env_home();
}

int io_util_read_all_str(int fd, char **buf, size_t *size)
{
    return mock.io_util_read_all_str(fd, buf, size);
}

int stat(const char *path, struct stat *buf)
{
    return mock.stat(path, buf);
}

int ioctl(int fd, int request, int *arg)
{
    return mock.ioctl(fd, request, arg);
}

int fstat(int fd, struct stat *buf)
{
    return mock.fstat(fd, buf);
}

ssize_t read(int fd, void *buf, size_t size)
{
    return mock.read(fd, buf, size);
}

#ifdef __cplusplus
}
#endif

TEST(item_list_init, 001)
{
    const char *dirs_list[] = {"/usr/bin", "/usr/bin:/usr/local/bin", NULL};
    const char cache[] = "/tmp/ut-crudebox.cache";
    const char cache_data[] = {"/usr/bin/:/usr/local/bin\n"
                               "5\n\n"
                               "a\nb\nc\nd\ne\n"};
    struct item_list list;
    struct stat st1, st2;

    memset(&st1, 0, sizeof(st1));
    memset(&st2, 0, sizeof(st2));

    st1.st_mtim.tv_sec = 1;

    (void) unlink(cache);

    EXPECT_CALL(mock, fstat)
        .WillRepeatedly(testing::DoAll(testing::SetArgPointee<1>(st1),
                                       testing::SetErrnoAndReturn(0, 0)));

    EXPECT_CALL(mock, stat)
        .WillRepeatedly(testing::DoAll(testing::SetArgPointee<1>(st2),
                                       testing::SetErrnoAndReturn(0, 0)));

    EXPECT_CALL(mock, io_util_read_all_str)
        .WillOnce(testing::DoAll(testing::SetArgPointee<1>(nullptr),
                                 testing::Return(-1)))
        .WillOnce(testing::DoAll(testing::SetArgPointee<1>(strdup(cache_data)),
                                 testing::Return(0)));

    EXPECT_CALL(mock, ioctl)
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(0),
                                 testing::SetErrnoAndReturn(0, 0)));

    for (int i = 0; i < ARRAY_SIZE(dirs_list); ++i) {
        EXPECT_CALL(mock, env_crudebox_cache).WillOnce(testing::Return(cache));

        //        EXPECT_CALL(mock, env_xdg_cache)
        //            .WillOnce(testing::ReturnNull());
        //
        //        EXPECT_CALL(mock, env_home)
        //            .WillOnce(testing::Return(home));

        item_list_init(&list, dirs_list[i]);
        item_list_destroy(&list);
    }

    ASSERT_EQ(0, unlink(cache));
}

TEST(item_list_init, 002)
{

}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
