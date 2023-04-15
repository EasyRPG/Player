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

#ifndef EP_PLATFORM_H
#define EP_PLATFORM_H

// Headers
#include "system.h"
#include <cstdint>
#include <string>
#ifdef _WIN32
#  include <windows.h>
#else
#  ifdef __vita__
#    include <psp2/io/dirent.h>
#    include <psp2/io/stat.h>
#  else
#    include <dirent.h>
#    include <sys/stat.h>
#  endif
#  include <unistd.h>
#  include <sys/types.h>
#endif

/**
 * Provides abstractions for accessing operating system APIs.
 *
 * Don't use them directly, use FileFinder instead
 */
namespace Platform {
	enum class FileType {
		File,
		Directory,
		Other,
		Unknown
	};

	/** Wrapper around file access */
	class File {
	public:
		explicit File() = delete;
		File& operator=(const File&) = delete;
		File(const File&) = delete;

		/**
		 * Accesses a file.
		 *
		 * @param name File to access
		 */
		explicit File(std::string name);

		~File() = default;

		/** @return True when the file exists */
		bool Exists() const;

		/**
		 * @param follow_symlinks Whether to follow symlinks (if supported on this platform)
		 * @return true if it is a regular file
		 */
		bool IsFile(bool follow_symlinks) const;

		/**
		 * @param follow_symlinks Whether to follow symlinks (if supported on this platform)
		 * @return true if it is a directory
		 */
		bool IsDirectory(bool follow_symlinks) const;

		/**
		 * @param follow_symlinks Whether to follow symlinks (if supported on this platform)
		 * @return type of the file or FileType::Unknown on error
		 */
		FileType GetType(bool follow_symlinks) const;

		/** @return Filesize or -1 on error */
		int64_t GetSize() const;

		/**
		 * Creates a directory recursively at the filename path.
		 * @param follow_symlinks Whether to follow symlinks (if supported on this platform)
		 * @return true when the directory was created.
		 */
		bool MakeDirectory(bool follow_symlinks) const;

	private:
#ifdef _WIN32
		const std::wstring filename;
#else
		const std::string filename;
#endif
	};

	/** Wrapper around directory reading */
	class Directory {
	public:
		explicit Directory() = delete;
		Directory& operator=(const Directory&) = delete;
		Directory(const Directory&) = delete;

		/**
		 * Opens a directory for reading.
		 *
		 * @param name Directory to open
		 */
		explicit Directory(const std::string& name);
		~Directory();

		/**
		 * Reads one directory entry.
		 * Use GetEntryName/GetEntryType afterwards to retrieve information.
		 *
		 * @return true on success, false on failure or when end is reached.
		 */
		bool Read();

		/** @return Name of the last read entry */
		std::string GetEntryName() const;

		/** @return Type of the last read entry */
		FileType GetEntryType() const;

		/** Closes the directory */
		void Close();

		/** @return true if opening the directory was successful */
		explicit operator bool() const noexcept;

	private:
#if defined(_WIN32)
		HANDLE dir_handle = nullptr;
		WIN32_FIND_DATAW entry = {};
		bool first_entry = true;
#elif defined(__vita__)
		int dir_handle = -1;
		struct SceIoDirent entry = {};
#else
		DIR* dir_handle = nullptr;
		struct dirent* entry = nullptr;
#endif
		bool valid_entry = false;
	};

	inline Directory::operator bool() const noexcept {
#ifdef __vita__
		return dir_handle >= 0;
#else
		return dir_handle != nullptr;
#endif
	}
}

#endif
