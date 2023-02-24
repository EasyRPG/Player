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
#include <cassert>
#include <cstdint>
#include <memory>
#include <istream>
#include <ostream>
#include <unordered_map>
#include <vector>
#include "directory_tree.h"

class FilesystemView;
class RootFilesystem;
namespace Filesystem_Stream {
	class InputStream;
	class OutputStream;
}

/**
 * The base class for a filesystem abstraction.
 * A virtual filesystem provides ways to open files and read directories even
 * if they are not real filesystem structures in terms of the operating system
 * e.g. ZIP archives or internet resources.
 */
class Filesystem : public std::enable_shared_from_this<Filesystem> {
public:
	/** Features provided by the filesystem */
	enum class Feature {
		/** Filesystem supports Write operations */
		Write = 1
	};

	virtual ~Filesystem() = default;

	Filesystem(const Filesystem&) = delete;
	Filesystem& operator=(const Filesystem&) = delete;

	/**
	 * Checks whether the path used to initialize the filesystem exists.
	 * Calling functions on an invalid filesystem is undefined.
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

	/**
	 * Returns the owner of this filesystem.
	 * When there is no owner an invalid Filesystem is returned.
	 *
	 * @return parent filesystem
	 */
	FilesystemView GetParent() const;

	/**
	 * Creates stream from filename for reading.
	 *
	 * @param name filename.
	 * @param m stream mode. Default is binary.
	 * @return A Stream. The stream is invalid when the open failed.
	 */
	Filesystem_Stream::InputStream OpenInputStream(StringView name,
		std::ios_base::openmode m = std::ios_base::in | std::ios_base::binary) const;

	/**
	 * Creates stream from filename for reading.
	 * When the file is missing it is created.
	 *
	 * @param name filename.
	 * @param m stream mode. Default is binary.
	 * @return A Stream. The stream is invalid when the open failed.
	 */
	Filesystem_Stream::InputStream OpenOrCreateInputStream(StringView name,
		std::ios_base::openmode m = std::ios_base::in | std::ios_base::binary) const;

	/**
	 * Creates stream from filename for writing.
	 *
	 * @param name filename.
	 * @param m stream mode. Default is binary.
	 * @return A Stream. The stream is invalid when the open failed or write is not supported.
	 */
	Filesystem_Stream::OutputStream OpenOutputStream(StringView name,
		std::ios_base::openmode m = std::ios_base::out | std::ios_base::binary) const;

	/**
	 * Returns a directory listing of the given path.
	 *
	 * @param path a path relative to the filesystems root
	 * @return List of directory entries
	 */
	DirectoryTree::DirectoryListType* ListDirectory(StringView path) const;

	/**
	 * Clears the filesystem cache. Changes in the filesystem become visible
	 * to the FindFile functions.
	 * This is automatically called when an output stream is closed.
	 *
	 * @param path Path to flush
	 */
	void ClearCache(StringView path) const;

	/**
	 * Creates a new appropriate filesystem from the specified path.
	 * The path is processed to initialize the proper virtual filesystem handler.
	 *
	 * @param p Virtual path to use
	 * @return Valid Filesystem when the parsing was successful, otherwise invalid
	 */
	virtual FilesystemView Create(StringView p) const;

	/**
	 * Creates a subview into the filesystem tree.
	 * This function is much faster than Create and should be preferred because it
	 * does not attempt to create new virtual filesystems.
	 *
	 * @param sub_path sub path to view
	 * @return Valid Filesystem when the path exists, otherwise invalid
	 */
	FilesystemView Subtree(std::string sub_path) const;

	// Helper functions for finding files in a case insensitive way
	/**
	 * Does a case insensitive search for the file.
	 *
	 * @param filename a path relative to the filesystem root
	 * @param exts List of file extensions to probe
	 * @return Path to file or empty string when not found
	 */
	std::string FindFile(StringView filename, const Span<const StringView> exts = {}) const;

	/**
	 * Does a case insensitive search for the file in a specific
	 * directory.
	 *
	 * @param directory a path relative to the filesystem root
	 * @param filename Name of the file to search
	 * @param exts List of file extensions to probe
	 * @return Path to file or empty string when not found
	 */
	std::string FindFile(StringView directory, StringView filename, const Span<const StringView> exts = {}) const;

	/**
	 * Does a case insensitive search for a file.
	 * Advanced version for special purposes searches. Usually not needed.
	 *
	 * @see DirectoryTree::Args
	 * @param args See documentation of DirectoryTree::Args
	 * @return Path to file or empty string when not found
	 */
	std::string FindFile(const DirectoryTree::Args& args) const;

