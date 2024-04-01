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

#include "filesystem_drive.h"
#include "filefinder.h"
#include "output.h"

#include <algorithm>

#ifdef _WIN32
#  include <windows.h>
#  include <fileapi.h>
#endif

DriveFilesystem::DriveFilesystem() : Filesystem("", FilesystemView()) {
#ifdef _WIN32
	std::wstring volume = L"A:\\";

	DWORD logical_drives = GetLogicalDrives();
	for (int i = 0; i < 26; i++) {
		if ((logical_drives & (1 << i)) > 0) {
			DirectoryTree::Entry entry = { Utils::FromWideString(volume), DirectoryTree::FileType::Filesystem };

			wchar_t volume_name[MAX_PATH];
			if (GetVolumeInformation(volume.c_str(), volume_name, MAX_PATH, nullptr, nullptr, nullptr, nullptr, 0) != 0) {
				entry.human_name = fmt::format("{} ({})", Utils::FromWideString(volume), Utils::FromWideString(volume_name));
			}

			drives.push_back(entry);
		}
		volume[0]++; // Increment drive letter
	}
#endif
}

bool DriveFilesystem::HasDrives() const {
	return !drives.empty();
}

bool DriveFilesystem::IsFile(StringView path) const {
	(void)path;
	return false;
}

bool DriveFilesystem::IsDirectory(StringView path, bool) const {
	return path.empty();
}

bool DriveFilesystem::IsFilesystemNode(StringView path) const {
	for (const auto& drive: drives) {
		if (drive.name == path) {
			return true;
		}
#ifdef _WIN32
		if (drive.name == Utils::ReplaceAll(ToString(path), "/", "\\")) {
			return true;
		}
#endif
	}

	return false;
}

bool DriveFilesystem::Exists(StringView path) const {
	return IsDirectory(path, false) || IsFilesystemNode(path);
}

int64_t DriveFilesystem::GetFilesize(StringView path) const {
	(void)path;
	return 0;
}

FilesystemView DriveFilesystem::CreateFromNode(StringView path) const {
	if (!IsFilesystemNode(path)) {
		return {};
	}

	return FileFinder::Root().Create(path);
}

std::streambuf* DriveFilesystem::CreateInputStreambuffer(StringView path, std::ios_base::openmode mode) const {
	return nullptr;
}

bool DriveFilesystem::GetDirectoryContent(StringView path, std::vector<DirectoryTree::Entry>& tree) const {
	if (!path.empty()) {
		return false;
	}

	tree = drives;
	return true;
}

std::string DriveFilesystem::Describe() const {
	return "[Drive]";
}
