#include <cstdlib>
#include "filefinder.h"
#include "player.h"
#include "main_data.h"

int main(int argc, char** argv) {
	Player::ParseCommandLine(argc, argv);
	Main_Data::Init();

	FileFinder::CreateDirectoryTree(Main_Data::GetProjectPath());
	FileFinder::Quit();

	return EXIT_SUCCESS;
}
