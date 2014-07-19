#include <cstdlib>
#include "filefinder.h"
#include "player.h"
#include "main_data.h"

namespace {

	void CheckIsRPG2kProject() {
		EASYRPG_SHARED_PTR<FileFinder::ProjectTree> const tree = FileFinder::CreateProjectTree(".");
		assert(FileFinder::IsRPG2kProject(*tree));
	}

	void CheckIsDirectory() { assert(FileFinder::IsDirectory(".")); }

	void CheckEnglishFilename() { assert(!FileFinder::FindImage("Backdrop", "castle").empty()); }
}

int main(int, char**) {
	Main_Data::Init();
	Player::engine = Player::EngineRpg2k;
	FileFinder::Init();
	FileFinder::InitRtpPaths();

	CheckIsDirectory();
	CheckIsRPG2kProject();
	CheckEnglishFilename();

	FileFinder::Quit();

	return EXIT_SUCCESS;
}
