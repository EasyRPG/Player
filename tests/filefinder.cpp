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

	Player::escape_symbol = "\\";

	auto tree = FileFinder::CreateDirectoryTree(EP_TEST_PATH "/game");
	CHECK(FileFinder::IsRPG2kProject(*tree));

	FileFinder::SetDirectoryTree(std::move(tree));
	CHECK(!FileFinder::FindImage("CharSet", "Chara1").empty());

	Player::escape_symbol = "";
}

TEST_CASE("IsNotRPG2kProject") {
	Main_Data::Init();

	auto tree = FileFinder::CreateDirectoryTree(EP_TEST_PATH "/notagame");
	CHECK(!FileFinder::IsRPG2kProject(*tree));
}

TEST_CASE("MakeCanonical") {
	auto make_canonical = [](const char* s, int deep) {
		auto canon = FileFinder::MakeCanonical(s, deep);
		#ifdef _WIN32
		std::replace(canon.begin(), canon.end(), '\\', '/');
		#endif
		return canon;
	};

	CHECK(make_canonical("/folder/file", 1) == "/folder/file");
	CHECK(make_canonical("folder/file", 1) == "folder/file");
	CHECK(make_canonical("file", 1) == "file");
	CHECK(make_canonical("/path/../path2", 1) == "/path2");
	CHECK(make_canonical("folder/././//file", 0) == "folder/file");
	CHECK(make_canonical("folder/././//file", 0) == "folder/file");
	CHECK(make_canonical("folder/../file", 1) == "file");
	CHECK(make_canonical("../folder/folder2/../file", 1) == "folder/file");
	CHECK(make_canonical("folder/../../file", 1) == "file");

	Player::escape_symbol = "\\";
	CHECK(make_canonical("..\\folder\\folder2\\..\\file", 1) == "folder/file");
	CHECK(make_canonical("folder\\..\\..\\file", 1) == "file");
	Player::escape_symbol = "";
}

TEST_CASE("GetPathAndFilename") {
	// This function converts all paths to "/" to simplify the DirectoryTree
	// No non-canonical paths tested, the path is expected to be canonical

	std::string path, file;

	std::tie(path, file) = FileFinder::GetPathAndFilename("folder/file");
	CHECK(path == "folder");
	CHECK(file == "file");

	std::tie(path, file) = FileFinder::GetPathAndFilename("/file");
	CHECK(path == "");
	CHECK(file == "file");

	std::tie(path, file) = FileFinder::GetPathAndFilename("file");
	CHECK(path == "");
	CHECK(file == "file");

	std::tie(path, file) = FileFinder::GetPathAndFilename("folder/folder2/file");
	CHECK(path == "folder/folder2");
	CHECK(file == "file");

	Player::escape_symbol = "\\";
	std::tie(path, file) = FileFinder::GetPathAndFilename("folder\\folder2\\file");
	CHECK(path == "folder/folder2");
	CHECK(file == "file");
	Player::escape_symbol = "";
}

TEST_SUITE_END();
