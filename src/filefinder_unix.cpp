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

#if UNIX || DINGOO || GEKKO || PSP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <cctype>
#include <dirent.h>
#include <unistd.h>

#ifdef GEKKO
	#include <cstdlib>
	#include <cstdio>
#endif

#include <errno.h>
#include "filefinder.h"
#include "output.h"

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

#define COUNT(x) (sizeof(x) / sizeof(*(x)))
#define VECTOR(x) x, x + COUNT(x)

namespace FileFinder {
	std::vector<Tree*> trees;

	std::string Find(const std::string& _dir,
					 const std::string& _file,
					 const std::string exts[]);
	std::string FindDefault(const std::string& dir, const std::string& file);
	std::string FindImage(const std::string& dir, const std::string& file);
	std::string FindSound(const std::string& dir, const std::string& file);
	std::string FindMusic(const std::string& dir, const std::string& file);
	std::string FindVideo(const std::string& dir, const std::string& file);
	std::string FindFont(const std::string& name);
	std::string DefaultFont();
}

#undef VECTOR

////////////////////////////////////////////////////////////
/// Utility functions
////////////////////////////////////////////////////////////
static std::string lower(const std::string& str) {
	std::string result = str;
	std::string::iterator it;
	for (it = result.begin(); it != result.end(); it++)
		*it = tolower(*it);
	return result;
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
		if (dirs) {
			std::string test = path + "/" + dirent->d_name + "/.";
			if (access(test.c_str(), F_OK) != 0)
				continue;
		}
		std::string name = dirent->d_name;
		std::string lname = lower(name);
		result[lname] = name;
	}

	closedir(dir);

	return result;
}

static Tree* scandirs(const std::string& root) {
	Tree* tree = new Tree;

	tree->root = root;
	tree->dirs = scandir(root, true);

	string_map::const_iterator it;
	for (it = tree->dirs.begin(); it != tree->dirs.end(); it++) {
		string_map m = scandir(it->second);
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

////////////////////////////////////////////////////////////
/// Check if file exists
////////////////////////////////////////////////////////////
std::string FileFinder::Find(const std::string& _dir,
							 const std::string& _file,
							 const std::string exts[]) {
	std::string dir = lower(_dir);
	std::string file = lower(_file);
	std::vector<Tree*>::const_iterator it;
	for (it = trees.begin(); it != trees.end(); it++) {
		Tree* tree = *it;
		std::string& dirname = tree->dirs[dir];
		if (dirname.empty())
			continue;
		std::string dirpath = tree->root + "/" + dirname;
		for (const std::string* pext = exts; !pext->empty(); pext++) {
			std::string& filename = tree->files[dir][file + *pext];
			if (filename.empty())
				continue;
			std::string result = dirpath + "/" + filename;
			if (access(result.c_str(), F_OK) == 0)
				return result;
		}
	}

	return "";
}

////////////////////////////////////////////////////////////
/// Find file
////////////////////////////////////////////////////////////
std::string FileFinder::FindDefault(const std::string& dir, const std::string& file) {
	static const std::string no_exts[] = {""};
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
	return Find("sound", file, SOUND_TYPES);
}
////////////////////////////////////////////////////////////
/// Find music
////////////////////////////////////////////////////////////
std::string FileFinder::FindMusic(const std::string& file) {
	return Find("music", file, MUSIC_TYPES);
}

////////////////////////////////////////////////////////////
/// Find font
////////////////////////////////////////////////////////////
std::string FileFinder::FindFont(const std::string& file) {
	return Find("font", file, FONTS_TYPES);
}

////////////////////////////////////////////////////////////
std::string FileFinder::DefaultFont() {
	// TODO
	return "DejaVuLGCSansMono";
}

#endif
