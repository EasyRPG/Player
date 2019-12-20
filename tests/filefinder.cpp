#include <cassert>
#include <cstdlib>
#include "filefinder.h"
#include "player.h"
#include "main_data.h"
#include "doctest.h"

TEST_SUITE_BEGIN("FileFinder");

TEST_CASE("IsDirectory") {
	Main_Data::Init();

	CHECK(FileFinder::IsDirectory(".", false));
	CHECK(FileFinder::IsDirectory(".", true));
	CHECK(FileFinder::IsDirectory(EP_TEST_PATH "/game", true));
	CHECK(FileFinder::IsDirectory(EP_TEST_PATH "/notagame", true));
	CHECK(!FileFinder::IsDirectory(EP_TEST_PATH "/game/RPG_RT.ldb", true));
}

TEST_CASE("IsRPG2kProject") {
	Main_Data::Init();

	std::shared_ptr<FileFinder::DirectoryTree> const tree = FileFinder::CreateDirectoryTree(EP_TEST_PATH "/game");
	CHECK(FileFinder::IsRPG2kProject(*tree));

	Player::escape_symbol = "\\";
	FileFinder::SetDirectoryTree(tree);
	CHECK(!FileFinder::FindImage("CharSet", "Chara1").empty());
}

TEST_CASE("IsNotRPG2kProject") {
	Main_Data::Init();

	std::shared_ptr<FileFinder::DirectoryTree> const tree = FileFinder::CreateDirectoryTree(EP_TEST_PATH "/notagame");
	CHECK(!FileFinder::IsRPG2kProject(*tree));
}

TEST_SUITE_END();
