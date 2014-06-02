#include <cstdlib>
#include "filefinder.h"
#include "player.h"

namespace filefinder_test_anon {

	void CheckIsRPG2kProject() {
		EASYRPG_SHARED_PTR<FileFinder::ProjectTree> const
			tree = FileFinder::CreateProjectTree(".");
		assert(FileFinder::IsRPG2kProject(*tree));
	}

	void CheckIsDirectory() {
		assert(FileFinder::IsDirectory("."));
	}

	void CheckEnglishFilename() {
		assert(!FileFinder::FindImage("Backdrop", "castle").empty());
	}

}

using namespace filefinder_test_anon;

int main(int, char**) {
	Player::engine = Player::EngineRpg2k;
	FileFinder::Init();
	FileFinder::InitRtpPaths();

	CheckIsDirectory();
	CheckIsRPG2kProject();
	CheckEnglishFilename();

	FileFinder::Quit();

	return EXIT_SUCCESS;
}
