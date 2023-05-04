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

// Headers
#include "platform.h"
#include "filefinder.h"
#include "utils.h"
#include <cassert>
#include <utility>

#ifndef DT_UNKNOWN
#define DT_UNKNOWN 0
#endif
#ifndef DT_REG
#define DT_REG DT_UNKNOWN
#endif
#ifndef DT_DIR
#define DT_DIR DT_UNKNOWN
#endif

Platform::File::File(std::string name) :
#ifdef _WIN32
		filename(Utils::ToWideString(name.empty() ? "." : name))
#else
		filename(name.empty() ? "." : std::move(name))
#endif
{
	// no-op
}

bool Platform::File::Exists() const {
#ifdef _WIN32
	return ::GetFileAttributesW(filename.c_str()) != (DWORD)-1;
#elif defined(__vita__)
	struct SceIoStat sb;
	return (::sceIoGetstat(filename.c_str(), &sb) >= 0);
#else
	return ::access(filename.c_str(), F_OK) != -1;
#endif
}

bool Platform::File::IsFile(bool follow_symlinks) const {
	return GetType(follow_symlinks) == FileType::File;
}

bool Platform::File::IsDirectory(bool follow_symlinks) const {
	return GetType(follow_symlinks) == FileType::Directory;
}

Platform::FileType Platform::File::GetType(bool follow_symlinks) const {
#if defined(_WIN32)
	(void)follow_symlinks;
	int attribs = ::GetFileAttributesW(filename.c_str());

	if (attribs == INVALID_FILE_ATTRIBUTES) {
		return FileType::Unknown;
	} else if ((attribs & FILE_ATTRIBUTE_DIRECTORY) == 0) {
		return FileType::File;
	} else if ((attribs & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) == FILE_ATTRIBUTE_DIRECTORY) {
		return FileType::Directory;
	}
	return FileType::Other;
#elif defined(__vita__)
	(void)follow_symlinks;
	struct SceIoStat sb = {};
	if (::sceIoGetstat(filename.c_str(), &sb) >= 0) {
		return SCE_S_ISREG(sb.st_mode) ? FileType::File :
			SCE_S_ISDIR(sb.st_mode) ? FileType::Directory :
			FileType::Other;
	}
	return FileType::Unknown;
#else
	struct stat sb = {};
#  if defined(PLAYER_NINTENDO)
	// no symlink support (FAT)
	(void)follow_symlinks;
	auto fn = ::stat;
#  else
	auto fn = follow_symlinks ? ::stat : ::lstat;
#  endif
	if (fn(filename.c_str(), &sb) == 0) {
		return S_ISREG(sb.st_mode) ? FileType::File :
			S_ISDIR(sb.st_mode) ? FileType::Directory :
			FileType::Other;
	}
	return FileType::Unknown;
#endif
}

int64_t Platform::File::GetSize() const {
#if defined(_WIN32)
	WIN32_FILE_ATTRIBUTE_DATA data;
	BOOL res = ::GetFileAttributesExW(filename.c_str(),
			GetFileExInfoStandard,
			&data);
	if (!res) {
		return -1;
	}

	return ((int64_t)data.nFileSizeHigh << 32) | (int64_t)data.nFileSizeLow;
#elif defined(__vita__)
	struct SceIoStat sb = {};
	int result = ::sceIoGetstat(filename.c_str(), &sb);
	return (result >= 0) ? (int64_t)sb.st_size : (int64_t)-1;
#else
	struct stat sb = {};
	int result = ::stat(filename.c_str(), &sb);
	return (result == 0) ? (int64_t)sb.st_size : (int64_t)-1;
#endif
}

