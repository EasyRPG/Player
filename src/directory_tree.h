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
#include <memory>
#include <unordered_map>

#include "span.h"
#include "string_view.h"

class DirectoryTreeView;

/**
 * A directory tree manages case-insenseitive file searching in a root folder
 * and its subdirectories.
 * RTP and translation support can be enabled via advanced arguments.
 * For performance reasons the entries are cached.
 */
class DirectoryTree {
public:
	/** Type of a Entry */
	enum class FileType {
		/** Regular file */
		Regular,
		/** Directory */
		Directory,
		/** Anything of no interest such as block devices */
		Other
	};

	/** A entry of a directory (eg. file or subdirectory) */
	struct Entry {
		/** Unmodified filename */
		std::string name;
		/** File type */
		FileType type;

		Entry(std::string name, FileType type) : name(std::move(name)), type(type) {}
	};

	/** Argument struct for more complex find operations */
	struct Args {
		/** File relative to the current tree to search */
		std::string path;
		/** File extensions to append to the filename when searching */
		Span<StringView> exts;
		/**
		 * How often moving upwards when ".." is encountered in the path is
		 * allowed (to prevent directory traversal)
		 */
		int canonical_initial_deepness = 0;
		/**
		 * When true the RTP paths are scanned for the file and translated
		 * RTP namings are checked.
		 */
		bool use_rtp = false;
		/**
		 * When true a "Not Found" message is shown in the terminal.
		 * Off by default because file probing would spam the terminal alot.
		 */
		bool file_not_found_warning = false;
		/**
		 * When true the currently active translation folder is checked for
		 * the existance of the requested asset and redirected.
		 */
		bool translate = false;
	};

	using DirectoryListType = std::unordered_map<std::string, Entry>;

	/** @return Constructs an empty, invalid DirectoryTree */
	static std::unique_ptr<DirectoryTree> Create();

	/**
	 * Creates a new DirectoryTree
	 *
	 * @param path root path of the tree
	 * @return new DirectoryTree
	 */
	static std::unique_ptr<DirectoryTree> Create(std::string path);

	/**
	 * Creates a new view on the tree that is rooted at the sub_path.
	 *
	 * @param sub_path root of the subtree
	 * @return subtree rooted at sub_path
	 */
	DirectoryTreeView Subtree(std::string sub_path);

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
	std::string FindFile(StringView directory, StringView filename, Span<StringView> exts = {}) const;

	/**
	 * Does a case insensitive search for a file.
	 * Advanced version for special purposes searches. Usually not needed.
	 * 
	 * @see DirectoryTree::Args
	 * @param args See documentation of DirectoryTree::Args
	 * @return Path to file or empty string when not found
	 */
	std::string FindFile(const DirectoryTree::Args& args) const;

	/** @return root path of the tree */
	StringView GetRootPath() const;

	/** 
	 * @param subpath Path to append to the root
	 * @return Combined path
	 */
	std::string MakePath(StringView subpath) const;

	/**
	 * Enumerates a directory.
	 *
	 * @param path Path to enumerate, empty for root path
	 * @return list of directory entries or nullptr on failure
	 */
	DirectoryListType* ListDirectory(StringView path = "") const;

	/** Implicit conversion to TreeView */
	operator DirectoryTreeView ();

private:
	std::string root;

	/** lowered dir (full path from root) -> <map of> lowered file -> Entry */
	mutable std::unordered_map<std::string, DirectoryListType> fs_cache;

	/** lowered dir -> real dir (both full path from root) */
	mutable std::unordered_map<std::string, std::string> dir_cache;
};

/**
 * A TreeView is a non-owning view at a subdirectory of the base DirectoryTree.
 * The DirectoryTree must stay valid.
 */
class DirectoryTreeView {
public:
	/** Constructs an invalid view */
	DirectoryTreeView() = default;

	/**
	 * @param tree Tree this view is based on
	 * @param sub_path Path relative to the tree
	 */
	DirectoryTreeView(const DirectoryTree* tree, std::string sub_path);

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
	std::string FindFile(StringView directory, StringView filename, Span<StringView> exts = {}) const;

	/**
	 * Does a case insensitive search for a file.
	 * Advanced version for special purposes searches. Usually not needed.
	 * 
	 * @see DirectoryTree::Args
	 * @param args See documentation of DirectoryTree::Args
	 * @return Path to file or empty string when not found
	 */
	std::string FindFile(const DirectoryTree::Args& args) const;

	/** @return root path of the subtree */
	StringView GetRootPath() const;

	/** 
	 * @param subpath Path to append to the subtree root
	 * @return Combined path
	 */
	std::string MakePath(StringView subdir) const;

	/**
	 * Enumerates a directory.
	 *
	 * @param path Path to enumerate, empty for subtree root path
	 * @return list of directory entries or nullptr on failure
	 */
	DirectoryTree::DirectoryListType* ListDirectory(StringView path = "") const;

	/**
	 * Creates a new view on the subtree that is rooted at the sub_path.
	 *
	 * @param sub_path root of the subtree
	 * @return subtree rooted at sub_path
	 */
	DirectoryTreeView Subtree(const std::string& sub_path);

	/** @return true when the subtree points at a readable directory */
	explicit operator bool() const noexcept;

private:
	const DirectoryTree* tree = nullptr;
	std::string sub_path;
	bool valid = false;
};

inline DirectoryTreeView::operator bool() const noexcept {
	return valid;
}

inline bool operator<(const DirectoryTree::Entry& l, const DirectoryTree::Entry& r) {
	return std::tie(l.name, l.type) < std::tie(r.name, r.type);
}

inline bool operator==(const DirectoryTree::Entry& l, const DirectoryTree::Entry& r) {
	return l.name == r.name && l.type == r.type;
}

inline bool operator<=(const DirectoryTree::Entry& l, const DirectoryTree::Entry& r) {
	return l < r || l == r;
}

inline bool operator>(const DirectoryTree::Entry& l, const DirectoryTree::Entry& r) {
	return !(l <= r);
}

inline bool operator!=(const DirectoryTree::Entry& l, const DirectoryTree::Entry& r) {
	return !(l == r);
}

inline bool operator>=(const DirectoryTree::Entry& l, const DirectoryTree::Entry& r) {
	return !(l < r);
}

inline DirectoryTree::operator DirectoryTreeView() { return Subtree(""); }

#endif
