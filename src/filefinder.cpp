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
#include "filefinder_rtp.h"
#include "filesystem.h"
#include "filesystem_root.h"
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

	std::string fonts_path;
	std::shared_ptr<Filesystem> root_fs;
	FilesystemView game_fs;
	FilesystemView save_fs;

	constexpr const auto IMG_TYPES = Utils::MakeSvArray(".bmp",  ".png", ".xyz");
	constexpr const auto MUSIC_TYPES = Utils::MakeSvArray(
			".opus", ".oga", ".ogg", ".wav", ".mid", ".midi", ".mp3", ".wma");
	constexpr const auto SOUND_TYPES = Utils::MakeSvArray(
			".opus", ".oga", ".ogg", ".wav", ".mp3", ".wma");
	constexpr const auto FONTS_TYPES = Utils::MakeSvArray(".fon", ".fnt", ".bdf", ".ttf", ".ttc", ".otf", ".woff2", ".woff");
	constexpr const auto TEXT_TYPES = Utils::MakeSvArray(".txt", ".csv");
}

FilesystemView FileFinder::Game() {
	return game_fs;
}

void FileFinder::SetGameFilesystem(FilesystemView filesystem) {
	game_fs = filesystem;
}

FilesystemView FileFinder::Save() {
	if (save_fs) {
		// This means the save filesystem was overwritten
		if (!save_fs.IsFeatureSupported(Filesystem::Feature::Write)) {
			Output::Error("{} is not a valid save path (not writable)", GetFullFilesystemPath(save_fs));
		}
		return save_fs;
	}

	if (!game_fs) {
		// Filesystem not initialized yet (happens on startup)
		return FilesystemView();
	}

	// Not overwritten, check if game fs is writable. If not redirect the write operation.
	if (!game_fs.IsFeatureSupported(Filesystem::Feature::Write)) {
		// When the Project path equals the Save path (this means the path was not configured)
		// and the filesystem has no write support do a redirection to a folder with ".save" appended
		FilesystemView parent = game_fs;
		FilesystemView redir;
		std::vector<std::string> comps;
		for (;;) {
			comps.emplace_back(parent.GetSubPath());
			comps.emplace_back(parent.GetBasePath());
			parent = parent.GetOwner().GetParent();
			if (!parent) {
				break;
			}
			if (parent.IsFeatureSupported(Filesystem::Feature::Write)) {
				comps.back() += ".save";
				std::reverse(comps.begin(), comps.end());
				std::string save_path = MakePath(lcf::MakeSpan(comps));
				if (!parent.IsDirectory(save_path, true)) {
					parent.MakeDirectory(save_path, true);
				}
				redir = parent.Subtree(save_path);

				if (!redir) {
					Output::Error("Invalid save directory {}", save_path);
				}

				break;
			}
		}

		if (!redir) {
			Output::Error("No suitable save directory found");
		}

		return redir;
	}

	return game_fs;
}

void FileFinder::SetSaveFilesystem(FilesystemView filesystem) {
	save_fs = filesystem;
}

FilesystemView FileFinder::Root() {
	if (!root_fs) {
		root_fs = std::make_unique<RootFilesystem>();
	}

	return root_fs->Subtree("");
}

std::string FileFinder::MakePath(StringView dir, StringView name) {
	std::string str;
	if (dir.empty()) {
		str = ToString(name);
	} else if (name.empty()) {
		str = ToString(dir);
	} else if (dir.ends_with("/")) {
		str = ToString(dir) + ToString(name);
	} else {
		str = ToString(dir) + "/" + ToString(name);
	}

	ConvertPathDelimiters(str);

	return str;
}

