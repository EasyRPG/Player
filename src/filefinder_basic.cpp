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

#if GEKKO || PSP 

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <cstdio>
#include <vector>
#include "filefinder.h"
#include "options.h"
#include "player.h"

////////////////////////////////////////////////////////////
// Helper Variables
////////////////////////////////////////////////////////////
static std::vector<std::string> search_paths;

////////////////////////////////////////////////////////////
// Helper Methods
////////////////////////////////////////////////////////////
static bool FileExists(std::string filename) {
	FILE* file = fopen(filename.c_str(), "r");
	if (file != NULL) {
		fclose(file);
		return true;
	} else {
		return false;
	}
}

static std::string MakePath(const std::string &dir, const std::string &name, bool ending_slash = false) {
	std::string str;
	if (dir.empty()) {
		str = name;
	} else {
		str = dir + "/" + name;
	}

	if (ending_slash && !str.empty() && str[str.length() - 1] != '/')
		str += '/';

	return str;
}
static std::string FindFile(const std::string &dir, const std::string &_name, const char* const exts[]) {
	std::string name = MakePath(dir, _name);

	for (std::size_t i = 0; i < search_paths.size(); i++) {
		std::string path = search_paths[i] + name;
		const char*const* pexts = exts;
		while (const char*const* ext = pexts++) {
			if (!*ext) break;

			if (FileExists(path + *ext))
				return path + *ext;
		}
	}

	return "";
}

////////////////////////////////////////////////////////////
void FileFinder::Init() {
	search_paths.clear();
	search_paths.push_back("");
}

void FileFinder::InitRtpPaths() {
	if (Player::engine == Player::EngineRpg2k) {
#ifdef GEKKO
		search_paths.push_back(MakePath("sd:/data/rtp/2000/", ""));
		search_paths.push_back(MakePath("usb:/data/rtp/2000/", ""));
#else
		search_paths.push_back(MakePath("/data/rtp/2000/", ""));
#endif
	} else if (Player::engine == Player::EngineRpg2k3) {
#ifdef GEKKO
		search_paths.push_back(MakePath("sd:/data/rtp/2003/", ""));
		search_paths.push_back(MakePath("usb:/data/rtp/2003/", ""));
#else
		search_paths.push_back(MakePath("/data/rtp/2003/", ""));
#endif
	}
}


void FileFinder::Quit() {

}

////////////////////////////////////////////////////////////
std::string FileFinder::FindImage(const std::string& dir, const std::string& name) {
	return FindFile(dir, name, IMG_TYPES);
}

std::string FileFinder::FindDefault(const std::string& dir, const std::string& name) {
	static const char* const no_exts[] = {""};
	return FindFile(dir, name, no_exts);
}

////////////////////////////////////////////////////////////
std::string FileFinder::FindMusic(const std::string& name) {
	return FindFile("Music", name, MUSIC_TYPES);
}

////////////////////////////////////////////////////////////
std::string FileFinder::FindSound(const std::string& name) {
	return FindFile("Sound", name, SOUND_TYPES);
}

////////////////////////////////////////////////////////////
std::string FileFinder::FindFont(const std::string& name) {
	return FindFile("Font", name, FONTS_TYPES);
}

////////////////////////////////////////////////////////////
std::string FileFinder::DefaultFont() {
	return "DejaVuLGCSansMono";
}

FILE* FileFinder::fopenUTF8(const std::string& name_utf8, const std::string& mode) {
	return fopen(name_utf8.c_str(), mode.c_str());
}

#endif
