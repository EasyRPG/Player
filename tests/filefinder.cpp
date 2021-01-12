#include <cassert>
#include <cstdlib>
#include "filefinder.h"
#include "player.h"
#include "main_data.h"
#include "doctest.h"

static bool skip_tests() {
#ifdef EMSCRIPTEN
	return true;
#else
	return false;
#endif
}

TEST_SUITE_BEGIN("FileFinder" * doctest::skip(skip_tests()));

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

	auto tree = FileFinder::CreateDirectoryTree(EP_TEST_PATH "/game");
	CHECK(FileFinder::IsRPG2kProject(*tree));

	Player::escape_symbol = "\\";
	FileFinder::SetDirectoryTree(std::move(tree));
	CHECK(!FileFinder::FindImage("CharSet", "Chara1").empty());
}

TEST_CASE("IsNotRPG2kProject") {
	Main_Data::Init();

	auto tree = FileFinder::CreateDirectoryTree(EP_TEST_PATH "/notagame");
	CHECK(!FileFinder::IsRPG2kProject(*tree));
}

TEST_SUITE_END();
