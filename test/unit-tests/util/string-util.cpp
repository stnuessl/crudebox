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

#ifdef __cplusplus
extern "C" {
#endif

#include <util/macro.h>

#include <util/string-util.h>

#ifdef __cplusplus
}
#endif

TEST(strconcat2a, 001)
{
    char *str1, *str2, *str3, *str4;

    strconcat2a(&str1, "aa", "bb");
    strconcat2a(&str2, "aa", "");
    strconcat2a(&str3, "", "bb");
    strconcat2a(&str4, "", "");

    ASSERT_STREQ(str1, "aabb");
    ASSERT_STREQ(str2, "aa");
    ASSERT_STREQ(str3, "bb");
    ASSERT_STREQ(str4, "");
}

TEST(streq, 001)
{
    ASSERT_TRUE(streq("a", "a"));
    ASSERT_TRUE(streq("aa", "aa"));
    ASSERT_TRUE(streq("aaa", "aaa"));
}

TEST(streq, 002)
{
    ASSERT_FALSE(streq("a", "b"));
    ASSERT_FALSE(streq("aa", "bb"));
    ASSERT_FALSE(streq("aaa", "aaaa"));
    ASSERT_FALSE(streq("aaaa", "aaa"));
}

TEST(strlower, 001)
{
    char str[] = "!!AABB11";

    ASSERT_STREQ("!!aabb11", strlower(str));
}

TEST(strupper, 001)
{
    char str[] = "!!aabb[[";

    ASSERT_STREQ("!!AABB[[", strupper(str));
}

TEST(strprefix, 001)
{
    ASSERT_EQ(0, strprefix("", ""));
    ASSERT_EQ(0, strprefix("a", ""));
    ASSERT_EQ(0, strprefix("a", "a"));
    ASSERT_EQ(0, strprefix("aa", "a"));
    ASSERT_EQ(0, strprefix("ab", "a"));
    ASSERT_EQ(0, strprefix("ab", "ab"));
    ASSERT_EQ(0, strprefix("abc", "ab"));

    ASSERT_NE(0, strprefix("", "a"));
    ASSERT_NE(0, strprefix("", "b"));
    ASSERT_NE(0, strprefix("a", "aa"));
    ASSERT_NE(0, strprefix("b", "a"));
    ASSERT_NE(0, strprefix("aa", "bb"));
    ASSERT_NE(0, strprefix("aa", "aaa"));
}

TEST(strsuffix, 001)
{
    ASSERT_EQ(0, strsuffix("", ""));
    ASSERT_EQ(0, strsuffix("a", ""));
    ASSERT_EQ(0, strsuffix("a", "a"));
    ASSERT_EQ(0, strsuffix("aa", "a"));
    ASSERT_EQ(0, strsuffix("ab", "b"));
    ASSERT_EQ(0, strsuffix("ab", "ab"));

    ASSERT_NE(0, strsuffix("", "a"));
    ASSERT_NE(0, strsuffix("a", "aa"));
    ASSERT_NE(0, strsuffix("a", "b"));
    ASSERT_NE(0, strsuffix("a", "bb"));
    ASSERT_NE(0, strsuffix("ab", "bc"));
}

TEST(strconcat, 001)
{
    const char *buf[] = {"aa", "bb", "cc", "dd"};
    const char *results[] = {"", "aa", "aabb", "aabbcc", "aabbccdd"};

    for (int i = 0; i <= ARRAY_SIZE(buf); ++i) {
        char *str = strconcat(buf, i);

        ASSERT_STREQ(results[i], str);

        free(str);
    }
}

TEST(strconcat2, 001)
{
    char *str1, *str2, *str3, *str4;

    str1 = strconcat2("aa", "bb");
    str2 = strconcat2("aa", "");
    str3 = strconcat2("", "bb");
    str4 = strconcat2("", "");

    ASSERT_STREQ("aabb", str1);
    ASSERT_STREQ("aa", str2);
    ASSERT_STREQ("bb", str3);
    ASSERT_STREQ("", str4);

    free(str1);
    free(str2);
    free(str3);
    free(str4);
}

TEST(strnconcat, 001)
{
    const char *buf[] = {"aa", "bb", "cc", "dd"};
    char mem[9];

    ASSERT_STREQ(strnconcat(mem, 1, buf, ARRAY_SIZE(buf)), "");
    ASSERT_STREQ(strnconcat(mem, 2, buf, ARRAY_SIZE(buf)), "a");
    ASSERT_STREQ(strnconcat(mem, 3, buf, ARRAY_SIZE(buf)), "aa");
    ASSERT_STREQ(strnconcat(mem, 5, buf, ARRAY_SIZE(buf)), "aabb");
    ASSERT_STREQ(strnconcat(mem, 6, buf, ARRAY_SIZE(buf)), "aabbc");
    ASSERT_STREQ(strnconcat(mem, 7, buf, ARRAY_SIZE(buf)), "aabbcc");
    ASSERT_STREQ(strnconcat(mem, 9, buf, ARRAY_SIZE(buf)), "aabbccdd");
    ASSERT_STREQ(strnconcat(mem, 10, buf, ARRAY_SIZE(buf)), "aabbccdd");
    ASSERT_STREQ(strnconcat(mem, 11, buf, ARRAY_SIZE(buf)), "aabbccdd");
    ASSERT_STREQ(strnconcat(mem, 12, buf, ARRAY_SIZE(buf)), "aabbccdd");
    ASSERT_STREQ(strnconcat(mem, 99, buf, ARRAY_SIZE(buf)), "aabbccdd");
}

TEST(strnconcat2, 001)
{
    char mem[9];

    ASSERT_STREQ(strnconcat2(mem, 1, "aa", "bb"), "");
    ASSERT_STREQ(strnconcat2(mem, 2, "aa", "bb"), "a");
    ASSERT_STREQ(strnconcat2(mem, 4, "aa", "bb"), "aab");
    ASSERT_STREQ(strnconcat2(mem, 5, "aa", "bb"), "aabb");
    ASSERT_STREQ(strnconcat2(mem, 10, "aa", "bb"), "aabb");
    ASSERT_STREQ(strnconcat2(mem, 11, "aa", "bb"), "aabb");
    ASSERT_STREQ(strnconcat2(mem, 12, "aa", "bb"), "aabb");
    ASSERT_STREQ(strnconcat2(mem, 99, "aa", "bb"), "aabb");
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
