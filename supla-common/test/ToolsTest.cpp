/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "gtest/gtest.h"
#include "ToolsTest.h"
#include "../tools.h"


namespace {

	class ToolsTest : public ::testing::Test {
	protected:
	};

	TEST_F(ToolsTest, st_file_exists) {
		ASSERT_TRUE(st_file_exists("/dev/null") == 1);
		ASSERT_FALSE(st_file_exists(NULL) == 1);
	}

	TEST_F(ToolsTest, pid_file) {

		char file[] = "/tmp/QDZgKvbTrNh8.pid";

		ASSERT_FALSE(st_file_exists(file) == 1);
		st_setpidfile(file);
		ASSERT_TRUE(st_file_exists(file) == 1);
		st_delpidfile(file);
		ASSERT_FALSE(st_file_exists(file) == 1);
	}

	TEST_F(ToolsTest, st_bin2hex) {

		char src[2] = {(char)255,(char)255};
		char dest[5];

		EXPECT_STREQ("FFFF", st_bin2hex(dest, src, 2));
		EXPECT_EQ(NULL, st_bin2hex(NULL, NULL, 0));

	}

	TEST_F(ToolsTest, st_guid2hex) {

		EXPECT_EQ(16, SUPLA_GUID_SIZE);
		EXPECT_EQ(33, SUPLA_GUID_HEXSIZE);

		const char guid[SUPLA_GUID_SIZE] = {
				(char)255,
				(char)255,
				(char)255,
				(char)255,
				(char)0,
				(char)0,
				(char)0,
				(char)0,
				(char)255,
				(char)255,
				(char)255,
				(char)255,
				(char)0,
				(char)0,
				(char)0,
				(char)0
		};

		char result[SUPLA_GUID_HEXSIZE];
		st_guid2hex(result, guid);

		EXPECT_STREQ("FFFFFFFF00000000FFFFFFFF00000000", result);
	}


	TEST_F(ToolsTest, st_authkey2hex) {

		EXPECT_EQ(16, SUPLA_AUTHKEY_SIZE);
		EXPECT_EQ(33, SUPLA_AUTHKEY_HEXSIZE);

		const char guid[SUPLA_AUTHKEY_SIZE] = {
				(char)255,
				(char)255,
				(char)255,
				(char)255,
				(char)0,
				(char)0,
				(char)0,
				(char)0,
				(char)255,
				(char)255,
				(char)255,
				(char)255,
				(char)0,
				(char)0,
				(char)0,
				(char)0
		};

		char result[SUPLA_AUTHKEY_HEXSIZE];
		st_authkey2hex(result, guid);

		EXPECT_STREQ("FFFFFFFF00000000FFFFFFFF00000000", result);
	}

	TEST_F(ToolsTest, st_str2hex) {

		const char src[] = "SUPLA";
		char dest[15];

		EXPECT_STREQ("5355504C41", st_str2hex(dest, src, 5));
	}

	TEST_F(ToolsTest, st_read_randkey_from_file) {

		char file[] = "/tmp/t7oosvsTZS89";
		char key1[4];
		char key2[4];

		memset(key1, 0 ,4);
		memset(key2, 0 ,4);

		EXPECT_EQ(1, st_read_randkey_from_file(file, key1, 4, 1));
		ASSERT_TRUE(st_file_exists(file) == 1);
		EXPECT_NE(0, memcmp(key1, key2, 4));
		unlink(file);

	}
}


