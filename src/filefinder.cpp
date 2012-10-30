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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/optional.hpp>

#include "system.h"
#include "options.h"
#include "utils.h"
#include "filefinder.h"
#include "output.h"
#include "player.h"
#include "main_data.h"

#ifdef _WIN32
#  include "dirent_win.h"
#  include <windows.h>
#  include <shlobj.h>
#  include "registry_win.h"
#else
#  include <dirent.h>
#  include <unistd.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#endif

// MinGW shlobj.h does not define this
#ifndef SHGFP_TYPE_CURRENT
#define SHGFP_TYPE_CURRENT 0
#endif

namespace {
	const char* const MOVIE_TYPES[] = {
		".avi", ".mpg" };

	typedef boost::ptr_vector<FileFinder::ProjectTree> search_path_list;
	search_path_list search_paths;
	std::string fonts_path;

	boost::optional<std::string> FindFile(FileFinder::ProjectTree const& tree,
										  std::string const& dir,
										  std::string const& name,
										  char const* exts[])
	{
		using namespace FileFinder;

		std::string const lower_dir = Utils::LowerCase(dir);
		string_map::const_iterator dir_it = tree.directories.find(lower_dir);
		if(dir_it == tree.directories.end()) { return boost::none; }

		string_map const& dir_map = tree.sub_members.find(lower_dir)->second;

		for(char const** c = exts; *c != NULL; ++c) {
			std::string const lower_name = Utils::LowerCase(name + *c);
			string_map::const_iterator const name_it = dir_map.find(lower_name);
			if(name_it != dir_map.end()) {
				return MakePath(std::string(tree.project_path).append("/")
								.append(dir_it->second).append("/"),
								name_it->second);
			}
		}

		return boost::none;
	}

	std::string FindFile(const std::string &dir, const std::string& name, const char* exts[]) {
		for(search_path_list::const_iterator i = search_paths.begin(); i != search_paths.end(); ++i) {
			boost::optional<std::string> const ret = FindFile(*i, dir, name, exts);
			if(ret != boost::none) { return *ret; }
		}

		FileFinder::ProjectTree const& tree = FileFinder::GetProjectTree();
		boost::optional<std::string> const ret = FindFile(tree, dir, name, exts);

		return(ret != boost::none? *ret : "");
	}

} // anonymous namespace

std::auto_ptr<FileFinder::ProjectTree> FileFinder::CreateProjectTree(std::string const& p) {
	std::auto_ptr<ProjectTree> tree(new ProjectTree());
	tree->project_path = p;

	Directory mem = GetDirectoryMembers(tree->project_path, ALL);
	for(string_map::const_iterator i = mem.members.begin(); i != mem.members.end(); ++i) {
		(IsDirectory(MakePath(tree->project_path, i->second))?
		 tree->directories : tree->files)[i->first] = i->second;
	}

	for(string_map::const_iterator i = tree->directories.begin(); i != tree->directories.end(); ++i) {
		GetDirectoryMembers(MakePath(tree->project_path, i->second), FILES)
			.members.swap(tree->sub_members[i->first]);
	}

	return tree;
}

std::string FileFinder::MakePath(const std::string &dir, std::string const& name) {
	std::string str = dir.empty()? name : dir + "/" + name;
#ifdef _WIN32
	std::replace(str.begin(), str.end(), '/', '\\');
#else
	std::replace(str.begin(), str.end(), '\\', '/');
#endif
	return str;
}

#ifdef _WIN32
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
			fonts_path = FileFinder::MakePath(fpath, "");
		}

		init = true;

		return fonts_path;
	}
}

