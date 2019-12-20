#include "filefinder.h"
#include "main_data.h"
#include "doctest.h"

TEST_SUITE_BEGIN("DirectoryTree");

TEST_CASE("CreateDirectoryTree") {
	Main_Data::Init();
	FileFinder::CreateDirectoryTree(Main_Data::GetProjectPath());
	FileFinder::Quit();
}

TEST_SUITE_END();
