#include <cassert>
#include <cstdlib>
#include "filefinder.h"
#include "player.h"
#include "reader_util.h"
#include "main_data.h"

namespace {
	void CheckIsRPG2kProject() {
		std::shared_ptr<FileFinder::DirectoryTree> const
			tree = FileFinder::CreateDirectoryTree(".");
		assert(FileFinder::IsRPG2kProject(*tree));
		FileFinder::SetDirectoryTree(tree);
	}

	void CheckIsDirectory() {
		assert(FileFinder::IsDirectory("."));
	}

	void CheckEnglishFilename() {
		assert(!FileFinder::FindImage("CharSet", "Chara1").empty());
	}
}

int main(int, char**) {
	Main_Data::Init();
	
	CheckIsDirectory();
	CheckIsRPG2kProject();

	Player::GetEncoding();
	Player::escape_symbol = ReaderUtil::Recode("\\", Player::encoding);
	Player::engine = Player::EngineRpg2k;
	FileFinder::InitRtpPaths();

	CheckEnglishFilename();

	FileFinder::Quit();

	return EXIT_SUCCESS;
}
