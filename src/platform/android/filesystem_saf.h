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

#ifndef EP_FILESYSTEM_SAF_H
#define EP_FILESYSTEM_SAF_H

#include "filesystem.h"

#include <jni.h>

/**
 * A virtual filesystem for accessing the APK content of the current Android executable
 */
class SafFilesystem : public Filesystem {
public:
	/**
	 * Initializes a Storage Access Framework Filesystem.
	 */
	explicit SafFilesystem(std::string base_path, FilesystemView parent_fs);

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
	bool IsFeatureSupported(Feature f) const override;
	std::string Describe() const override;
	/** @} */

private:
	jobject jni_fs = 0;
};

#endif
