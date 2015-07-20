/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

// Headers
#include <cerrno>
#include <cstdio>
#include <cstdlib>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <boost/optional.hpp>

#include "system.h"
#include "options.h"
#include "utils.h"
#include "filefinder.h"
#include "output.h"
#include "player.h"
#include "main_data.h"
#include "reader_util.h"
#include "registry.h"

#ifdef _MSC_VER
#  include "rtp_table_bom.h"
#else
#  include "rtp_table.h"
#endif

#ifdef __MINGW32__
#  include <dirent.h>
#elif defined(_MSC_VER)
#  include "dirent_win.h"
#endif

#ifdef _WIN32
#  include <windows.h>
#  include <shlobj.h>
#else
#  include <dirent.h>
#  include <unistd.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#endif

#ifdef __ANDROID__
#   include <jni.h>
#   include <SDL_system.h>
#endif

#ifdef EMSCRIPTEN
#  include <sstream>
#  include <iterator>
#  include <iomanip>
#endif

// MinGW shlobj.h does not define this
#ifndef SHGFP_TYPE_CURRENT
#define SHGFP_TYPE_CURRENT 0
#endif

namespace {
	const char* const MOVIE_TYPES[] = { ".avi", ".mpg" };

	typedef std::vector<EASYRPG_SHARED_PTR<FileFinder::ProjectTree> > search_path_list;
	search_path_list search_paths;
	std::string fonts_path;

	boost::optional<std::string> FindFile(FileFinder::ProjectTree const& tree,
										  std::string const& dir,
										  std::string const& name,
										  char const* exts[])
	{
		using namespace FileFinder;

		std::string const lower_dir = Utils::LowerCase(dir);
		std::string const escape_symbol = Player::escape_symbol;
		std::string corrected_name = Utils::LowerCase(name);
		if (escape_symbol != "\\") {
			std::size_t escape_pos = corrected_name.find(escape_symbol);
			while (escape_pos != std::string::npos) {
				corrected_name.erase(escape_pos, escape_symbol.length());
				corrected_name.insert(escape_pos, "/");
				escape_pos = corrected_name.find(escape_symbol);
			}
		}

		string_map::const_iterator dir_it = tree.directories.find(lower_dir);
		if(dir_it == tree.directories.end()) { return boost::none; }

		string_map const& dir_map = tree.sub_members.find(lower_dir)->second;

		for(char const** c = exts; *c != NULL; ++c) {
			string_map::const_iterator const name_it = dir_map.find(corrected_name + *c);
			if(name_it != dir_map.end()) {
				return MakePath
					(std::string(tree.project_path).append("/")
					 .append(dir_it->second), name_it->second);
			}
		}

		return boost::none;
	}

	bool is_not_ascii_char(uint8_t c) { return c > 0x80; }

	bool is_not_ascii_filename(std::string const& n) {
		return std::find_if(n.begin(), n.end(), &is_not_ascii_char) != n.end();
	}

	std::string const& translate_rtp(std::string const& dir, std::string const& name) {
		rtp_table_type const& table =
			Player::IsRPG2k() ? RTP_TABLE_2000 : RTP_TABLE_2003;

		rtp_table_type::const_iterator dir_it = table.find(Utils::LowerCase(dir));
		std::string lower_name = Utils::LowerCase(name);

		if (dir_it == table.end()) { return name; }

		std::map<std::string, std::string>::const_iterator file_it =
			dir_it->second.find(lower_name);
		if (file_it == dir_it->second.end()) {
			if (is_not_ascii_filename(lower_name)) {
				// Linear Search: Japanese file name to English file name
				for (std::map<std::string, std::string>::const_iterator it = dir_it->second.begin(); it != file_it; ++it) {
					if (it->second == lower_name) {
						return it->first;
					}
				}
			}
			return name;
		}
		return file_it->second;
	}