std::string GetFontFilename(std::string const& name) {
	std::string real_name = Registry::ReadStrValue(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", name + " (TrueType)");
	if (real_name.length() > 0) {
		if (FileFinder::Exists(real_name))
			return real_name;
		if (FileFinder::Exists(GetFontsPath() + real_name))
			return GetFontsPath() + real_name;
	}

	real_name = Registry::ReadStrValue(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Fonts", name + " (TrueType)");
	if (real_name.length() > 0) {
		if (FileFinder::Exists(real_name))
			return real_name;
		if (FileFinder::Exists(GetFontsPath() + real_name))
			return GetFontsPath() + real_name;
	}

	return name;
}
#endif

////////////////////////////////////////////////////////////
std::string FileFinder::FindFont(const std::string& name) {
	static const char* FONTS_TYPES[] = {
		".ttf", ".ttc", ".otf", ".fon", NULL, };
	std::string path = FindFile("Font", name, FONTS_TYPES);

#ifdef _WIN32
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
		if (FileFinder::Exists(folder_path + font_filename))
			return folder_path + font_filename;

		if (FileFinder::Exists(fonts_path + font_filename))
			return fonts_path + font_filename;
	}

	return "";
#else
	return path;
#endif
}

////////////////////////////////////////////////////////////
std::string FileFinder::DefaultFont() {
#ifdef _WIN32
	static std::string default_font = "";
	static bool init = false;

	if (!init) {
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
#else
	return "DejaVuLGCSansMono";
#endif
}

FileFinder::ProjectTree const& FileFinder::GetProjectTree() {
	static ProjectTree tree_;

	if(tree_.project_path != Main_Data::project_path) {
		tree_ = *CreateProjectTree(Main_Data::project_path);
	}

	return tree_;
}

void FileFinder::Init() {
	GetProjectTree(); // empty call
}

void FileFinder::InitRtpPaths() {
	std::string const version_str =
		Player::engine == Player::EngineRpg2k? "2000":
		Player::engine == Player::EngineRpg2k3? "2003":
		"";

	assert(!version_str.empty());

#ifdef _WIN32
	std::string rtp_path = Registry::ReadStrValue(HKEY_CURRENT_USER, "Software\\ASCII\\RPG" + version_str, "RuntimePackagePath");
	if(! rtp_path.empty())
		search_paths.push_back(CreateProjectTree(rtp_path));

	rtp_path = Registry::ReadStrValue(HKEY_LOCAL_MACHINE, "Software\\ASCII\\RPG" + version_str, "RuntimePackagePath");
	if(! rtp_path.empty())
		search_paths.push_back(CreateProjectTree(rtp_path));
#elif defined(GEKKO)
	search_paths.push_back(CreateProjectTree("sd:/data/rtp/" + version_str + "/"));
	search_paths.push_back(CreateProjectTree("usb:/data/rtp/" + version_str + "/"));
#else
	search_paths.push_back(CreateProjectTree("/data/rtp/" + version_str + "/"));
#endif

	if (Player::engine == Player::EngineRpg2k && getenv("RPG2K_RTP_PATH"))
		search_paths.push_back(CreateProjectTree(getenv("RPG2K_RTP_PATH")));
	else if (Player::engine == Player::EngineRpg2k3 && getenv("RPG2K3_RTP_PATH"))
		search_paths.push_back(CreateProjectTree(getenv("RPG2K3_RTP_PATH")));
	if(getenv("RPG_RTP_PATH"))
		search_paths.push_back(CreateProjectTree(getenv("RPG_RTP_PATH")));
}


void FileFinder::Quit() {
	search_paths.clear();
}

FILE* FileFinder::fopenUTF8(const std::string& name_utf8, char const* mode) {
#ifdef _WIN32
	return _wfopen(Utils::ToWideString(name_utf8).c_str(),
				   Utils::ToWideString(mode).c_str());
#else
	return fopen(name_utf8.c_str(), mode);
#endif
}

std::string FileFinder::FindImage(const std::string& dir, const std::string& name) {
	static const char* IMG_TYPES[] = {
		".bmp",  ".png", ".xyz", ".gif", ".jpg", ".jpeg", NULL };
	return FindFile(dir, name, IMG_TYPES);
}

std::string FileFinder::FindDefault(const std::string& dir, const std::string& name) {
	static const char* no_exts[] = {"", NULL};
	return FindFile(dir, name, no_exts);
}

std::string FileFinder::FindDefault(std::string const& name) {
	ProjectTree const& p = GetProjectTree();
	string_map const& files = p.files;

	string_map::const_iterator const it = files.find(Utils::LowerCase(name));

	return(it != files.end())? MakePath(p.project_path, it->second) : "";
}

bool FileFinder::IsRPG2kProject(ProjectTree const& dir) {
	string_map::const_iterator const
		ldb_it = dir.files.find(Utils::LowerCase(DATABASE_NAME)),
		lmt_it = dir.files.find(Utils::LowerCase(TREEMAP_NAME));

	return(ldb_it != dir.files.end() && lmt_it != dir.files.end());
}

std::string FileFinder::FindMusic(const std::string& name) {
	static const char* MUSIC_TYPES[] = {
		".wav", ".mid", ".midi", ".ogg", ".mp3", NULL };
	return FindFile("Music", name, MUSIC_TYPES);
}

std::string FileFinder::FindSound(const std::string& name) {
	static const char* SOUND_TYPES[] = {
		".wav", ".ogg", ".mp3", NULL };
	return FindFile("Sound", name, SOUND_TYPES);
}

bool FileFinder::Exists(std::string const& filename) {
#ifdef _WIN32
	return ::GetFileAttributesW(Utils::ToWideString(filename).c_str()) != (DWORD)-1;
#else
	return ::access(filename.c_str(), F_OK) != -1;
#endif
}

bool FileFinder::IsDirectory(std::string const& dir) {
	assert(Exists(dir));
#ifdef _WIN32
	return(::GetFileAttributesW(Utils::ToWideString(dir).c_str()) & FILE_ATTRIBUTE_DIRECTORY);
#else
	struct stat sb;
	BOOST_VERIFY(::stat(dir.c_str(), &sb) != -1);
	return S_ISDIR(sb.st_mode);
#endif
}

FileFinder::Directory FileFinder::GetDirectoryMembers(const std::string& path, FileFinder::Mode const m) {
	assert(FileFinder::Exists(path));
	assert(FileFinder::IsDirectory(path));

	Directory result;

	result.base = path;

#ifdef _WIN32
#  define DIR _WDIR
#  define opendir _wopendir
#  define closedir _wclosedir
#  define wpath Utils::ToWideString(path)
#  define dirent _wdirent
#  define readdir _wreaddir
#else
#  define wpath path
#endif

	EASYRPG_SHARED_PTR< ::DIR> dir(::opendir(wpath.c_str()), ::closedir);
	if (!dir) {
		Output::Error("Error opening dir %s: %s", path.c_str(),
					  ::strerror(errno));
		return result;
	}

	struct dirent* ent;
	while ((ent = ::readdir(dir.get())) != NULL) {
		if (ent->d_name[0] == '.') { continue; }
#ifdef _WIN32
		std::string const name = Utils::FromWideString(ent->d_name);
#else
		std::string const name = ent->d_name;
#endif
		switch(m) {
		case FILES:
			if(IsDirectory(MakePath(path, name))) { continue; }
			else{ break; }
		case DIRECTORIES:
			if(! IsDirectory(MakePath(path, name))) { continue; }
			else{ break; }
		case ALL:
			break;
		}
		result.members[Utils::LowerCase(name)] = name;
	}

#ifdef _WIN32
#  undef DIR
#  undef opendir
#  undef closedir
#  undef dirent
#  undef readdir
#endif
#undef wpath

	return result;
}
