#include "filefinder.h"
#include <SDL.h>
#include <cstdlib>

static void CheckIsRPG2kProject() {
	EASYRPG_SHARED_PTR<FileFinder::ProjectTree> const
		tree = FileFinder::CreateProjectTree(".");
	assert(FileFinder::IsRPG2kProject(*tree));
}

static void CheckIsDirectory() {
	assert(FileFinder::IsDirectory("."));
}

extern "C" int main(int, char**) {
	FileFinder::Init();

	CheckIsDirectory();
	CheckIsRPG2kProject();

	FileFinder::Quit();

	return EXIT_SUCCESS;
}