	std::string FindFile(const std::string &dir, const std::string& name, const char* exts[]) {
		FileFinder::ProjectTree const& tree = FileFinder::GetProjectTree();
		boost::optional<std::string> const ret = FindFile(tree, dir, name, exts);
		if (ret != boost::none) { return *ret; }

		std::string const& rtp_name = translate_rtp(dir, name);
		Output::Debug("RTP name %s(%s)", rtp_name.c_str(), name.c_str());

		for(search_path_list::const_iterator i = search_paths.begin(); i != search_paths.end(); ++i) {
			if (! *i) { continue; }

			boost::optional<std::string> const ret = FindFile(*(*i), dir, name, exts);
			if (ret) { return *ret; }

			boost::optional<std::string> const ret_rtp = FindFile(*(*i), dir, rtp_name, exts);
			if (ret_rtp) { return *ret_rtp; }
		}

		Output::Debug("Cannot find: %s/%s", dir.c_str(), name.c_str());

		return std::string();
	}

} // anonymous namespace

EASYRPG_SHARED_PTR<FileFinder::ProjectTree> FileFinder::CreateProjectTree(std::string const& p, bool recursive) {
	if(! (Exists(p) && IsDirectory(p))) { return EASYRPG_SHARED_PTR<ProjectTree>(); }

	EASYRPG_SHARED_PTR<ProjectTree> tree = EASYRPG_MAKE_SHARED<ProjectTree>();
	tree->project_path = p;

	Directory mem = GetDirectoryMembers(tree->project_path, recursive ? ALL : FILES);
	for(string_map::const_iterator i = mem.members.begin(); i != mem.members.end(); ++i) {
		(IsDirectory(MakePath(tree->project_path, i->second))?
		 tree->directories : tree->files)[i->first] = i->second;
	}

	// Stop here if the tree is invalid
	if (p == Main_Data::project_path && !IsRPG2kProject(*tree) && !IsEasyRpgProject(*tree))
		return tree;

	for(string_map::const_iterator i = tree->directories.begin(); i != tree->directories.end(); ++i) {
		GetDirectoryMembers(MakePath(tree->project_path, i->second), RECURSIVE)
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

FileFinder::ProjectTree const& FileFinder::GetProjectTree(bool init) {
	static ProjectTree tree_;

	if(tree_.project_path != Main_Data::project_path || init) {
		EASYRPG_SHARED_PTR<ProjectTree> t = CreateProjectTree(Main_Data::project_path);
		if(! t) {
			Output::Error("invalid project path: %s", Main_Data::project_path.c_str());
			return tree_;
		}
		tree_ = *t;
	}

	return tree_;
}

void FileFinder::Init() {
	GetProjectTree(true); // empty call
}

static void add_rtp_path(std::string const& p) {
	using namespace FileFinder;
	EASYRPG_SHARED_PTR<ProjectTree> tree(CreateProjectTree(p));
	if(tree) {
		Output::Debug("Adding %s to RTP path", p.c_str());
		search_paths.push_back(tree);
	}
}

static void read_rtp_registry(const std::string& company, const std::string& version_str, const std::string& key) {
	std::string rtp_path = Registry::ReadStrValue(HKEY_CURRENT_USER, "Software\\" + company + "\\RPG" + version_str, key);
	if (!rtp_path.empty()) {
		add_rtp_path(rtp_path);
	}

	rtp_path = Registry::ReadStrValue(HKEY_LOCAL_MACHINE, "Software\\" + company + "\\RPG" + version_str, key);
	if (!rtp_path.empty()) {
		add_rtp_path(rtp_path);
	}
}

void FileFinder::InitRtpPaths() {
#ifdef EMSCRIPTEN
	// No RTP support for emscripten at the moment.
	return;
#endif

	std::string const version_str =
		Player::IsRPG2k() ? "2000" :
		Player::IsRPG2k3() ? "2003" :
		"";

	assert(!version_str.empty());

	if (Player::IsRPG2k()) {
		read_rtp_registry("ASCII", version_str, "RuntimePackagePath");
	}
	else if (Player::IsRPG2k3Legacy()) {
		// Original 2003 RTP installer registry key is upper case
		// and Wine registry is case insensitive but new 2k3v1.10 installer is not
		// Prefer Enterbrain RTP over Kadokawa for old RPG2k3 (search order)
		read_rtp_registry("Enterbrain", version_str, "RUNTIMEPACKAGEPATH");
		read_rtp_registry("KADOKAWA", version_str, "RuntimePackagePath");
	}
	else if (Player::IsRPG2k3E()) {
		// Prefer Kadokawa RTP over Enterbrain for new RPG2k3
		read_rtp_registry("KADOKAWA", version_str, "RuntimePackagePath");
		read_rtp_registry("Enterbrain", version_str, "RUNTIMEPACKAGEPATH");
	}

#ifdef GEKKO
	add_rtp_path("sd:/data/rtp/" + version_str + "/");
	add_rtp_path("usb:/data/rtp/" + version_str + "/");
#elif __ANDROID__
	// Invoke "String getRtpPath()" in EasyRPG Activity via JNI
	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject sdl_activity = (jobject)SDL_AndroidGetActivity();
	jclass cls = env->GetObjectClass(sdl_activity);
	jmethodID jni_getRtpPath = env->GetMethodID(cls , "getRtpPath", "()Ljava/lang/String;");
	jstring return_string = (jstring)env->CallObjectMethod(sdl_activity, jni_getRtpPath);
	
	const char *js = env->GetStringUTFChars(return_string, NULL);
	std::string cs(js);

	env->ReleaseStringUTFChars(return_string, js);
	env->DeleteLocalRef(sdl_activity);
	env->DeleteLocalRef(cls);

	add_rtp_path(cs + "/" + version_str + "/");
#else
	add_rtp_path("/data/rtp/" + version_str + "/");
#endif

	if (Player::IsRPG2k() && getenv("RPG2K_RTP_PATH"))
		add_rtp_path(getenv("RPG2K_RTP_PATH"));
	else if (Player::IsRPG2k3() && getenv("RPG2K3_RTP_PATH"))
		add_rtp_path(getenv("RPG2K3_RTP_PATH"));

	if (getenv("RPG_RTP_PATH")) {
		add_rtp_path(getenv("RPG_RTP_PATH"));
	}
	if (search_paths.empty()) {
		Output::Warning("RTP not found. This may create missing file errors.\n"
			"Install RTP files or check they are installed fine.\n"
			"If this game really does not require RTP, then add\n"
			"FullPackageFlag=1 line to the RPG_RT.ini game file.");
	}
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

EASYRPG_SHARED_PTR<std::fstream> FileFinder::openUTF8(const std::string& name,
													  std::ios_base::openmode m)
{
	EASYRPG_SHARED_PTR<std::fstream> ret(new std::fstream(
#ifdef _MSC_VER
		Utils::ToWideString(name).c_str(),
#else
		name.c_str(),
#endif
		m));
	return (*ret)? ret : EASYRPG_SHARED_PTR<std::fstream>();
}

std::string FileFinder::FindImage(const std::string& dir, const std::string& name) {
#ifdef EMSCRIPTEN
	return FindDefault(dir, name);
#endif

	static const char* IMG_TYPES[] = {
		".bmp",  ".png", ".xyz", /*".gif", ".jpg", ".jpeg",*/ NULL };
	return FindFile(dir, name, IMG_TYPES);
}

std::string FileFinder::FindDefault(const std::string& dir, const std::string& name) {
	static const char* no_exts[] = {"", NULL};
	return FindFile(dir, name, no_exts);
}

std::string FileFinder::FindDefault(std::string const& name) {
	return FindDefault(GetProjectTree(), name);
}

std::string FileFinder::FindDefault(const ProjectTree& tree, const std::string& dir, const std::string& name) {
	static const char* no_exts[] = { "", NULL };

	boost::optional<std::string> file = FindFile(tree, dir, name, no_exts);
	if (file != boost::none) {
		return *file;
	}
	return "";
}

std::string FileFinder::FindDefault(const ProjectTree& tree, const std::string& name) {
	ProjectTree const& p = tree;
	string_map const& files = p.files;

	string_map::const_iterator const it = files.find(Utils::LowerCase(name));

	return(it != files.end()) ? MakePath(p.project_path, it->second) : "";
}

bool FileFinder::IsRPG2kProject(ProjectTree const& dir) {
	string_map::const_iterator const
		ldb_it = dir.files.find(Utils::LowerCase(DATABASE_NAME)),
		lmt_it = dir.files.find(Utils::LowerCase(TREEMAP_NAME));

	return(ldb_it != dir.files.end() && lmt_it != dir.files.end());
}

bool FileFinder::IsEasyRpgProject(ProjectTree const& dir){
	string_map::const_iterator const
		ldb_it = dir.files.find(Utils::LowerCase(DATABASE_NAME_EASYRPG)),
		lmt_it = dir.files.find(Utils::LowerCase(TREEMAP_NAME_EASYRPG));

	return(ldb_it != dir.files.end() && lmt_it != dir.files.end());
}

std::string FileFinder::FindMusic(const std::string& name) {
#ifdef EMSCRIPTEN
	return FindDefault("Music", name);
#endif

	static const char* MUSIC_TYPES[] = {
		".wav", ".ogg", ".mid", ".midi", ".mp3", NULL };
	return FindFile("Music", name, MUSIC_TYPES);
}

std::string FileFinder::FindSound(const std::string& name) {
#ifdef EMSCRIPTEN
	return FindDefault("Sound", name);
#endif

	static const char* SOUND_TYPES[] = {
		".wav", ".ogg", ".mp3", NULL };
	return FindFile("Sound", name, SOUND_TYPES);
}

bool FileFinder::Exists(std::string const& filename) {
#ifdef _WIN32
	return ::GetFileAttributesW(Utils::ToWideString(filename).c_str()) != (DWORD)-1;
#elif GEKKO
	struct stat sb;
	return ::stat(filename.c_str(), &sb) == 0;
#else
	return ::access(filename.c_str(), F_OK) != -1;
#endif
}

bool FileFinder::IsDirectory(std::string const& dir) {
#ifdef EMSCRIPTEN
	std::stringstream hexpath;
	hexpath << std::setw(2) << std::setfill('0') << std::hex << std::uppercase;
	std::copy((const unsigned char*)&*dir.begin(), (const unsigned char*)&*dir.end(), std::ostream_iterator<unsigned int>(hexpath, " "));
	Output::Debug("IsDirectory() hex dump: %s", hexpath.str().c_str());
#else
	assert(Exists(dir));
#endif
#ifdef _WIN32
	int attribs = ::GetFileAttributesW(Utils::ToWideString(dir).c_str());
	return (attribs & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT))
	      == FILE_ATTRIBUTE_DIRECTORY;
#else
	struct stat sb;
#   ifdef GEKKO
	BOOST_VERIFY(::stat(dir.c_str(), &sb) != -1);
#   else
#      ifdef EMSCRIPTEN
	::lstat(dir.c_str(), &sb);
#      else
	BOOST_VERIFY(::lstat(dir.c_str(), &sb) != -1);
#   endif
	return S_ISDIR(sb.st_mode);
#endif
}

FileFinder::Directory FileFinder::GetDirectoryMembers(const std::string& path, FileFinder::Mode const m, const std::string& parent) {
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
		    break;
		case DIRECTORIES:
			if(! IsDirectory(MakePath(path, name))) { continue; }
			break;
		case ALL:
			break;
		case RECURSIVE:
			if(IsDirectory(MakePath(path, name))) {
				Directory rdir = GetDirectoryMembers(MakePath(path, name), RECURSIVE, MakePath(parent, name));
				result.members.insert(rdir.members.begin(), rdir.members.end());
				continue;
			}

			result.members[Utils::LowerCase(MakePath(parent, name))] = MakePath(parent, name);
			continue;
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
