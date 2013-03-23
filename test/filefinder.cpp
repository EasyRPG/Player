#include "filefinder.h"
#include <cstdlib>
#include <ciso646>

namespace {
void CheckIsRPG2kProject() {
	EASYRPG_SHARED_PTR<FileFinder::ProjectTree> const
		tree = FileFinder::CreateProjectTree(".");
	assert(FileFinder::IsRPG2kProject(*tree));
}

void CheckIsDirectory() {
	assert(FileFinder::IsDirectory("."));
}

void CheckEnglishFilename() {
	assert(not FileFinder::FindImage("Backdrop", "castle").empty());
}
}

int main(int, char**) {
	FileFinder::Init();

	CheckIsDirectory();
	CheckIsRPG2kProject();
	CheckEnglishFilename();

	FileFinder::Quit();

	return EXIT_SUCCESS;
}
