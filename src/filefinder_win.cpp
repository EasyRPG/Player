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

#ifdef WIN32

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <iostream>
#include <fstream>
#include <windows.h>
#include <shlobj.h>
#include <io.h>
#include "filefinder.h"
#include "options.h"
#include "registry_win.h"

// MinGW shlobj.h does not define this
#ifndef SHGFP_TYPE_CURRENT
#define SHGFP_TYPE_CURRENT 0
#endif

////////////////////////////////////////////////////////////
/// Global Variables
////////////////////////////////////////////////////////////
namespace FileFinder {
	std::string fonts_path;
	std::string rtp_path;
}

////////////////////////////////////////////////////////////
/// Initialize
////////////////////////////////////////////////////////////
void FileFinder::Init() {
	#if RPGMAKER == RPG2K
		rtp_path = Registry::ReadStrValue(HKEY_CURRENT_USER, "Software\\ASCII\\RPG2000", "RuntimePackagePath");
		if (rtp_path == "") {
			rtp_path = Registry::ReadStrValue(HKEY_LOCAL_MACHINE, "Software\\ASCII\\RPG2000", "RuntimePackagePath");
		}
	#elif RPGMAKER == RPG2K3
		rtp_path = Registry::ReadStrValue(HKEY_CURRENT_USER, "Software\\Enterbrain\\RPG2003", "RuntimePackagePath");
		if (rtp_path == "") {
			rtp_path = Registry::ReadStrValue(HKEY_LOCAL_MACHINE, "Software\\Enterbrain\\RPG2003", "RuntimePackagePath");
		}
	#else
		#error Set RPGMAKER to RPG2K or RPG2K3
	#endif

	// Retrieve the Path of the Font Directory
	TCHAR path[MAX_PATH];

	if (SHGetFolderPath(NULL, CSIDL_FONTS, NULL, SHGFP_TYPE_CURRENT, path) != S_OK)	{
		fonts_path = "";
	} else {
		char fpath[MAX_PATH];
		#ifdef UNICODE
		WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS | WC_COMPOSITECHECK, path, MAX_PATH, fpath, MAX_PATH, NULL, NULL);
		#endif

		fonts_path = std::string(fpath);
		fonts_path += "\\";
	}
}

////////////////////////////////////////////////////////////
/// Check if file exists
////////////////////////////////////////////////////////////
static bool fexists(std::string filename) {
	return (_access(filename.c_str(), 4) == 0);
}

////////////////////////////////////////////////////////////
/// Make path
////////////////////////////////////////////////////////////
static std::string slasher(std::string str) {
	for (unsigned int i = 0; i < str.length(); i++) {
		if (str[i] == '/') {
			str[i] = '\\';
		}
	}
	return str;
}

////////////////////////////////////////////////////////////
/// Find image
////////////////////////////////////////////////////////////
std::string FileFinder::FindImage(std::string name) {
	name = slasher(name);
	std::string path = name;
	if (fexists(path)) return path;
	path = name; path += ".bmp";
	if (fexists(path)) return path;
	path = name; path += ".gif";
	if (fexists(path)) return path;
	path = name; path += ".png";
	if (fexists(path)) return path;
	path = name; path += ".xyz";
	if (fexists(path)) return path;
	if (rtp_path != "") {
		std::string srtp_path = slasher(rtp_path);
		srtp_path += name;
		path = srtp_path;
		if (fexists(path)) return path;
		path = srtp_path; path += ".bmp";
		if (fexists(path)) return path;
		path = srtp_path; path += ".gif";
		if (fexists(path)) return path;
		path = srtp_path; path += ".png";
		if (fexists(path)) return path;
		path = srtp_path; path += ".xyz";
		if (fexists(path)) return path;
	}
	return "";
}

////////////////////////////////////////////////////////////
/// Find music
////////////////////////////////////////////////////////////
std::string FileFinder::FindMusic(std::string name) {
	name = slasher(name);
	std::string path = name;
	if (fexists(path)) return path;
	path = name; path += ".wav";
	if (fexists(path)) return path;
	path = name; path += ".mid";
	if (fexists(path)) return path;
	path = name; path += ".midi";
	if (fexists(path)) return path;
	path = name; path += ".ogg";
	if (fexists(path)) return path;
	path = name; path += ".mp3";
	if (fexists(path)) return path;
	if (rtp_path != "") {
		std::string srtp_path = slasher(rtp_path);
		srtp_path += name;
		path = srtp_path;
		if (fexists(path)) return path;
		path = srtp_path; path += ".wav";
		if (fexists(path)) return path;
		path = srtp_path; path += ".mid";
		if (fexists(path)) return path;
		path = srtp_path; path += ".midi";
		if (fexists(path)) return path;
		path = srtp_path; path += ".ogg";
		if (fexists(path)) return path;
		path = srtp_path; path += ".mp3";
		if (fexists(path)) return path;
	}
	return "";
}

////////////////////////////////////////////////////////////
/// Find font
////////////////////////////////////////////////////////////
std::string FileFinder::FindFont(std::string name) {
	name = slasher(name);
	std::string path = name;
	if (fexists(path)) return path;
	path = name; path += ".ttf";
	if (fexists(path)) return path;

	std::string real_name;
	real_name = Registry::ReadStrValue(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", name + " (TrueType)");
	if (real_name.length() > 0) {
		path = real_name;
		if (fexists(path)) return path;
		path = fonts_path; path += real_name;
		if (fexists(path)) return path;
	}

	real_name = Registry::ReadStrValue(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Fonts", name + " (TrueType)");
	if (real_name.length() > 0) {
		path = real_name;
		if (fexists(path)) return path;
		path = fonts_path; path += real_name;
		if (fexists(path)) return path;
	}

	return "";
}

#endif