std::string FileFinder::MakeCanonical(StringView path, int initial_deepness) {
	StringView ns;
	// Check if the path contains a namespace and prevent that the :// is replaced with :/
	auto ns_pos = path.find("://");
	if (ns_pos != std::string::npos) {
		ns = path.substr(0, ns_pos + 3);
		path = path.substr(ns_pos + 3);
	}

	std::string root_slash = (!path.empty() && path[0] == '/') ? "/" : "";

	std::vector<std::string> path_components = SplitPath(path);
	std::vector<std::string> path_can;

	for (const std::string& path_comp : path_components) {
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

	// Determine if the slash of a filesystem root (drive:/) was removed and readd it
	auto psize = path.size();
	std::string drive_slash;
	if (psize >= 2 && path[psize - 1] == '/' && path[psize - 2] == ':') {
		drive_slash = "/";
	}

	return ToString(ns) + root_slash + ret + drive_slash;
}

std::vector<std::string> FileFinder::SplitPath(StringView path) {
	// Tokens are path delimiters ("/" and encoding aware "\")
	std::function<bool(char32_t)> f = [](char32_t t) {
		char32_t escape_char_back = '\0';
		if (!Player::escape_symbol.empty()) {
			escape_char_back = Utils::DecodeUTF32(Player::escape_symbol).front();
		} else {
			escape_char_back = Utils::DecodeUTF32("\\").front();
		}
		char32_t escape_char_forward = Utils::DecodeUTF32("/").front();
		return t == escape_char_back || t == escape_char_forward;
	};
	return Utils::Tokenize(path, f);
}

std::pair<std::string, std::string> FileFinder::GetPathAndFilename(StringView path) {
	if (path.empty()) {
		return {"", ""};
	}

	std::string path_copy = ToString(path);
	ConvertPathDelimiters(path_copy);

	const size_t last_slash_idx = path_copy.find_last_of('/');
	if (last_slash_idx == std::string::npos) {
		return {ToString(""), path_copy};
	}

	// Determine if the file is located at the filesystem root (e.g. / or drive:/)
	// In that case do not remove the slash
	std::string root_slash;
	const size_t first_slash_idx = path_copy.find_first_of('/');
	if (first_slash_idx == last_slash_idx &&
		(first_slash_idx == 0 || path[first_slash_idx - 1] == ':')) {
		root_slash = "/";
	}

	return {
		path_copy.substr(0, last_slash_idx) + root_slash,
		path_copy.substr(last_slash_idx + 1)
	};
}

void FileFinder::ConvertPathDelimiters(std::string& path) {
	auto replace = [&](const std::string& esc_ch) {
		std::size_t escape_pos = path.find(esc_ch);
		while (escape_pos != std::string::npos) {
			path.erase(escape_pos, esc_ch.length());
			path.insert(escape_pos, "/");
			escape_pos = path.find(esc_ch);
		}
	};

	replace("\\");
	if (!Player::escape_symbol.empty() && Player::escape_symbol != "\\") {
		replace(Player::escape_symbol);
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
	return FileFinder::GetPathInsidePath(Game().GetFullPath(), path_in);
}

void FileFinder::Quit() {
	root_fs.reset();
}

bool FileFinder::IsValidProject(const FilesystemView& fs) {
	return IsRPG2kProject(fs) || IsEasyRpgProject(fs) || IsRPG2kProjectWithRenames(fs);
}

bool FileFinder::IsRPG2kProject(const FilesystemView& fs) {
	return !fs.FindFile(DATABASE_NAME).empty() &&
		!fs.FindFile(TREEMAP_NAME).empty();
}

bool FileFinder::IsEasyRpgProject(const FilesystemView& fs){
	return !fs.FindFile(DATABASE_NAME_EASYRPG).empty() &&
		   !fs.FindFile(TREEMAP_NAME_EASYRPG).empty();
}

bool FileFinder::IsRPG2kProjectWithRenames(const FilesystemView& fs) {
	return !FileExtGuesser::GetRPG2kProjectWithRenames(fs).Empty();
}

bool FileFinder::HasSavegame() {
	return GetSavegames() > 0;
}

int FileFinder::GetSavegames() {
	auto fs = Save();

	for (int i = 1; i <= 15; i++) {
		std::stringstream ss;
		ss << "Save" << (i <= 9 ? "0" : "") << i << ".lsd";
		std::string filename = fs.FindFile(ss.str());

		if (!filename.empty()) {
			return true;
		}
	}
	return false;
}

std::string find_generic(const DirectoryTree::Args& args) {
	if (!Tr::GetCurrentTranslationId().empty()) {
		auto tr_fs = Tr::GetCurrentTranslationFilesystem();
		auto translated_file = tr_fs.FindFile(args);
		if (!translated_file.empty()) {
			return translated_file;
		}
	}

	return FileFinder::Game().FindFile(args);
}

std::string FileFinder::FindImage(StringView dir, StringView name) {
	DirectoryTree::Args args = { MakePath(dir, name), IMG_TYPES, 1, false };
	return find_generic(args);
}

std::string FileFinder::FindMusic(StringView name) {
	DirectoryTree::Args args = { MakePath("Music", name), MUSIC_TYPES, 1, false };
	return find_generic(args);

}

std::string FileFinder::FindSound(StringView name) {
	DirectoryTree::Args args = { MakePath("Sound", name), SOUND_TYPES, 1, false };
	return find_generic(args);
}

std::string FileFinder::FindFont(StringView name) {
	DirectoryTree::Args args = { MakePath("Font", name), FONTS_TYPES, 1, true };
	return find_generic(args);
}

std::string FileFinder::FindText(StringView name) {
	DirectoryTree::Args args = { MakePath("Text", name), TEXT_TYPES, 1, true };
	return find_generic(args);
}

Filesystem_Stream::InputStream open_generic(StringView dir, StringView name, DirectoryTree::Args& args) {
	if (!Tr::GetCurrentTranslationId().empty()) {
		auto tr_fs = Tr::GetCurrentTranslationFilesystem();
		auto is = tr_fs.OpenFile(args);
		if (is) {
			return is;
		}
	}

	auto is = FileFinder::Game().OpenFile(args);
	if (!is && Main_Data::filefinder_rtp) {
		is = Main_Data::filefinder_rtp->Lookup(dir, name, args.exts);
		if (!is) {
			Output::Debug("Cannot find: {}/{}", dir, name);
		}
	}
	return is;
}

Filesystem_Stream::InputStream FileFinder::OpenImage(StringView dir, StringView name) {
	DirectoryTree::Args args = { MakePath(dir, name), IMG_TYPES, 1, false };
	return open_generic(dir, name, args);
}

Filesystem_Stream::InputStream FileFinder::OpenMusic(StringView name) {
	DirectoryTree::Args args = { MakePath("Music", name), MUSIC_TYPES, 1, false };
	return open_generic("Music", name, args);
}

Filesystem_Stream::InputStream FileFinder::OpenSound(StringView name) {
	DirectoryTree::Args args = { MakePath("Sound", name), SOUND_TYPES, 1, false };
	return open_generic("Sound", name, args);
}

Filesystem_Stream::InputStream FileFinder::OpenFont(StringView name) {
	DirectoryTree::Args args = { MakePath("Font", name), FONTS_TYPES, 1, false };
	return open_generic("Font", name, args);
}

Filesystem_Stream::InputStream FileFinder::OpenText(StringView name) {
	DirectoryTree::Args args = { MakePath("Text", name), TEXT_TYPES, 1, false };
	return open_generic("Text", name, args);
}

bool FileFinder::IsMajorUpdatedTree() {
	auto fs = Game();
	assert(fs);

	// Find an MP3 music file only when official Harmony.dll exists
	// in the gamedir or the file doesn't exist because
	// the detection doesn't return reliable results for games created with
	// "RPG2k non-official English translation (older engine) + MP3 patch"
	bool find_mp3 = true;
	std::string harmony = Game().FindFile("Harmony.dll");
	if (!harmony.empty()) {
		auto size = fs.GetFilesize(harmony);
		if (size != -1 && size != KnownFileSize::OFFICIAL_HARMONY_DLL) {
			Output::Debug("Non-official Harmony.dll found, skipping MP3 test");
			find_mp3 = false;
		}
	}

	if (find_mp3) {
		auto entries = fs.ListDirectory("music");
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
	std::string rpg_rt = Game().FindFile("RPG_RT.exe");
	if (!rpg_rt.empty()) {
		auto size = fs.GetFilesize(rpg_rt);
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

std::string FileFinder::GetFullFilesystemPath(FilesystemView fs) {
	FilesystemView cur_fs = fs;
	std::string full_path;
	while (cur_fs) {
		full_path = MakePath(cur_fs.GetFullPath(), full_path);
		cur_fs = cur_fs.GetOwner().GetParent();
	}
	return full_path;
}

void FileFinder::DumpFilesystem(FilesystemView fs) {
	FilesystemView cur_fs = fs;
	int i = 1;
	while (cur_fs) {
		Output::Debug("{}: {}", i++, cur_fs.Describe());
		cur_fs = cur_fs.GetOwner().GetParent();
	}
}
