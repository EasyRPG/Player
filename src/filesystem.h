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

#ifndef EP_FILESYSTEM_H
#define EP_FILESYSTEM_H

// Headers
#include <istream>
#include <ostream>
#include <unordered_map>
#include <vector>
#include <cassert>
#include "directory_tree.h"
#include "filesystem_stream.h"

class FilesystemView;

class Filesystem {
public:
	enum class Feature {
		/** Filesystem supports Write operations */
		Write = 1
	};

	virtual ~Filesystem() {}

	Filesystem(const Filesystem&) = delete;
	Filesystem& operator=(const Filesystem&) = delete;

	/**
	 * Checks whether the path used to initialize the filesystem exists.
	 *
	 * @return If the filesystem is valid
	 */
	bool IsValid() const;

	/**
	 * Return the path used to initialize the filesystem.
	 * Passing this string to Create of the parent filesystem must result
	 * in a filesystem pointing to the same folder/file as this filesystem.
	 *
	 * @return filesystem root path
	 */
	std::string GetPath() const;

	FilesystemView GetParent() const;

	/**
	 * Creates stream from UTF-8 file name for reading.
	 *
	 * @param name UTF-8 string file name.
	 * @param m stream mode.
	 * @return NULL if open failed.
	 */
	Filesystem_Stream::InputStream OpenInputStream(StringView name,
		std::ios_base::openmode m = std::ios_base::in | std::ios_base::binary) const;

	/**
	 * Creates stream from UTF-8 file name for writing.
	 *
	 * @param name UTF-8 string file name.
	 * @param m stream mode.
	 * @return NULL if open failed.
	 */
	Filesystem_Stream::OutputStream OpenOutputStream(StringView name,
		std::ios_base::openmode m = std::ios_base::out | std::ios_base::binary) const;

	/**
	 * Returns a directory listing of the given path.
	 *
	 * @param path a path relative to the filesystems root
	 * @param error When non-null receives true when reading failed, otherwise false
	 * @return List of directory entries
	 */
	DirectoryTree::DirectoryListType* ListDirectory(StringView path) const;

	/**
	 * Clears the filesystem cache. Changes in the filesystem become visible
	 * to the FindFile functions.
	 * This is automatically called when an output stream is closed.
	 */
	void ClearCache();

	/**
	 * Creates a new appropriate filesystem from the specified path.
	 * The path is processed to initialize the proper virtual filesystem handler.
	 *
	 * @param p Virtual path to use
	 * @return Filesystem when the parsing was successful, otherwise nullptr
	 */
	FilesystemView Create(StringView p) const;

	FilesystemView Subtree(std::string sub_path) const;

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

	/** Implicit conversion to FilesystemView */
	operator FilesystemView();

	/**
	 * Abstract methods to be implemented by filesystems.
	 */
	/** @{ */
	virtual bool IsFile(StringView path) const = 0;
	virtual bool IsDirectory(StringView path, bool follow_symlinks) const = 0;
	virtual bool Exists(StringView path) const = 0;
	virtual int64_t GetFilesize(StringView path) const = 0;
	virtual bool CreateDirectory(StringView dir, bool follow_symlinks) const;
	virtual bool IsFeatureSupported(Feature f) const;
	/** @} */

protected:
	/**
	 * Abstract methods to be implemented by filesystems.
	 */
	/** @{ */
	virtual bool GetDirectoryContent(StringView path, std::vector<DirectoryTree::Entry>& entries) const = 0;
	virtual std::streambuf* CreateInputStreambuffer(StringView path, std::ios_base::openmode mode) const = 0;
	virtual std::streambuf* CreateOutputStreambuffer(StringView path, std::ios_base::openmode mode) const;
	/** @} */

	explicit Filesystem(std::string base_path, FilesystemView parent_fs);

	friend FilesystemView;
	friend DirectoryTree;

	std::unique_ptr<DirectoryTree> tree;

private:
	std::string base_path;
	mutable std::vector<std::unique_ptr<Filesystem>> child_fs;
	std::unique_ptr<FilesystemView> parent_fs;
};

/**
 * A TreeView is a non-owning view at a subdirectory of the base DirectoryTree.
 * The DirectoryTree must stay valid.
 */
class FilesystemView {
public:
	/** Constructs an invalid view */
	FilesystemView() = default;

	/**
	 * @param tree Tree this view is based on
	 * @param sub_path Path relative to the tree
	 */
	FilesystemView(const Filesystem* fs, std::string sub_path);

	std::string GetPath() const;

	const Filesystem& GetOwner() const;

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

	/**
	 * @param subpath Path to append to the subtree root
	 * @return Combined path
	 */
	std::string MakePath(StringView subdir) const;

	bool IsFile(StringView path) const;
	bool IsDirectory(StringView path, bool follow_symlinks) const;
	bool Exists(StringView path) const;
	int64_t GetFilesize(StringView path) const;

	/**
	 * Enumerates a directory.
	 *
	 * @param path Path to enumerate, empty for subtree root path
	 * @return list of directory entries or nullptr on failure
	 */
	DirectoryTree::DirectoryListType* ListDirectory(StringView path = "") const;

	Filesystem_Stream::InputStream OpenInputStream(StringView name,
		std::ios_base::openmode m = std::ios_base::in | std::ios_base::binary) const;

	Filesystem_Stream::OutputStream OpenOutputStream(StringView name,
		std::ios_base::openmode m = std::ios_base::out | std::ios_base::binary) const;

	std::streambuf* CreateInputStreambuffer(StringView path, std::ios_base::openmode mode) const;
	std::streambuf* CreateOutputStreambuffer(StringView path, std::ios_base::openmode mode) const;

	FilesystemView Create(StringView p) const;

	bool CreateDirectory(StringView dir, bool follow_symlinks) const;
	bool IsFeatureSupported(Filesystem::Feature f) const;

	/**
	 * Creates a new view on the subtree that is rooted at the sub_path.
	 *
	 * @param sub_path root of the subtree
	 * @return subtree rooted at sub_path
	 */
	FilesystemView Subtree(StringView sub_path) const;

	/** @return true when the subtree points at a readable directory */
	explicit operator bool() const noexcept;

	friend DirectoryTree;

private:
	const Filesystem* fs = nullptr;
	std::string sub_path;
	bool valid = false;
};

inline FilesystemView::operator bool() const noexcept {
	return valid;
}

inline std::string Filesystem::GetPath() const {
	return base_path;
}

inline FilesystemView Filesystem::GetParent() const {
	return *parent_fs;
}

inline bool Filesystem::IsFeatureSupported(Filesystem::Feature) const {
	return false;
}

inline std::streambuf* Filesystem::CreateOutputStreambuffer(StringView, std::ios_base::openmode) const {
	assert(!IsFeatureSupported(Feature::Write) && "Write supported but CreateOutputStreambuffer not implemented");
	return nullptr;
}

inline DirectoryTree::DirectoryListType* Filesystem::ListDirectory(StringView path) const {
	return tree->ListDirectory(path);
}

inline Filesystem::operator FilesystemView() { return Subtree(""); }

#endif