bool Platform::File::MakeDirectory(bool follow_symlinks) const {
	if (IsDirectory(follow_symlinks)) {
		return true;
	}

#ifdef _WIN32
	std::string path = Utils::FromWideString(filename);
#else
	std::string path = filename;
#endif

	auto components = FileFinder::SplitPath(path);
	std::string cur_path;
	if (StringView(path).starts_with("/")) {
		cur_path += "/";
	}

	bool first = true;
	for (const auto& comp : components) {
		if (comp.empty() || comp == ".") {
			continue;
		}

		cur_path = FileFinder::MakePath(cur_path, comp);

		if (first) {
			// Do not check stuff that looks like drives, such as C:, ux0: or sd:
			// Some systems do not consider them directories
			first = false;
			if (comp.back() == ':') {
				continue;
			}
		}

		File cf(cur_path);
		if (cf.IsDirectory(follow_symlinks)) {
			continue;
		} else if (cf.IsFile(follow_symlinks) || cf.Exists()) {
			return false;
		} else {
#ifdef _WIN32
			if (!CreateDirectoryW(Utils::ToWideString(cur_path).c_str(), nullptr)) {
				return false;
			}
#else
#  if defined(__vita__)
			int res = sceIoMkdir(cur_path.c_str(), 0777);
#  else
			int res = mkdir(cur_path.c_str(), 0777);
#  endif
			if (res < 0) {
				return false;
			}
#endif
		}
	}
	return true;
}

Platform::Directory::Directory(const std::string& name) {
#if defined(_WIN32)
	std::wstring wname = Utils::ToWideString((name.empty() ? "." : name) + "\\*");
	dir_handle = FindFirstFileW(wname.c_str(), &entry);
#elif defined(__vita__)
	dir_handle = ::sceIoDopen(name.empty() ? "." : name.c_str());
#else
	dir_handle = ::opendir(name.empty() ? "." : name.c_str());
#endif
}

Platform::Directory::~Directory() {
	Close();
}

bool Platform::Directory::Read() {
#if defined(__vita__)
	assert(dir_handle >= 0);

	valid_entry = ::sceIoDread(dir_handle, &entry) > 0;
#elif defined(_WIN32)
	assert(dir_handle != INVALID_HANDLE_VALUE);

	if (!first_entry) {
		valid_entry = FindNextFile(dir_handle, &entry) != 0;
	} else {
		valid_entry = true;
		first_entry = false;
	}
#else
	assert(dir_handle);

	entry = ::readdir(dir_handle);

	valid_entry = entry != nullptr;
#endif

	return valid_entry;
}

std::string Platform::Directory::GetEntryName() const {
	assert(valid_entry);

#if defined(__vita__)
	return entry.d_name;
#elif defined(_WIN32)
	return Utils::FromWideString(entry.cFileName);
#else
	return entry->d_name;
#endif
}

#if !defined(__vita__) && !defined(_WIN32)
[[maybe_unused]] static inline Platform::FileType GetEntryType(...) {
	return Platform::FileType::Unknown;
}

template <typename T, typename = decltype(std::declval<T>().d_type)>
static inline Platform::FileType GetEntryType(T* entry) {
	return entry->d_type == DT_REG ? Platform::FileType::File :
		   entry->d_type == DT_DIR ? Platform::FileType::Directory :
		   entry->d_type == DT_UNKNOWN ? Platform::FileType::Unknown : Platform::FileType::Other;
}
#endif

Platform::FileType Platform::Directory::GetEntryType() const {
	assert(valid_entry);

#if defined(__vita__)
	return SCE_S_ISREG(entry.d_stat.st_mode) ? FileType::File :
			SCE_S_ISDIR(entry.d_stat.st_mode) ? FileType::Directory : FileType::Other;
#elif defined(_WIN32)
	int attribs = entry.dwFileAttributes;
	if (attribs == INVALID_FILE_ATTRIBUTES) {
		return FileType::Unknown;
	} else if ((attribs & FILE_ATTRIBUTE_DIRECTORY) == 0) {
		return FileType::File;
	} else if ((attribs & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) == FILE_ATTRIBUTE_DIRECTORY) {
		return FileType::Directory;
	}
	return FileType::Other;
#else
	return ::GetEntryType(entry);
#endif
}

void Platform::Directory::Close() {
	if (*this) {
#if defined(_WIN32)
		FindClose(dir_handle);
		dir_handle = nullptr;
#elif defined(__vita__)
		::sceIoDclose(dir_handle);
		dir_handle = -1;
#else
		::closedir(dir_handle);
		dir_handle = nullptr;
#endif
	}

	valid_entry = false;
}
