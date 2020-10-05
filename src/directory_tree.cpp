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

#include "directory_tree.h"
#include "filefinder.h"
#include "output.h"
#include "platform.h"
#include "player.h"
#include <lcf/reader_util.h>

#define EP_DEBUG_DIRECTORYTREE
#ifdef EP_DEBUG_DIRECTORYTREE
template <typename... Args>
static void DebugLog(const char* fmt, Args&&... args) {
	Output::Debug(fmt, std::forward<Args>(args)...);
}
#else
template <typename... Args>
static void DebugLog(const char*, Args&&...) {}
#endif

namespace {
	std::string make_key(StringView n) {
		return Utils::LowerCase(lcf::ReaderUtil::Normalize(n));
	};
}

std::unique_ptr<DirectoryTree> DirectoryTree::Create(std::string path) {
	// FIXME: Requires VFS handle passed in, currently hardcoded to FileFinder

	if (!(FileFinder::Exists(path) && FileFinder::IsDirectory(path, true))) {
		return std::unique_ptr<DirectoryTree>();
	}
	std::unique_ptr<DirectoryTree> tree = std::make_unique<DirectoryTree>();
	tree->root = std::move(path);

	DebugLog("Create: {}", tree->root);

	return tree;
}

std::vector<DirectoryTree::DirectoryEntry> DirectoryTree::ListDirectory(StringView path, bool *error) const {
	std::vector<DirectoryEntry> entries;
	std::string full_path = MakePath(path);

	DebugLog("ListDirectory: {}", full_path);

	if (error) {
		*error = false;
	}

	DirectoryEntry result;

	Platform::Directory dir(full_path);
	if (!dir) {
		Output::Debug("Error opening dir {}: {}", path, ::strerror(errno));
		if (error) {
			*error = true;
		}
		return entries;
	}

	while (dir.Read()) {
		const std::string name = dir.GetEntryName();
		Platform::FileType type = dir.GetEntryType();

		static bool has_fast_dir_stat = true;
		bool is_directory = false;
		if (has_fast_dir_stat) {
			if (type == Platform::FileType::Unknown) {
				has_fast_dir_stat = false;
			} else {
				is_directory = type == Platform::FileType::Directory;
			}
		}

		if (!has_fast_dir_stat) {
			is_directory = FileFinder::IsDirectory(FileFinder::MakePath(path, name), true);
		}

		if (name == "." || name == "..") {
			continue;
		}

		result.type = is_directory ? FileType::Directory : FileType::Regular;
		result.name = name;

		entries.push_back(result);

#if 0
		FIXME
		std::string name_norm = lcf::ReaderUtil::Normalize(name);
		if (is_directory) {
			if (result.directories.find(name_norm) != result.directories.end()) {
				Output::Warning("This game provides the folder \"{}\" twice.", name);
				Output::Warning("This can lead to file not found errors. Merge the directories manually in a file browser.");
			}
			result.directories[name_norm] = name;
		} else {
			result.files[name_norm] = name;
		}
#endif
	}

	return entries;
}

DirectoryTreeView DirectoryTree::AsView(std::string sub_path) {
	return DirectoryTreeView(this, std::move(sub_path));
}

std::string DirectoryTree::FindFile(StringView filename, Span<StringView> exts) const {
	std::string dir, name, canonical_path;
	// Few games (e.g. Yume2kki) use path traversal (..) in the filenames to point
	// to files outside of the actual directory.
	canonical_path = FileFinder::MakeCanonical(filename, 1);

#ifdef EMSCRIPTEN
	if (Exists(canonical_path))
		return canonical_path;
#endif

	std::tie(dir, name) = FileFinder::GetPathAndFilename(canonical_path);
	DebugLog("FindFile: {} | {} | {}", canonical_path, dir, name);

	std::string dir_key = make_key(dir);

	auto dir_it = dir_cache.find(dir_key);
	if (dir_it == dir_cache.end()) {
		assert(fs_cache.find(dir_key) == fs_cache.end());

		// FIXME: Error handling
		auto entries = ListDirectory(dir);
		dir_cache[dir_key] = dir;

		std::unordered_map<std::string, DirectoryEntry> fs_cache_entry;
		for (auto& entry : entries) {
			fs_cache_entry[make_key(entry.name)] = entry;
		}
		fs_cache.emplace(dir_key, fs_cache_entry);

		dir_it = dir_cache.find(dir_key);
	}

	auto file_it = fs_cache.find(dir_key);
	assert(file_it != fs_cache.end());

	std::string name_key = make_key(name);
	if (exts.empty()) {
		auto entry_it = file_it->second.find(name_key);
		if (entry_it != file_it->second.end()) {
			return FileFinder::MakePath((*dir_it).second, (*entry_it).second.name);
		}
	} else {
		for (const auto& ext : exts) {
			auto entry_it = file_it->second.find(name_key + ToString(ext));
			if (entry_it != file_it->second.end()) {
				return FileFinder::MakePath((*dir_it).second, (*entry_it).second.name);
			}
		}
	}

	Output::Debug("Cannot find: {}/{}", dir, name);

	return "";
}

std::string DirectoryTree::FindFile(StringView directory, StringView filename, lcf::Span<StringView> exts) const {
	return FindFile(FileFinder::MakePath(directory, filename), exts);
}

std::string DirectoryTree::MakePath(StringView subpath) const {
	Output::Debug("MakePath {} {} : {}", root, subpath, FileFinder::MakePath(root, subpath));
	return FileFinder::MakePath(root, subpath);
}

StringView DirectoryTree::GetRootPath() const {
	return root;
}

std::string DirectoryTreeView::FindFile(StringView name, Span<StringView> exts) const {
	assert(tree);
	DebugLog("TreeView FindFile: {}", name);
	return tree->FindFile(MakePath(name), exts);
}

std::string DirectoryTreeView::FindFile(StringView dir, StringView name, Span<StringView> exts) const {
	assert(tree);
	DebugLog("TreeView FindFile: {} {}", dir, name);
	return tree->FindFile(MakePath(dir), name, exts);
}

StringView DirectoryTreeView::GetRootPath() const {
	assert(tree);
	return tree->MakePath(sub_path);
}

std::string DirectoryTreeView::MakePath(StringView subdir) const {
	assert(tree);
	return FileFinder::MakePath(sub_path, subdir);
}
