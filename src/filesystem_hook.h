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

#ifndef EP_FILESYSTEM_HOOK_H
#define EP_FILESYSTEM_HOOK_H

#include "filesystem.h"

/**
 * A virtual filesystem that applies modifications to game files
 */
class HookFilesystem : public Filesystem {
public:
	enum class Hook {
		// The Sacred Tears: TRUE
		// Shifts the offset of all bytes in the LMT back by one
		SacredTears
	};

	explicit HookFilesystem(FilesystemView parent_fs, Hook hook);

	static FilesystemView Detect(FilesystemView fs);

	/**
	 * Implementation of abstract methods
	 */
	/** @{ */
	bool IsFile(std::string_view path) const override;
	bool IsDirectory(std::string_view path, bool follow_symlinks) const override;
	bool Exists(std::string_view path) const override;
	int64_t GetFilesize(std::string_view path) const override;
	bool MakeDirectory(std::string_view dir, bool follow_symlinks) const override;
	bool IsFeatureSupported(Feature f) const override;
	std::string Describe() const override;
	/** @} */
protected:
	/** @{ */
	bool GetDirectoryContent(std::string_view path, std::vector<DirectoryTree::Entry>& entries) const override;
	std::streambuf* CreateInputStreambuffer(std::string_view path, std::ios_base::openmode mode) const override;
	std::streambuf* CreateOutputStreambuffer(std::string_view path, std::ios_base::openmode mode) const override;
	/** @} */

	Hook active_hook;
};

#endif