	// Helper functions for finding and opening files
	/**
	 * Does a case insensitive search for the file and opens a read handle.
	 *
	 * @param filename a path relative to the filesystem root
	 * @param exts List of file extensions to probe
	 * @return Handle to the file if found, otherwise an invalid handle
	 */
	Filesystem_Stream::InputStream OpenFile(StringView filename, const Span<const StringView> exts = {}) const;

	/**
	 * Does a case insensitive search for the file in a specific directory
	 * and opens a read handle.
	 *
	 * @param directory a path relative to the filesystem root
	 * @param filename Name of the file to search
	 * @param exts List of file extensions to probe
	 * @return Handle to the file if found, otherwise an invalid handle
	 */
	Filesystem_Stream::InputStream OpenFile(StringView directory, StringView filename, const Span<const StringView> exts = {}) const;

	/**
	 * Does a case insensitive search for the file and opens a read handle.
	 * Advanced version for special purposes searches. Usually not needed.
	 *
	 * @see DirectoryTree::Args
	 * @param args See documentation of DirectoryTree::Args
	 * @return Handle to the file if found, otherwise an invalid handle
	 */
	Filesystem_Stream::InputStream OpenFile(const DirectoryTree::Args& args) const;

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
	virtual bool MakeDirectory(StringView dir, bool follow_symlinks) const;
	virtual bool IsFeatureSupported(Feature f) const;
	virtual std::string Describe() const = 0;
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

	/**
	 * Creates a new filesystem
	 *
	 * @param base_path Path the filesystem is rooted at
	 * @param parent_fs parent filesystem
	 */
	explicit Filesystem(std::string base_path, FilesystemView parent_fs);

	friend FilesystemView;
	friend DirectoryTree;
	friend RootFilesystem;

	std::unique_ptr<DirectoryTree> tree;

private:
	std::string base_path;
	std::unique_ptr<FilesystemView> parent_fs;
};

/**
 * A FilesystemView is a non-owning view at a subdirectory of the base Filesystem.
 * The Filesystem must stay valid.
 */
class FilesystemView {
public:
	/** Constructs an invalid view */
	FilesystemView() = default;

	/**
	 * @param tree Tree this view is based on
	 * @param sub_path Path relative to the tree
	 */
	FilesystemView(const std::shared_ptr<const Filesystem>& fs, std::string sub_path);

	/** @return The path of the owning filesystem, NOT of the view */
	std::string GetBasePath() const;

	/** @return The path of the view */
	std::string GetSubPath() const;

	/** @return The path of the owning filesystem followed by the view path,
	 *   same as MakePath(GetBasePath(), GetSubPath()) */
	std::string GetFullPath() const;

	/** @return The owning filesystem */
	const Filesystem& GetOwner() const;

	/**
	 * Clears the filesystem cache of the view. Changes in the filesystem become
	 * visible to the FindFile functions.
	 * This is automatically called when an output stream is closed.
	 */
	void ClearCache() const;

	/**
	 * Does a case insensitive search for the file.
	 *
	 * @param filename a path relative to the filesystem root
	 * @param exts List of file extensions to probe
	 * @return Path to file or empty string when not found
	 */
	std::string FindFile(StringView filename, const Span<const StringView> exts = {}) const;

	/**
	 * Does a case insensitive search for the file in a specific
	 * directory.
	 *
	 * @param directory a path relative to the filesystem root
	 * @param filename Name of the file to search
	 * @param exts List of file extensions to probe
	 * @return Path to file or empty string when not found
	 */
	std::string FindFile(StringView directory, StringView filename, const Span<const StringView> exts = {}) const;

	/**
	 * Does a case insensitive search for a file.
	 * Advanced version for special purposes searches. Usually not needed.
	 *
	 * @see DirectoryTree::Args
	 * @param args See documentation of DirectoryTree::Args
	 * @return Path to file or empty string when not found
	 */
	std::string FindFile(const DirectoryTree::Args& args) const;

	// Helper functions for finding and opening files
	/**
	 * Does a case insensitive search for the file and opens a read handle.
	 *
	 * @param filename a path relative to the filesystem root
	 * @param exts List of file extensions to probe
	 * @return Handle to the file if found, otherwise an invalid handle
	 */
	Filesystem_Stream::InputStream OpenFile(StringView filename, const Span<const StringView> exts = {}) const;

	/**
	 * Does a case insensitive search for the file in a specific directory
	 * and opens a read handle.
	 *
	 * @param directory a path relative to the filesystem root
	 * @param filename Name of the file to search
	 * @param exts List of file extensions to probe
	 * @return Handle to the file if found, otherwise an invalid handle
	 */
	Filesystem_Stream::InputStream OpenFile(StringView directory, StringView filename, const Span<const StringView> exts = {}) const;

