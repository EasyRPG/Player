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

#ifndef EP_FILESYSTEM_DRIVE_H
#define EP_FILESYSTEM_DRIVE_H

#include "filesystem.h"

/**
 * A virtual filesystem that lists e.g. drive letters on Windows
 */
class DriveFilesystem : public Filesystem {
public:
	/**
	 * Initializes a OS Filesystem on the given os path
	 */
	explicit DriveFilesystem();

	/** @return Whether the current target platform has drive letters to list */
	bool HasDrives() const;

protected:
	/**
 	 * Implementation of abstract methods
 	 */
	/** @{ */
	bool IsFile(StringView path) const override;
	bool IsDirectory(StringView path, bool follow_symlinks) const override;
	bool IsFilesystemNode(StringView path) const override;
	bool Exists(StringView path) const override;
	int64_t GetFilesize(StringView path) const override;
	FilesystemView CreateFromNode(StringView path) const override;
	std::streambuf* CreateInputStreambuffer(StringView path, std::ios_base::openmode mode) const override;
	bool GetDirectoryContent(StringView path, std::vector<DirectoryTree::Entry>& entries) const override;
	std::string Describe() const override;
	/** @} */

private:
	std::vector<DirectoryTree::Entry> drives;
};

#endif
