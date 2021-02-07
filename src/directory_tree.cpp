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
#include "filefinder_rtp.h"
#include "main_data.h"
#include "output.h"
#include "platform.h"
#include "player.h"
#include <lcf/reader_util.h>

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
		return lcf::ReaderUtil::Normalize(n);
	};
}

std::unique_ptr<DirectoryTree> DirectoryTree::Create() {
	return std::make_unique<DirectoryTree>();
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

DirectoryTree::DirectoryListType* DirectoryTree::ListDirectory(StringView path) const {
	std::vector<Entry> entries;
	std::string fs_path = ToString(path);
	std::string full_path = MakePath(path);

	DebugLog("ListDirectory: {}", full_path);

	auto dir_key = make_key(fs_path);

	auto dir_it = dir_cache.find(dir_key);
	if (dir_it != dir_cache.end()) {
		// Already cached
		DebugLog("ListDirectory Cache Hit: {}", dir_key);
		auto file_it = fs_cache.find(dir_key);
		assert(file_it != fs_cache.end());
		return &file_it->second;
	}

	assert(fs_cache.find(dir_key) == fs_cache.end());

	// FIXME: Skip this scan when the directory name matches the passed name

	if (!fs_path.empty()) {
		// Go up and determine the proper casing of the folder
		std::string parent_dir, child_dir;
		std::tie(parent_dir, child_dir) = FileFinder::GetPathAndFilename(fs_path);

		auto* parent_tree = ListDirectory(parent_dir);
		if (!parent_tree) {
			return nullptr;
		}

		auto parent_key = make_key(parent_dir);
		auto parent_it = dir_cache.find(parent_key);
		assert(parent_it != dir_cache.end());

		auto child_key = make_key(child_dir);
		auto child_it = parent_tree->find(child_key);
		if (child_it != parent_tree->end()) {
			full_path = FileFinder::MakePath(MakePath(parent_it->second), child_it->second.name);
			fs_path = FileFinder::MakePath(parent_it->second, child_it->second.name);
		} else {
			return nullptr;
		}
	}

	Platform::Directory dir(full_path);
	if (!dir) {
		Output::Debug("Error opening dir {}: {}", full_path, ::strerror(errno));
		return nullptr;
	}

	while (dir.Read()) {
		const auto& name = dir.GetEntryName();
		Platform::FileType type = dir.GetEntryType();

		if (name == "." || name == "..") {
			continue;
		}

		bool is_directory = false;
		if (type == Platform::FileType::Directory) {
			is_directory = true;
		} else if (type == Platform::FileType::Unknown) {
			is_directory = FileFinder::IsDirectory(FileFinder::MakePath(full_path, name), true);
		}

		if (is_directory) {
			std::string new_entry_key = make_key(name);
			if (std::find_if(entries.begin(), entries.end(), [&](const auto& e) {
				return e.type == DirectoryTree::FileType::Directory && make_key(e.name) == new_entry_key;
			}) != entries.end()) {
				Output::Warning("This game provides the folder \"{}\" twice.", name);
				Output::Warning("This can lead to file not found errors. Merge the directories manually in a file browser.");
			}
		}

		entries.emplace_back(
			name,
			is_directory ? FileType::Directory : FileType::Regular);
	}

	dir_cache[dir_key] = fs_path;

	DirectoryListType fs_cache_entry;

	for (auto& entry : entries) {
		fs_cache_entry.emplace(std::make_pair(make_key(entry.name), entry));
	}
	fs_cache.emplace(dir_key, fs_cache_entry);

	return &fs_cache.find(dir_key)->second;
}

DirectoryTreeView DirectoryTree::Subtree(std::string sub_path) {
	return DirectoryTreeView(this, std::move(sub_path));
}

std::string DirectoryTree::FindFile(StringView filename, Span<StringView> exts) const {
	return FindFile({ ToString(filename), exts });
}

std::string DirectoryTree::FindFile(StringView directory, StringView filename, Span<StringView> exts) const {
	return FindFile({ FileFinder::MakePath(directory, filename), exts });
}

std::string DirectoryTree::FindFile(const DirectoryTree::Args& args) const {
	std::string dir, name, canonical_path;
	// Few games (e.g. Yume2kki) use path traversal (..) in the filenames to point
	// to files outside of the actual directory.
	canonical_path = FileFinder::MakeCanonical(args.path, args.canonical_initial_deepness);

#ifdef EMSCRIPTEN
	if (FileFinder::Exists(canonical_path))
		return canonical_path;
#endif

	if (args.translate && !Tr::GetCurrentTranslationId().empty()) {
		// Search in the active language tree but do not translate again and swallow not found warnings
		auto translated_file = Tr::GetCurrentTranslationTree().FindFile(
				{args.path, args.exts, args.canonical_initial_deepness, args.use_rtp, false, false});
		if (!translated_file.empty()) {
			DebugLog("Translated {} as {}", args.path, translated_file);
			return translated_file;
		}
	}

	std::tie(dir, name) = FileFinder::GetPathAndFilename(canonical_path);
	DebugLog("FindFile: {} | {} | {}", canonical_path, dir, name);

	auto* entries = ListDirectory(dir);
	if (!entries) {
		if (args.file_not_found_warning) {
			Output::Debug("Cannot find: {}/{}", dir, name);
		}
		return "";
	}

	std::string dir_key = make_key(dir);
	auto dir_it = dir_cache.find(dir_key);
	assert(dir_it != dir_cache.end());

	std::string name_key = make_key(name);
	if (args.exts.empty()) {
		auto entry_it = entries->find(name_key);
		if (entry_it != entries->end() && entry_it->second.type == FileType::Regular) {
			return MakePath(FileFinder::MakePath(dir_it->second, entry_it->second.name));
		}
	} else {
		for (const auto& ext : args.exts) {
			auto full_name_key = name_key + ToString(ext);
			auto entry_it = entries->find(full_name_key);
			if (entry_it != entries->end() && entry_it->second.type == FileType::Regular) {
				return MakePath(FileFinder::MakePath(dir_it->second, entry_it->second.name));
			}
		}
	}

	if (args.use_rtp && Main_Data::filefinder_rtp) {
		auto rtp_file = Main_Data::filefinder_rtp->Lookup(dir, name, args.exts);
		if (!rtp_file.empty()) {
			return rtp_file;
		}
	}

	if (args.file_not_found_warning) {
		Output::Debug("Cannot find: {}/{}", dir, name);
	}

	return "";
}

std::string DirectoryTree::MakePath(StringView subpath) const {
	DebugLog("MakePath {} {} : {}", root, subpath, FileFinder::MakePath(root, subpath));
	return FileFinder::MakePath(root, subpath);
}

StringView DirectoryTree::GetRootPath() const {
	return root;
}

DirectoryTreeView::DirectoryTreeView(const DirectoryTree* tree, std::string sub_path) :
	tree(tree), sub_path(std::move(sub_path)) {
	valid = (tree->ListDirectory(this->sub_path) != nullptr);
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

std::string DirectoryTreeView::FindFile(const DirectoryTree::Args& args) const {
	auto args_cp = args;
	std::string path = MakePath(args.path);
	args_cp.path = path;
	return tree->FindFile(args_cp);
}

StringView DirectoryTreeView::GetRootPath() const {
	assert(tree);
	return tree->MakePath(sub_path);
}

std::string DirectoryTreeView::MakePath(StringView subdir) const {
	assert(tree);
	return FileFinder::MakePath(sub_path, subdir);
}

DirectoryTree::DirectoryListType* DirectoryTreeView::ListDirectory(StringView path) const {
	return tree->ListDirectory(MakePath(path));
}

DirectoryTreeView DirectoryTreeView::Subtree(const std::string& sub_path) {
	return DirectoryTreeView(tree, MakePath(sub_path));
}
