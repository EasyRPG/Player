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
}

TEST_CASE("IsRPG2kProject") {
	Main_Data::Init();

	std::shared_ptr<FileFinder::DirectoryTree> const tree = FileFinder::CreateDirectoryTree(".");
	CHECK(FileFinder::IsRPG2kProject(*tree));
	FileFinder::SetDirectoryTree(tree);
}

TEST_CASE("Project contains English filename") {
	Main_Data::Init();
	Player::escape_symbol = "\\";
	Player::engine = Player::EngineRpg2k;

	CHECK(!FileFinder::FindImage("CharSet", "Chara1").empty());
}

TEST_SUITE_END();
