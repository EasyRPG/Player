#include "filesystem.h"
#include "filefinder.h"
#include "main_data.h"
#include "doctest.h"
#include "player.h"

TEST_SUITE_BEGIN("Tar filesystem");

TEST_CASE("Initialization") {
	CHECK(FileFinder::Root().Create(EP_TEST_PATH "/filesystem/test.tar"));
	CHECK(!FileFinder::Root().Create(EP_TEST_PATH "/filesystem/missing"));
	CHECK(!FileFinder::Root().Create(EP_TEST_PATH "/filesystem/corrupt.tar"));
	CHECK(!FileFinder::Root().Create(EP_TEST_PATH "/filesystem/non-ustar.tar"));
	CHECK(FileFinder::Root().Create(EP_TEST_PATH "/filesystem/unterminated.tar"));
}

TEST_CASE("Read errors") {
	auto test = FileFinder::Root().Create(EP_TEST_PATH "/filesystem/unterminated.tar");
	CHECK(test);

	CHECK(test.OpenInputStream("valid"));
	CHECK(!test.OpenInputStream("directory"));
	CHECK(!test.OpenInputStream("truncated"));
}

TEST_CASE("Tree introspection") {
	auto test = FileFinder::Root().Create(EP_TEST_PATH "/filesystem/test.tar");
	CHECK(test);

	CHECK(test.Exists("file1"));
	CHECK(test.IsFile("file1"));
	CHECK(test.IsDirectory("dir1", false));
	CHECK(!test.IsFile("dir1/subdirectory"));
	CHECK(test.GetFilesize("file1") == 14);
	CHECK(!test.IsFile("dfhgcjnhcjcjh"));
	CHECK(!test.IsDirectory("dfhgcjnhcjcjh", false));
}

TEST_CASE("Reading files") {
	auto test = FileFinder::Root().Create(EP_TEST_PATH "/filesystem/test.tar");
	CHECK(test);

	auto stream = test.OpenInputStream("file1");
	CHECK(stream);
	std::string data(std::istreambuf_iterator<char>(stream), {});
	CHECK(data == "Hello, world!\n");
}
