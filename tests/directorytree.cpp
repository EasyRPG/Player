#include "filefinder.h"
#include "main_data.h"
#include "doctest.h"

static bool skip_tests() {
#ifdef EMSCRIPTEN
	return true;
#else
	return false;
#endif
}

TEST_SUITE_BEGIN("DirectoryTree" * doctest::skip(skip_tests()));

TEST_CASE("CreateDirectoryTree") {
	Main_Data::Init();
	FileFinder::CreateDirectoryTree(Main_Data::GetProjectPath());
	FileFinder::Quit();
}

TEST_SUITE_END();
