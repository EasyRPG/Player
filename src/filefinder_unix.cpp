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

#if UNIX || DINGOO || GEKKO

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#ifdef GEKKO
	#include <fat.h>
	#include <cstdlib>
	#include <cstdio>
#endif

#include <errno.h>
#include "filefinder.h"
#include "output.h"

////////////////////////////////////////////////////////////
/// Global Variables
////////////////////////////////////////////////////////////
namespace FileFinder {
	std::string fonts_path;
	std::string rtp_path;
	std::string rtp_paths[3];
}

////////////////////////////////////////////////////////////
/// Initialize
////////////////////////////////////////////////////////////
void FileFinder::Init() {
	// TODO find default paths
	rtp_path = "";
	fonts_path = "";

#ifdef GEKKO
	// Init libfat (Mount SD/USB)
	if (!fatInitDefault()) {
		// No display is available yet and no Volume mounted
		exit(1);
	}
#endif
}

////////////////////////////////////////////////////////////
/// Check if file exists
////////////////////////////////////////////////////////////
static bool fexists(std::string& filename) {
	// Check if file exists and is readable
	// At first the case sensitive version
#ifdef GEKKO
	// libfat is case insensitive
	// No idea why but access is not available
	FILE* file = fopen(filename.c_str(), "r");
	if (file == NULL) {
		return false;
	} else {
		return true;
	}
#else
	if (access(filename.c_str(), R_OK) == 0) {
		return true;
	}

	// File not found, doing case insensitive search
	// Extract the directory out of the filename
	size_t dirpos = filename.rfind('/');
	std::string dirname;
	std::string file;
	if (dirpos == std::string::npos) { // No / found, assume . as dir
		dirname = std::string(".");
		file = filename;
	} else {
		dirname = filename.substr(0, dirpos+1);
		file = filename.substr(dirpos+1);
	}
	//printf("dir: %s", dirname.c_str());
	//printf("\nfile: %s\n", file.c_str());

	DIR* dir = opendir(dirname.c_str());
	if (dir == NULL) {
		Output::Error("Error opening dir %s: %s", dirname.c_str(),
						strerror(errno));
	}

	struct dirent* dirent;

	while ((dirent = readdir(dir)) != NULL)	{
		if (strcasecmp(file.c_str(), dirent->d_name) == 0) {
			// File found, now check if its readable
			if (dirname == ".") {
				filename = dirent->d_name;
			} else {
				filename = dirname + dirent->d_name;
			}
			//printf("Returning %s\n", filename.c_str());
			closedir(dir);
			return (access(filename.c_str(), R_OK) == 0);
		}
	}

	// No luck :(	
	closedir(dir);
	return false;
#endif
}

////////////////////////////////////////////////////////////
/// Make path
////////////////////////////////////////////////////////////
std::string slasher(std::string str) {
	for(unsigned int i = 0; i < str.length(); i++) {
		if (str[i] == '\\') {
			str[i] = '/';
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
	for (int i = 0; i < 3; i++) {
		if (rtp_paths[i] != "") {
			std::string rtp_path = slasher(rtp_paths[i]);
			rtp_path += name;
			path = rtp_path;
			if (fexists(path)) return path;
			path = rtp_path; path += ".bmp";
			if (fexists(path)) return path;
			path = rtp_path; path += ".gif";
			if (fexists(path)) return path;
			path = rtp_path; path += ".png";
			if (fexists(path)) return path;
			path = rtp_path; path += ".xyz";
			if (fexists(path)) return path;
		}
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
	for (int i = 0; i < 3; i++) {
		if (rtp_paths[i] != "") {
			std::string rtp_path = slasher(rtp_paths[i]);
			rtp_path += name;
			path = rtp_path;
			if (fexists(path)) return path;
			path = rtp_path; path += ".wav";
			if (fexists(path)) return path;
			path = rtp_path; path += ".mid";
			if (fexists(path)) return path;
			path = rtp_path; path += ".midi";
			if (fexists(path)) return path;
			path = rtp_path; path += ".ogg";
			if (fexists(path)) return path;
			path = rtp_path; path += ".mp3";
			if (fexists(path)) return path;
		}
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

	path = fonts_path; path += name;
	if (fexists(path)) return path;
	path = fonts_path; path += name; path += ".ttf";
	if (fexists(path)) return path;

	return "";
}

////////////////////////////////////////////////////////////
std::string FileFinder::DefaultFont() {
	// TODO
	return "Font/DejaVuLGCSansMono";
}

#endif
