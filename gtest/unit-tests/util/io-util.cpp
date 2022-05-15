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

#include <fcntl.h>
#include <stdint.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <util/macro.h>

#include <util/io-util.h>

class mock {
public:
    MOCK_METHOD(int, fstat, (int, struct stat *__restrict) );
    MOCK_METHOD(ssize_t, read, (int, void *, size_t));
};

static testing::StrictMock<mock> mock;

int fstat(int fd, struct stat *__restrict buf)
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

TEST(io_util_read, 001)
{
    char buf[16];
    int fd;

    fd = 3;

    EXPECT_CALL(mock, read(fd, testing::_, testing::_))
        .Times(testing::Exactly(1))
        .WillOnce(testing::Return(ARRAY_SIZE(buf)));

    ASSERT_EQ(0, io_util_read(fd, buf, ARRAY_SIZE(buf)));
}

TEST(io_util_read, 002)
{
    char buf[16];
    int fd;

    fd = 3;

    EXPECT_CALL(mock, read(fd, testing::_, testing::_))
        .Times(testing::Exactly(2))
        .WillRepeatedly(testing::Return(ARRAY_SIZE(buf) / 2));

    ASSERT_EQ(0, io_util_read(fd, buf, ARRAY_SIZE(buf)));
}

TEST(io_util_read, 003)
{
    char buf[16];
    int fd;

    fd = 3;

    EXPECT_CALL(mock, read(fd, testing::_, testing::_))
        .Times(testing::Exactly(16))
        .WillRepeatedly(testing::Return(ARRAY_SIZE(buf) / 16));

    ASSERT_EQ(0, io_util_read(fd, buf, ARRAY_SIZE(buf)));
}

TEST(io_util_read, 004)
{
    char buf[16];
    int fd;

    fd = 3;

    EXPECT_CALL(mock, read(fd, testing::_, testing::_))
        .WillOnce(testing::SetErrnoAndReturn(EINTR, -1))
        .WillOnce(testing::Return(ARRAY_SIZE(buf)));

    ASSERT_EQ(0, io_util_read(fd, buf, ARRAY_SIZE(buf)));
}

TEST(io_util_read, 005)
{
    char buf[16];
    int fd;

    fd = 3;

    EXPECT_CALL(mock, read(fd, testing::_, testing::_))
        .WillOnce(testing::SetErrnoAndReturn(EBADF, -1));

    ASSERT_EQ(-EBADF, io_util_read(fd, buf, ARRAY_SIZE(buf)));
}

TEST(io_util_path_read_all_str, 001)
{
    const char *path = "/usr/include/stdlib.h";
    struct stat st;
    char *buf;
    size_t size;

    st.st_size = 1024;

    EXPECT_CALL(mock, fstat)
        .WillOnce(DoAll(testing::SetArgPointee<1>(st),
                        testing::SetErrnoAndReturn(0, 0)));

    EXPECT_CALL(mock, read(testing::_, testing::_, st.st_size))
        .WillOnce(testing::SetErrnoAndReturn(0, st.st_size));

    ASSERT_EQ(0, io_util_path_read_all_str(path, &buf, &size));
    ASSERT_EQ(size, st.st_size);
    ASSERT_EQ('\0', buf[st.st_size]);

    free(buf);

    EXPECT_CALL(mock, fstat)
        .WillOnce(DoAll(testing::SetArgPointee<1>(st),
                        testing::SetErrnoAndReturn(0, 0)));

    EXPECT_CALL(mock, read(testing::_, testing::_, st.st_size))
        .WillOnce(testing::SetErrnoAndReturn(0, st.st_size));

    ASSERT_EQ(0, io_util_path_read_all_str(path, &buf, NULL));

    free(buf);
}

TEST(io_util_path_read_all_str, 002)
{
    const char *path = "/nonexistent/path";

    ASSERT_EQ(-ENOENT, io_util_path_read_all_str(path, NULL, NULL));
}

TEST(io_util_read_all_str, 001)
{
    EXPECT_CALL(mock, fstat).WillOnce(testing::SetErrnoAndReturn(EBADF, -1));

    ASSERT_EQ(-EBADF, io_util_read_all_str(-1, NULL, NULL));
}

TEST(io_util_read_all_str, 002)
{
    struct stat st;

    st.st_size = 4096;

    EXPECT_CALL(mock, fstat)
        .WillOnce(DoAll(testing::SetArgPointee<1>(st),
                        testing::SetErrnoAndReturn(0, 0)));

    EXPECT_CALL(mock, read).WillOnce(testing::SetErrnoAndReturn(EAGAIN, -1));

    ASSERT_EQ(-EAGAIN, io_util_read_all_str(-1, NULL, NULL));
}

TEST(io_util_path_read_all, 001)
{
    const char *path = "/usr/include/stdio.h";
    struct stat st;
    char *buf;
    size_t size;

    st.st_size = 255;

    EXPECT_CALL(mock, fstat)
        .WillOnce(DoAll(testing::SetArgPointee<1>(st),
                        testing::SetErrnoAndReturn(0, 0)));

    EXPECT_CALL(mock, read(testing::_, testing::_, st.st_size))
        .WillOnce(testing::SetErrnoAndReturn(0, st.st_size));

    ASSERT_EQ(0, io_util_path_read_all(path, &buf, &size));
    ASSERT_EQ(size, st.st_size);

    free(buf);
}

TEST(io_util_path_read_all, 002)
{
    const char *path = "/nonexistent/path";

    ASSERT_EQ(-ENOENT, io_util_path_read_all(path, NULL, NULL));
}

TEST(io_util_read_all, 001)
{
    struct stat st;
    char *buf;
    size_t size;

    st.st_size = 768;

    EXPECT_CALL(mock, fstat)
        .WillOnce(DoAll(testing::SetArgPointee<1>(st),
                        testing::SetErrnoAndReturn(0, 0)));

    EXPECT_CALL(mock, read(testing::_, testing::_, st.st_size))
        .WillOnce(testing::SetErrnoAndReturn(0, st.st_size));

    ASSERT_EQ(0, io_util_read_all(0, &buf, &size));
    ASSERT_EQ(size, st.st_size);

    free(buf);
}

TEST(io_util_read_all, 002)
{
    EXPECT_CALL(mock, fstat).WillOnce(testing::SetErrnoAndReturn(EBADF, -1));

    ASSERT_EQ(-EBADF, io_util_read_all(-1, NULL, NULL));
}

TEST(io_util_read_all, 003)
{
    struct stat st;

    st.st_size = 384;

    EXPECT_CALL(mock, fstat)
        .WillOnce(DoAll(testing::SetArgPointee<1>(st),
                        testing::SetErrnoAndReturn(0, 0)));

    EXPECT_CALL(mock, read(testing::_, testing::_, st.st_size))
        .WillOnce(testing::SetErrnoAndReturn(EAGAIN, -1));

    ASSERT_EQ(-EAGAIN, io_util_read_all(-1, NULL, NULL));
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
