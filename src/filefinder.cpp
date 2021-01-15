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
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

#ifdef _WIN32
#  include <windows.h>
#  include <shlobj.h>
#endif

#include "system.h"
#include "options.h"
#include "utils.h"
#include "filefinder.h"
#include "fileext_guesser.h"
#include "output.h"
#include "player.h"
#include "registry.h"
#include "main_data.h"
#include <lcf/reader_util.h>
#include "platform.h"

// MinGW shlobj.h does not define this
#ifndef SHGFP_TYPE_CURRENT
#define SHGFP_TYPE_CURRENT 0
#endif

namespace {
#ifdef SUPPORT_MOVIES
	auto MOVIE_TYPES = { ".avi", ".mpg" };
#endif

	std::unique_ptr<DirectoryTree> game_directory_tree;
	std::string fonts_path;
}

DirectoryTreeView FileFinder::GetDirectoryTree() {
	return *game_directory_tree;
}

std::unique_ptr<DirectoryTree> FileFinder::CreateSaveDirectoryTree() {
	std::string save_path = Main_Data::GetSavePath();
	return DirectoryTree::Create(save_path);
}

void FileFinder::SetDirectoryTree(std::unique_ptr<DirectoryTree> directory_tree) {
	game_directory_tree = std::move(directory_tree);
}

std::unique_ptr<DirectoryTree> FileFinder::CreateDirectoryTree(std::string p) {
	return DirectoryTree::Create(std::move(p));
}

std::string FileFinder::MakePath(StringView dir, StringView name) {
	std::string str;
	if (dir.empty()) {
		str = ToString(name);
	} else if (name.empty()) {
		str = ToString(dir);
	} else {
		str = std::string(dir) + "/" + std::string(name);
	}

#ifdef _WIN32
	std::replace(str.begin(), str.end(), '/', '\\');
#else
	std::replace(str.begin(), str.end(), '\\', '/');
#endif
	return str;
}

std::string FileFinder::MakeCanonical(StringView path, int initial_deepness) {
	bool initial_slash = !path.empty() && path[0] == '/';

	std::vector<std::string> path_components = SplitPath(path);
	std::vector<std::string> path_can;

	for (std::string path_comp : path_components) {
		if (path_comp == "..") {
			if (path_can.size() > 0) {
				path_can.pop_back();
			} else if (initial_deepness > 0) {
				// Ignore, we are in root
				--initial_deepness;
			} else {
				Output::Debug("Path traversal out of game directory: {}", path);
			}
		} else if (path_comp.empty() || path_comp == ".") {
			// ignore
		} else {
			path_can.push_back(path_comp);
		}
	}

	std::string ret;
	for (StringView s : path_can) {
		ret = MakePath(ret, s);
	}

	return (initial_slash ? "/" : "") + ret;
}

std::vector<std::string> FileFinder::SplitPath(StringView path) {
	// Tokens are path delimiters ("/" and encoding aware "\")
	std::function<bool(char32_t)> f = [](char32_t t) {
		char32_t escape_char_back = '\0';
		if (!Player::escape_symbol.empty()) {
			escape_char_back = Utils::DecodeUTF32(Player::escape_symbol).front();
		}
		char32_t escape_char_forward = Utils::DecodeUTF32("/").front();
		return t == escape_char_back || t == escape_char_forward;
	};
	return Utils::Tokenize(path, f);
}

std::pair<std::string, std::string> FileFinder::GetPathAndFilename(StringView path) {
	std::string path_copy = ToString(path);
	ConvertPathDelimiters(path_copy);

	const size_t last_slash_idx = path_copy.find_last_of('/');
	if (last_slash_idx == std::string::npos) {
		return {ToString(""), path_copy};
	}

	return {
		path_copy.substr(0, last_slash_idx),
		path_copy.substr(last_slash_idx + 1)
	};
}

void FileFinder::ConvertPathDelimiters(std::string& path) {
	const std::string& esc = Player::escape_symbol;
	if (!esc.empty()) {
		std::size_t escape_pos = path.find(esc);
		while (escape_pos != std::string::npos) {
			path.erase(escape_pos, esc.length());
			path.insert(escape_pos, "/");
			escape_pos = path.find(esc);
		}
	}
}

