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

#ifdef _WIN32

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <iostream>
#include <fstream>
#include <windows.h>
#include <shlobj.h>
#include <io.h>
#include <vector>
#include "filefinder.h"
#include "options.h"
#include "registry_win.h"

// MinGW shlobj.h does not define this
#ifndef SHGFP_TYPE_CURRENT
#define SHGFP_TYPE_CURRENT 0
#endif

////////////////////////////////////////////////////////////
// Helper Variables
////////////////////////////////////////////////////////////
static std::vector<std::string> search_paths;
static std::string fonts_path;

////////////////////////////////////////////////////////////
// Helper Methods
////////////////////////////////////////////////////////////
static bool FileExists(std::string filename) {
	return _access(filename.c_str(), 4) == 0;
}
static std::string MakePath(std::string str, bool ending_slash = false) {
	for (std::size_t i = 0; i < str.length(); i++) {
		if (str[i] == '/')
			str[i] = '\\';
	}

	if (ending_slash && str[str.length() - 1] != '\\')
		str += '\\';

	return str;
}
static std::string FindFile(std::string name, const std::string exts[]) {
	name = MakePath(name);

	for (std::size_t i = 0; i < search_paths.size(); i++) {
		std::string path = MakePath(search_paths[i]) + name;
		const std::string* pexts = exts;
		while(const std::string* ext = pexts++) {
			if (ext->empty()) break;

			if (FileExists(name + *ext))
				return name + *ext;
		}
	}

	return "";
}
std::string GetFontsPath() {
	static std::string fonts_path = "";
	static bool init = false;

	if (init) {
		return fonts_path;
	} else {
		// Retrieve the Path of the Font Directory
		TCHAR path[MAX_PATH];

		if (SHGetFolderPath(NULL, CSIDL_FONTS, NULL, SHGFP_TYPE_CURRENT, path) == S_OK)	{
			char fpath[MAX_PATH];
#ifdef UNICODE
			WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS | WC_COMPOSITECHECK, path, MAX_PATH, fpath, MAX_PATH, NULL, NULL);
#endif
			fonts_path = MakePath(fpath, true);
		}

		init = true;

		return fonts_path;
	}
}
std::string GetFontFilename(std::string name) {
	std::string real_name = Registry::ReadStrValue(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", name + " (TrueType)");
	if (real_name.length() > 0) {
		if (FileExists(real_name))
			return real_name;
		if (FileExists(GetFontsPath() + real_name))
			return GetFontsPath() + real_name;
	}

	real_name = Registry::ReadStrValue(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Fonts", name + " (TrueType)");
	if (real_name.length() > 0) {
		if (FileExists(real_name))
			return real_name;
		if (FileExists(GetFontsPath() + real_name))
			return GetFontsPath() + real_name;
	}
	
	return name;
}

////////////////////////////////////////////////////////////
void FileFinder::Init() {
	std::string rtp_path;

	#if RPGMAKER == RPG2K
		rtp_path = Registry::ReadStrValue(HKEY_CURRENT_USER, "Software\\ASCII\\RPG2000", "RuntimePackagePath");
		if (rtp_path.empty())
			rtp_path = Registry::ReadStrValue(HKEY_LOCAL_MACHINE, "Software\\ASCII\\RPG2000", "RuntimePackagePath");
	#elif RPGMAKER == RPG2K3
		rtp_path = Registry::ReadStrValue(HKEY_CURRENT_USER, "Software\\Enterbrain\\RPG2003", "RuntimePackagePath");
		if (rtp_path.empty())
			rtp_path = Registry::ReadStrValue(HKEY_LOCAL_MACHINE, "Software\\Enterbrain\\RPG2003", "RuntimePackagePath");
	#else
		#error Set RPGMAKER to RPG2K or RPG2K3
	#endif

	if (!rtp_path.empty())
		search_paths.push_back(MakePath(rtp_path, true));
}

////////////////////////////////////////////////////////////
std::string FileFinder::FindImage(std::string name) {
	return FindFile(name, IMG_TYPES);
}

////////////////////////////////////////////////////////////
std::string FileFinder::FindMusic(std::string name) {
	return FindFile(name, MUSIC_TYPES);
}

////////////////////////////////////////////////////////////
std::string FileFinder::FindSound(std::string name) {
	return FindFile(name, SOUND_TYPES);
}

////////////////////////////////////////////////////////////
std::string FileFinder::FindFont(std::string name) {
	name = MakePath(name);

	std::string path = FindFile(name, FONTS_TYPES);

	if (!path.empty()) {
		return path;
	}

	std::string folder_path = "";
	std::string filename = name;

	size_t separator_pos = path.rfind('\\');
	if (separator_pos != std::string::npos) {
		folder_path = path.substr(0, separator_pos);
		filename = path.substr(separator_pos, path.length() - separator_pos);
	}

	std::string font_filename = GetFontFilename(filename);
	if (font_filename.length() > 0) {
		if (FileExists(folder_path + font_filename))
			return folder_path +  font_filename;

		if (FileExists(fonts_path + font_filename))
			return fonts_path + font_filename;
	}

	return "";
}

#endif
