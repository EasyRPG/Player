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
#include <cstdio>
#include <iostream>
#include <fstream>
#include <windows.h>
#include <shlobj.h>
#include <vector>
#include "filefinder.h"
#include "options.h"
#include "player.h"
#include "registry_win.h"
#include "utils.h"

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
	std::wstring file = Utils::DecodeUTF(filename);
	return GetFileAttributes(file.c_str()) != (DWORD)-1;
}

static std::string MakePath(const std::string &dir, const std::string &name, bool ending_slash = false) {
	std::string str;
	if (dir.empty()) {
		str = name;
	} else {
		str = dir + "\\" + name;
	}

	for (std::size_t i = 0; i < str.length(); i++) {
		if (str[i] == '/')
			str[i] = '\\';
	}

	if (ending_slash && !str.empty() && str[str.length() - 1] != '\\')
		str += '\\';

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
			fonts_path = MakePath(fpath, "");
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
	search_paths.clear();
	search_paths.push_back("");
}

void FileFinder::InitRtpPaths() {
	std::string rtp_path;

	if (Player::engine == Player::EngineRpg2k) {
		rtp_path = Registry::ReadStrValue(HKEY_CURRENT_USER, "Software\\ASCII\\RPG2000", "RuntimePackagePath");
		if (rtp_path.empty())
			rtp_path = Registry::ReadStrValue(HKEY_LOCAL_MACHINE, "Software\\ASCII\\RPG2000", "RuntimePackagePath");
	} else if (Player::engine == Player::EngineRpg2k3) {
		rtp_path = Registry::ReadStrValue(HKEY_CURRENT_USER, "Software\\Enterbrain\\RPG2003", "RuntimePackagePath");
		if (rtp_path.empty())
			rtp_path = Registry::ReadStrValue(HKEY_LOCAL_MACHINE, "Software\\Enterbrain\\RPG2003", "RuntimePackagePath");
	}

	if (!rtp_path.empty())
		search_paths.push_back(MakePath(rtp_path, ""));
}

void FileFinder::Quit() {

}

////////////////////////////////////////////////////////////
std::string FileFinder::FindImage(const std::string& dir, const std::string& name) {
	return FindFile(dir, name, IMG_TYPES);
}

std::string FileFinder::FindDefault(const std::string& dir, const std::string& name) {
	static const char* const no_exts[] = {"", NULL};
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
	std::string path = FindFile("Font", name, FONTS_TYPES);

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
	if (!font_filename.empty()) {
		if (FileExists(folder_path + font_filename))
			return folder_path + font_filename;

		if (FileExists(fonts_path + font_filename))
			return fonts_path + font_filename;
	}

	return "";
}

////////////////////////////////////////////////////////////
std::string FileFinder::DefaultFont() {
	static std::string default_font = "";
	static bool init = false;

	if (!init) {
		search_paths.push_back("");
		std::string fonts[] = DEFAULT_FONTS;

		const std::string* pfont = fonts;
		while(const std::string* font = pfont++) {
			if (font->empty()) break;

			default_font = FindFont(*font);

			if (!default_font.empty()) break;
		}

		init = true;
	}
	
	return default_font;
}

FILE* FileFinder::fopenUTF8(const std::string& name_utf8, const std::string& mode) {
	std::wstring name_w = Utils::DecodeUTF(name_utf8);
	std::wstring mode_w = Utils::DecodeUTF(mode);
	return _wfopen(name_w.c_str(), mode_w.c_str());
}

#endif
