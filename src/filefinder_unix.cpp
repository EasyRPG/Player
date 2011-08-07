/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
// 
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

#if UNIX || DINGOO || GPH

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cstring>
#include <cstdlib>
#include <string>
#include <dirent.h>
#include <unistd.h>

#include <errno.h>
#include "utils.h"
#include "filefinder.h"
#include "output.h"
#include "player.h"

typedef std::map<std::string, std::string> string_map;
typedef std::map<std::string, string_map> directory_map;

////////////////////////////////////////////////////////////
/// Global Variables
////////////////////////////////////////////////////////////
struct Tree {
	std::string root;
	string_map dirs;
	directory_map files;
};

namespace FileFinder {
	std::vector<Tree*> trees;

	std::string Find(const std::string& _dir,
					 const std::string& _file,
					 const char* const exts[]);
	std::string FindDefault(const std::string& dir, const std::string& file);
	std::string FindImage(const std::string& dir, const std::string& file);
	std::string FindSound(const std::string& dir, const std::string& file);
	std::string FindMusic(const std::string& dir, const std::string& file);
	std::string FindVideo(const std::string& dir, const std::string& file);
	std::string FindFont(const std::string& name);
	std::string DefaultFont();
}

////////////////////////////////////////////////////////////
/// Utility functions
////////////////////////////////////////////////////////////
static bool isdir(const std::string& path) {
	std::string test = path + "/.";
	return access(test.c_str(), F_OK) == 0;
}

static string_map scandir(const std::string& path, bool dirs = false) {
	string_map result;

	DIR* dir = opendir(path.c_str());
	if (!dir) {
		Output::Error("Error opening dir %s: %s", path.c_str(),
					  strerror(errno));
		return result;
	}

	struct dirent* dirent;

	while ((dirent = readdir(dir)) != NULL) {
		if (dirent->d_name[0] == '.')
			continue;
		if (dirs != isdir(path + "/" + dirent->d_name))
			continue;
		std::string name = dirent->d_name;
		std::string lname = Utils::LowerCase(name);
		result[lname] = name;
	}

	closedir(dir);

	return result;
}

static Tree* scandirs(const std::string& root) {
	Tree* tree = new Tree;

	tree->root = root;
	tree->dirs = scandir(root, true);
	tree->dirs["."] = ".";

	string_map::const_iterator it;
	for (it = tree->dirs.begin(); it != tree->dirs.end(); it++) {
		string_map m = scandir(root + "/" + it->second);
		if (!m.empty())
			tree->files[it->first] = m;
	}

	return tree;
}

////////////////////////////////////////////////////////////
/// Initialize
////////////////////////////////////////////////////////////
void FileFinder::Init() {
	// TODO find default paths
	trees.push_back(scandirs("."));
}

namespace FileFinder {
	void AddPaths(const char *_path);
}

void FileFinder::AddPaths(const char *_path) {
	if (_path == NULL)
		return;
	std::string path(_path);
	if (!isdir(path))
		return;

	size_t size = path.size();
	for (size_t start = 0; start < size; ) {
		size_t end = path.find(':');
		if (end == path.npos)
			end = size;
		const std::string& dir = path.substr(start, end - start);
		if (!dir.empty())
			trees.push_back(scandirs(dir));
		start = end + 1;
	}
}

void FileFinder::InitRtpPaths() {
	if (Player::engine == Player::EngineRpg2k)
		AddPaths(getenv("RPG2K_RTP_PATH"));
	else if (Player::engine == Player::EngineRpg2k3)
		AddPaths(getenv("RPG2K3_RTP_PATH"));
	AddPaths(getenv("RPG_RTP_PATH"));
}

////////////////////////////////////////////////////////
/// Quit FileFinder.
////////////////////////////////////////////////////////
void FileFinder::Quit() {
	std::vector<Tree*>::iterator it;
	for (it = trees.begin(); it != trees.end(); ++it) {
		delete *it;
	}
}

////////////////////////////////////////////////////////////
/// Check if file exists
////////////////////////////////////////////////////////////
std::string FileFinder::Find(const std::string& _dir,
							 const std::string& _file,
							 const char* const exts[]) {
	std::string dir = Utils::LowerCase(_dir);
	std::string file = Utils::LowerCase(_file);
	std::vector<Tree*>::const_iterator it;
	for (it = trees.begin(); it != trees.end(); it++) {
		Tree* tree = *it;
		std::string& dirname = tree->dirs[dir];
		if (dirname.empty())
			continue;
		std::string dirpath = tree->root + "/" + dirname;
		for (const char*const* pext = exts; *pext != NULL; pext++) {
			std::string& filename = tree->files[dir][file + *pext];
			if (!filename.empty())
				return dirpath + "/" + filename;
		}
	}

	return "";
}

////////////////////////////////////////////////////////////
/// Find file
////////////////////////////////////////////////////////////
std::string FileFinder::FindDefault(const std::string& dir, const std::string& file) {
	static const char* no_exts[] = {"", NULL};
	return Find(dir, file, no_exts);
}

////////////////////////////////////////////////////////////
/// Find image
////////////////////////////////////////////////////////////
std::string FileFinder::FindImage(const std::string& dir, const std::string& file) {
	return Find(dir, file, IMG_TYPES);
}

////////////////////////////////////////////////////////////
/// Find sound
////////////////////////////////////////////////////////////
std::string FileFinder::FindSound(const std::string& file) {
	return Find("Sound", file, SOUND_TYPES);
}
////////////////////////////////////////////////////////////
/// Find music
////////////////////////////////////////////////////////////
std::string FileFinder::FindMusic(const std::string& file) {
	return Find("Music", file, MUSIC_TYPES);
}

////////////////////////////////////////////////////////////
/// Find font
////////////////////////////////////////////////////////////
std::string FileFinder::FindFont(const std::string& file) {
	return Find("Font", file, FONTS_TYPES);
}

////////////////////////////////////////////////////////////
std::string FileFinder::DefaultFont() {
	static std::string fonts[] = DEFAULT_FONTS;
	for (const std::string* pfont = fonts; !pfont->empty(); pfont++)
		if (!FindFont(*pfont).empty())
			return *pfont;
	return "";
}

////////////////////////////////////////////////////////////
FILE* FileFinder::fopenUTF8(const std::string& name_utf8, const std::string& mode) {
	return fopen(name_utf8.c_str(), mode.c_str());
}

#endif
