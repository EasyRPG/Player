#include <cassert>
#include <cstdlib>
#include "platform.h"
#include "doctest.h"

TEST_SUITE_BEGIN("Platform");

namespace {
	const std::string onekb = EP_TEST_PATH "/platform/1kb";
	const std::string empty = EP_TEST_PATH "/platform/empty";
	const std::string folder = EP_TEST_PATH "/platform/folder";
	const std::string bad = EP_TEST_PATH "/platform/!!!nonexistant!!!";
}

TEST_CASE("Exists") {
	CHECK(Platform::File(empty).Exists());
	CHECK(Platform::File(folder).Exists());
	CHECK(!Platform::File(bad).Exists());
}

TEST_CASE("IsFile") {
	CHECK(Platform::File(empty).IsFile(false));
	CHECK(!Platform::File(folder).IsFile(false));
	CHECK(!Platform::File(bad).IsFile(false));
}

TEST_CASE("IsDirectory") {
	CHECK(!Platform::File(empty).IsDirectory(false));
	CHECK(Platform::File(folder).IsDirectory(false));
	CHECK(!Platform::File(bad).IsDirectory(false));
}

TEST_CASE("GetType") {
	CHECK(Platform::File(empty).GetType(false) == Platform::FileType::File);
	CHECK(Platform::File(folder).GetType(false) == Platform::FileType::Directory);
	CHECK(Platform::File(bad).GetType(false) == Platform::FileType::Unknown);
}

TEST_CASE("GetSize") {
	CHECK(Platform::File(empty).GetSize() == 0);
	CHECK(Platform::File(onekb).GetSize() == 1024);
	CHECK(Platform::File(bad).GetSize() == -1);
}

TEST_CASE("ReadDirectory") {
	Platform::Directory dir(EP_TEST_PATH "/platform");

	CHECK(dir);

	int iterations = 0;
	int matches = 0;

	while (dir.Read()) {
		if (dir.GetEntryName() == "folder") {
			++matches;
			CHECK(dir.GetEntryType() == Platform::FileType::Directory);
		} else if (dir.GetEntryName() == "empty" ||
				dir.GetEntryName() == "1kb") {
			++matches;
			CHECK(dir.GetEntryType() == Platform::FileType::File);
		}
		++iterations;
	}

	CHECK(matches == 3);
	// Could contain . and .. but not more
	CHECK(iterations <= 5);
}

TEST_SUITE_END();