std::string FileFinder::GetPathInsidePath(StringView path_to, StringView path_in) {
	if (!path_in.starts_with(path_to)) {
		return ToString(path_in);
	}

	StringView path_out = path_in.substr(path_to.size());
	if (!path_out.empty() && (path_out[0] == '/' || path_out[0] == '\\')) {
		path_out = path_out.substr(1);
	}

	return ToString(path_out);
}

std::string FileFinder::GetPathInsideGamePath(StringView path_in) {
	return FileFinder::GetPathInsidePath(ToString(GetDirectoryTree().GetRootPath()), path_in);
}

#if defined(_WIN32) && !defined(_ARM_)
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

std::string GetFontFilename(StringView name) {
	std::string real_name = Registry::ReadStrValue(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", ToString(name) + " (TrueType)");
	if (real_name.length() > 0) {
		if (FileFinder::Exists(real_name))
			return real_name;
		if (FileFinder::Exists(GetFontsPath() + real_name))
			return GetFontsPath() + real_name;
	}

	real_name = Registry::ReadStrValue(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Fonts", ToString(name) + " (TrueType)");
	if (real_name.length() > 0) {
		if (FileFinder::Exists(real_name))
			return real_name;
		if (FileFinder::Exists(GetFontsPath() + real_name))
			return GetFontsPath() + real_name;
	}

	return ToString(name);
}
#endif

std::string FileFinder::FindFont(StringView name) {
	auto FONTS_TYPES = Utils::MakeSvArray(".ttf", ".ttc", ".otf", ".fon");
	std::string path = game_directory_tree->FindFile({ MakePath("Font", name), FONTS_TYPES, 1, true, true });

#if defined(_WIN32) && !defined(_ARM_)
	if (!path.empty()) {
		return path;
	}

	std::string folder_path = "";
	std::string filename = ToString(name);

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

void FileFinder::Quit() {
	game_directory_tree.reset();
}


Filesystem_Stream::InputStream FileFinder::OpenInputStream(const std::string& name, std::ios_base::openmode m) {
	auto* buf = new std::filebuf();
	buf->open(
#ifdef _MSC_VER
		Utils::ToWideString(name).c_str(),
#else
		name.c_str(),
#endif
		m);

	if (!buf->is_open()) {
		delete buf;
		return Filesystem_Stream::InputStream();
	}

	Filesystem_Stream::InputStream is(buf);
	return is;
}

Filesystem_Stream::OutputStream FileFinder::OpenOutputStream(const std::string& name, std::ios_base::openmode m) {
	auto* buf = new std::filebuf();
	buf->open(
#ifdef _MSC_VER
		Utils::ToWideString(name).c_str(),
#else
		name.c_str(),
#endif
		m);

	if (!buf->is_open()) {
		delete buf;
		return Filesystem_Stream::OutputStream();
	}

	Filesystem_Stream::OutputStream os(buf);
	return os;
}

std::string FileFinder::FindImage(StringView dir, StringView name) {
#ifdef EMSCRIPTEN
	return FindDefault(dir, name);
#endif

	auto IMG_TYPES = Utils::MakeSvArray(".bmp",  ".png", ".xyz");
	return game_directory_tree->FindFile({ MakePath(dir, name), IMG_TYPES, 1, true, true, true });
}

std::string FileFinder::FindDefault(StringView dir, StringView name) {
	return GetDirectoryTree().FindFile(dir, name);
}

std::string FileFinder::FindDefault(StringView name) {
	return GetDirectoryTree().FindFile(name);
}

bool FileFinder::IsValidProject(const DirectoryTreeView& tree) {
	return IsRPG2kProject(tree) || IsEasyRpgProject(tree) || IsRPG2kProjectWithRenames(tree);
}

bool FileFinder::IsRPG2kProject(const DirectoryTreeView& tree) {
	return !tree.FindFile(DATABASE_NAME).empty() &&
		!tree.FindFile(TREEMAP_NAME).empty();
}

bool FileFinder::IsEasyRpgProject(const DirectoryTreeView& tree){
	return !tree.FindFile(DATABASE_NAME_EASYRPG).empty() &&
		   !tree.FindFile(TREEMAP_NAME_EASYRPG).empty();
}

bool FileFinder::IsRPG2kProjectWithRenames(const DirectoryTreeView& tree) {
	return !FileExtGuesser::GetRPG2kProjectWithRenames(tree).Empty();
}

bool FileFinder::HasSavegame() {
	return GetSavegames() > 0;
}

int FileFinder::GetSavegames() {
	auto tree = FileFinder::CreateSaveDirectoryTree();

	for (int i = 1; i <= 15; i++) {
		std::stringstream ss;
		ss << "Save" << (i <= 9 ? "0" : "") << i << ".lsd";
		std::string filename = tree->FindFile(ss.str());

		if (!filename.empty()) {
			return true;
		}
	}
	return false;
}

std::string FileFinder::FindMusic(StringView name) {
#ifdef EMSCRIPTEN
	return FindDefault("Music", name);
#endif

	auto MUSIC_TYPES = Utils::MakeSvArray(
		".opus", ".oga", ".ogg", ".wav", ".mid", ".midi", ".mp3", ".wma");
	return game_directory_tree->FindFile({ MakePath("Music", name), MUSIC_TYPES, 1, true, true, true });
}

std::string FileFinder::FindSound(StringView name) {
#ifdef EMSCRIPTEN
	return FindDefault("Sound", name);
#endif

	auto SOUND_TYPES = Utils::MakeSvArray(
		".opus", ".oga", ".ogg", ".wav", ".mp3", ".wma");
	return game_directory_tree->FindFile({ MakePath("Sound", name), SOUND_TYPES, 1, true, true, true });
}

bool FileFinder::Exists(StringView filename) {
	return Platform::File(ToString(filename)).Exists();
}

bool FileFinder::IsDirectory(StringView dir, bool follow_symlinks) {
	return Platform::File(ToString(dir)).IsDirectory(follow_symlinks);
}

int64_t FileFinder::GetFileSize(StringView file) {
	return Platform::File(ToString(file)).GetSize();
}

bool FileFinder::IsMajorUpdatedTree() {
	// Find an MP3 music file only when official Harmony.dll exists
	// in the gamedir or the file doesn't exist because
	// the detection doesn't return reliable results for games created with
	// "RPG2k non-official English translation (older engine) + MP3 patch"
	bool find_mp3 = true;
	std::string harmony = FindDefault("Harmony.dll");
	if (!harmony.empty()) {
		auto size = GetFileSize(harmony);
		if (size != -1 && size != KnownFileSize::OFFICIAL_HARMONY_DLL) {
			Output::Debug("Non-official Harmony.dll found, skipping MP3 test");
			find_mp3 = false;
		}
	}

	if (find_mp3) {
		auto tree = GetDirectoryTree();
		auto entries = tree.ListDirectory("music");
		if (entries) {
			for (const auto& entry : *entries) {
				if (entry.second.type == DirectoryTree::FileType::Regular && StringView(entry.first).ends_with(".mp3")) {
					Output::Debug("MP3 file ({}) found", entry.second.name);
					return true;
				}
			}
		}
	}

	// Compare the size of RPG_RT.exe with threshold
	std::string rpg_rt = FindDefault("RPG_RT.exe");
	if (!rpg_rt.empty()) {
		auto size = GetFileSize(rpg_rt);
		if (size != -1) {
			return size > (Player::IsRPG2k() ? RpgrtMajorUpdateThreshold::RPG2K : RpgrtMajorUpdateThreshold::RPG2K3);
		}
	}
	Output::Debug("Could not get the size of RPG_RT.exe");

	// Assume the most popular version
	// Japanese or RPG2k3 games: newer engine
	// non-Japanese RPG2k games: older engine
	bool assume_newer = Player::IsCP932() || Player::IsRPG2k3();
	Output::Debug("Assuming {} engine", assume_newer ? "newer" : "older");
	return assume_newer;
}
