#include "filesystem.h"
#include "filefinder.h"
#include "main_data.h"
#include "doctest.h"
#include "player.h"

#define ZIP_PATH EP_TEST_PATH "/filesystem/test.zip"
#define ZIP_FOLDER_PATH EP_TEST_PATH "/filesystem/folder.zip"

TEST_SUITE_BEGIN("Filesystem ZIP");

TEST_CASE("Create") {
	CHECK(FileFinder::Root().Create(ZIP_PATH));
	CHECK(FileFinder::Root().Create(ZIP_PATH "/game"));
	CHECK(!FileFinder::Root().Create(ZIP_PATH "/!!!invalidpath!!!"));
}

TEST_CASE("Create inexistant ZIP") {
	CHECK(!FileFinder::Root().Create(EP_TEST_PATH "/!!!invalid.zip"));
}

TEST_CASE("Create: Folder ending in .zip") {
	CHECK(FileFinder::Root().Create(ZIP_FOLDER_PATH));
}

TEST_CASE("IsRPG2kProject") {
	auto fs = FileFinder::Root().Create(ZIP_PATH "/game");
	CHECK(FileFinder::IsRPG2kProject(fs));

	auto ext = Utils::MakeSvArray(".png");
	CHECK(!fs.FindFile("ExFont", ext).empty());
	CHECK(!fs.FindFile("Charset", "Chara1", ext).empty());
}

TEST_CASE("IsNotRPG2kProject") {
	auto fs = FileFinder::Root().Create(ZIP_PATH "/notagame");
	CHECK(!FileFinder::IsRPG2kProject(fs));
}

TEST_CASE("GetFilesize") {
	auto fs = FileFinder::Root().Create(ZIP_PATH);
	CHECK(fs.GetFilesize("1kb") == 1024);
}

TEST_CASE("File reading") {
	auto fs = FileFinder::Root().Create(ZIP_PATH);
	auto is = fs.OpenInputStream("text");
	CHECK(is);

	std::string line_out;
	CHECK(Utils::ReadLine(is, line_out));
	CHECK(line_out == "hello");

	is.seekg(1, std::ios_base::cur);
	CHECK(Utils::ReadLine(is, line_out));
	CHECK(line_out == "orld");

	is.seekg(3, std::ios_base::beg);
	CHECK(Utils::ReadLine(is, line_out));
	CHECK(line_out == "lo");
}

TEST_CASE("File IO error") {
	auto fs = FileFinder::Root().Create(ZIP_PATH);
	CHECK(!fs.OpenInputStream("game"));
	CHECK(!fs.OpenInputStream("!!!invalid_path"));
	CHECK(!fs.OpenOutputStream("not_supported"));
}

TEST_SUITE_END();
