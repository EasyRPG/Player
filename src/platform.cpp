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
#include "utils.h"
#include <cassert>
#include <utility>

Platform::File::File(std::string name) :
#ifdef _WIN32
		filename(Utils::ToWideString(name))
#else
		filename(std::move(name))
#endif
{
	// no-op
}

bool Platform::File::Exists() const {
#ifdef _WIN32
	return ::GetFileAttributesW(filename.c_str()) != (DWORD)-1;
#elif (defined(GEKKO) || defined(_3DS) || defined(__SWITCH__))
	struct stat sb;
	return ::stat(filename.c_str(), &sb) == 0;
#elif defined(PSP2)
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
#elif defined(PSP2)
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
#  if (defined(GEKKO) || defined(_3DS) || defined(__SWITCH__))
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
#elif defined(PSP2)
	struct SceIoStat sb = {};
	int result = ::sceIoGetstat(filename.c_str(), &sb);
	return (result >= 0) ? (int64_t)sb.st_size : (int64_t)-1;
#else
	struct stat sb = {};
	int result = ::stat(filename.c_str(), &sb);
	return (result == 0) ? (int64_t)sb.st_size : (int64_t)-1;
#endif
}

Platform::Directory::Directory(const std::string& name) {
#if defined(_WIN32)
	dir_handle = ::_wopendir(Utils::ToWideString(name).c_str());
#elif defined(PSP2)
	dir_handle = ::sceIoDopen(name.c_str());
#else
	dir_handle = ::opendir(name.c_str());
#endif
}

Platform::Directory::~Directory() {
	Close();
}

bool Platform::Directory::Read() {
#if defined(PSP2)
	assert(dir_handle >= 0);

	valid_entry = ::sceIoDread(dir_handle, &entry) > 0;
#else
	assert(dir_handle);

#	ifdef _WIN32
	entry = ::_wreaddir(dir_handle);
#	else
	entry = ::readdir(dir_handle);
#	endif

	valid_entry = entry != nullptr;
#endif

	return valid_entry;
}

std::string Platform::Directory::GetEntryName() const {
	assert(valid_entry);

#if defined(PSP2)
	return entry.d_name;
#elif defined(_WIN32)
	return Utils::FromWideString(entry->d_name);
#else
	return entry->d_name;
#endif
}

Platform::FileType Platform::Directory::GetEntryType() const {
	assert(valid_entry);

#if defined(PSP2)
	return SCE_S_ISREG(entry.d_stat.st_mode) ? FileType::File :
			SCE_S_ISDIR(entry.d_stat.st_mode) ? FileType::Directory : FileType::Other;
#else
#	if defined(_DIRENT_HAVE_D_TYPE) || defined(_3DS)
	return entry->d_type == DT_REG ? FileType::File :
		   entry->d_type == DT_DIR ? FileType::Directory :
		   entry->d_type == DT_UNKNOWN ? FileType::Unknown : FileType::Other;
#	else
	return FileType::Unknown;
#	endif
#endif
}

void Platform::Directory::Close() {
	if (*this) {
#if defined(_WIN32)
		::_wclosedir(dir_handle);
		dir_handle = nullptr;
#elif defined(PSP2)
		::sceIoDclose(dir_handle);
		dir_handle = -1;
#else
		::closedir(dir_handle);
		dir_handle = nullptr;
#endif
	}

	valid_entry = false;
}