	/**
	 * Does a case insensitive search for the file and opens a read handle.
	 * Advanced version for special purposes searches. Usually not needed.
	 *
	 * @see DirectoryTree::Args
	 * @param args See documentation of DirectoryTree::Args
	 * @return Handle to the file if found, otherwise an invalid handle
	 */
	Filesystem_Stream::InputStream OpenFile(const DirectoryTree::Args& args) const;


	/**
	 * @param subpath Path to append to the subtree root
	 * @return Combined path
	 */
	std::string MakePath(StringView subdir) const;

	/**
	 * @param path Path to check
	 * @return True when path is a regular file
	 */
	bool IsFile(StringView path) const;

	/**
	 * @param path Path to check
	 * @param follow_symlinks Whether to follow symlinks (if supported by this filesystem)
	 * @return True when path is a directory
	 */
	bool IsDirectory(StringView path, bool follow_symlinks) const;

	/**
	 * @param path Path to check
	 * @return True when a file exists at the path
	 */
	bool Exists(StringView path) const;

	/**
	 * @param path Path to check
	 * @return A filesize or -1 on error.
	 */
	int64_t GetFilesize(StringView path) const;

	/**
	 * Enumerates a directory.
	 *
	 * @param path Path to enumerate, empty for subtree root path
	 * @return list of directory entries or nullptr on failure
	 */
	DirectoryTree::DirectoryListType* ListDirectory(StringView path = "") const;

	/**
	 * Creates stream from filename for reading.
	 *
	 * @param name filename.
	 * @param m stream mode. Default is binary.
	 * @return A Stream. The stream is invalid when the open failed.
	 */
	Filesystem_Stream::InputStream OpenInputStream(StringView name,
		std::ios_base::openmode m = std::ios_base::in | std::ios_base::binary) const;

	/**
	 * Creates stream from filename for reading.
	 * When the file is missing it is created.
	 *
	 * @param name filename.
	 * @param m stream mode. Default is binary.
	 * @return A Stream. The stream is invalid when the open failed.
	 */
	Filesystem_Stream::InputStream OpenOrCreateInputStream(StringView name,
		std::ios_base::openmode m = std::ios_base::in | std::ios_base::binary) const;

	/**
	 * Creates stream from filename for writing.
	 *
	 * @param name filename.
	 * @param m stream mode. Default is binary.
	 * @return A Stream. The stream is invalid when the open failed or write is not supported.
	 */
	Filesystem_Stream::OutputStream OpenOutputStream(StringView name,
		std::ios_base::openmode m = std::ios_base::out | std::ios_base::binary) const;

	/**
	 * Opens a streambuffer from filename for reading.
	 * This is an internal function. Use OpenInputStream instead.
	 *
	 * @see OpenInputStream
	 * @param path filename.
	 * @param mode stream mode.
	 * @return A Stream. The stream is invalid when the open failed.
	 */
	std::streambuf* CreateInputStreambuffer(StringView path, std::ios_base::openmode mode) const;

	/**
	 * Creates stream from filename for writing.
	 * This is an internal function. Use OpenInputStream instead.
	 *
	 * @see OpenOutputStream
	 * @param path filename.
	 * @param mode stream mode.
	 * @return A Stream. The stream is invalid when the open failed or write is not supported.
	 */
	std::streambuf* CreateOutputStreambuffer(StringView path, std::ios_base::openmode mode) const;

	/**
	 * Creates a new appropriate filesystem from the specified path.
	 * The path is processed to initialize the proper virtual filesystem handler.
	 *
	 * @param p Virtual path to use
	 * @return Valid Filesystem when the parsing was successful, otherwise invalid
	 */
	FilesystemView Create(StringView p) const;

	/**
	 * Recursively creates a new directory.
	 * Not all filesystems support directory creation.
	 *
	 * @param dir Directory to create.
	 * @param follow_symlinks Whether to follow symlinks (if supported by this filesystem)
	 * @return true when the path was created
	 */
	bool MakeDirectory(StringView dir, bool follow_symlinks) const;

	/**
	 * @param f Filesystem feature to check
	 * @return true when the feature is supported.
	 */
	bool IsFeatureSupported(Filesystem::Feature f) const;

	/**
	 * Creates a new view on the subtree that is rooted at the sub_path.
	 *
	 * @param sub_path root of the subtree
	 * @return subtree rooted at sub_path
	 */
	FilesystemView Subtree(StringView sub_path) const;

	/** @return human readable representation of this filesystem for debug purposes */
	std::string Describe() const;

	/** @return true when the subtree points at a readable directory */
	explicit operator bool() const noexcept;

	friend DirectoryTree;

private:
	std::shared_ptr<const Filesystem> fs;
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
