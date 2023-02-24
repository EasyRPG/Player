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

#ifndef EP_FILESYSTEM_ROOT_H
#define EP_FILESYSTEM_ROOT_H

#include "filesystem.h"
#include "filesystem_native.h"

#include <memory>

/**
 * A virtual filesystem that is the root of the filesystem tree of the Player.
 * Contrary to other filesystems the root-fs does navigate into other VFS.
 * Non-prefixed path access forwards to the NativeFilesystem.
 * When namespaced (ns://) it is forwarded to the appropriate filesystem
 * that supports this namespace.
 * This ensures that code such as
 * - Exists("/home/user/some_file.txt")
 * - Exists("apk://assets/some_file.txt")
 * works as expected.
 * Non-prefixed path access forwards to the NativeFilesystem.
 * Paths with namespaces (ns://) are forwarded to the appropriate filesystem
 * that supports this namespace.
 */
class RootFilesystem : public Filesystem {
public:
	/**
	 * Initializes a Root filesystem.
	 * This filesystem does not support chaining, it is always the base.
	 */
	explicit RootFilesystem();

	/**
	 * Creates a new appropriate filesystem from the specified path.
	 * The path is processed to initialize the proper virtual filesystem handler.
	 * Enhanced version of Create with namespace support.
	 *
	 * @param p Virtual path to use
	 * @return Valid Filesystem when the parsing was successful, otherwise invalid
	 */
	FilesystemView Create(StringView path) const override;

protected:
	/**
 	 * Implementation of abstract methods
 	 */
	/** @{ */
	bool IsFile(StringView path) const override;
	bool IsDirectory(StringView path, bool follow_symlinks) const override;
	bool Exists(StringView path) const override;
	int64_t GetFilesize(StringView path) const override;
	std::streambuf* CreateInputStreambuffer(StringView path, std::ios_base::openmode mode) const override;
	std::streambuf* CreateOutputStreambuffer(StringView path, std::ios_base::openmode mode) const override;
	bool GetDirectoryContent(StringView path, std::vector<DirectoryTree::Entry>& entries) const override;
	bool MakeDirectory(StringView path, bool follow_symlinks) const override;
	std::string Describe() const override;
	/** @} */

private:
	/**
	 * Resolves namespaces to get the appropriate filesystem back.
	 * @param path Path to resolve.
	 * @return Filesystem that supports this path
	 */
	const Filesystem& FilesystemForPath(StringView path) const;

	// ns -> fs, NativeFilesystem is always last
	using FsList = std::vector<std::pair<std::string, std::shared_ptr<Filesystem>>>;
	FsList fs_list;
};

#endif
