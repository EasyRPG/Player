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

#ifndef EP_DIRECTORY_TREE_H
#define EP_DIRECTORY_TREE_H

#include <string>
#include <vector>
#include <unordered_map>

#include "span.h"
#include "string_view.h"

class DirectoryTreeView;

class DirectoryTree {
public:
	enum class FileType {
		Regular,
		Directory,
		Other
	};

	/**
	 * A entry of a directory (eg. file or subdir)
	 */
	struct DirectoryEntry {
		std::string name;
		FileType type;
	};

	static std::unique_ptr<DirectoryTree> Create(std::string path);
	DirectoryTreeView AsView(std::string sub_path = "");

	// Helper functions for finding files in a case insensitive way
	/**
	 * Does a case insensitive search for the file.
	 *
	 * @param filename a path relative to the filesystem root
	 * @param exts List of file extensions to probe
	 * @return Path to file or empty string when not found
	 */
	std::string FindFile(StringView filename, Span<StringView> exts = {}) const;

	/**
	 * Does a case insensitive search for the file in a specific
	 * directory.
	 *
	 * @param directory a path relative to the filesystem root
	 * @param filename Name of the file to search
	 * @param exts List of file extensions to probe
	 * @return Path to file or empty string when not found
	 */
	std::string FindFile(StringView directory, StringView filename, lcf::Span<StringView> exts = {}) const;

	StringView GetRootPath() const;

	std::string MakePath(StringView subpath) const;

private:
	std::vector<DirectoryEntry> ListDirectory(StringView path, bool* error = nullptr) const;

	std::string root;

	// lowered dir (full path from root) -> <map of> lowered file -> Entry
	mutable std::unordered_map<std::string, std::unordered_map<std::string, DirectoryEntry>> fs_cache;
	// lowered dir -> real dir (both full path from root)
	mutable std::unordered_map<std::string, std::string> dir_cache;
};

class DirectoryTreeView {
public:
	DirectoryTreeView() = default;
	DirectoryTreeView(const DirectoryTree* tree, std::string sub_path) :
		tree(tree), sub_path(std::move(sub_path)) {};

	std::string FindFile(StringView filename, Span<StringView> exts = {}) const;
	std::string FindFile(StringView directory, StringView filename, lcf::Span<StringView> exts = {}) const;
	StringView GetRootPath() const;
	std::string MakePath(StringView subdir) const;

private:
	const DirectoryTree* tree = nullptr;
	std::string sub_path;
};

#endif
