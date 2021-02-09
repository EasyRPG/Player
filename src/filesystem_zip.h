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

#ifndef EP_FILESYSTEM_ZIP_H
#define EP_FILESYSTEM_ZIP_H

#include "filesystem.h"
#include <fstream>
#include <memory>
#include <unordered_map>
#include <vector>

/**
 * A virtual filesystem that allows file/directory operations inside a ZIP archive.
 */
class ZIPFilesystem : public Filesystem {
public:

	/**
	 * Initializes a filesystem inside the given ZIP File
	 *
	 * @param source_fs Filesystem used to create handles on the zip file
	 * @param base_path Path passed to source_fs to open the zip file
	 * @param encoding Encoding to use, use empty string for autodetection
	 */
	ZIPFilesystem(FilesystemView source_fs, std::string base_path, StringView encoding = "");

	~ZIPFilesystem();

protected:
	/**
 	 * Implementation of abstract methods
 	 */
	/** @{ */
	bool IsFileImpl(StringView path) const override;
	bool IsDirectoryImpl(StringView path, bool follow_symlinks) const override;
	bool ExistsImpl(StringView path) const override;
	int64_t GetFilesizeImpl(StringView path) const override;
	std::streambuf* CreateInputStreambufferImpl(StringView path, std::ios_base::openmode mode) const override;
	std::streambuf* CreateOutputStreambufferImpl(StringView path, std::ios_base::openmode mode) const override;
	bool GetDirectoryContentImpl(StringView path, std::vector<DirectoryTree::Entry>& entries) const override;
	/** @} */

private:
	enum class StorageMethod {Unknown,Plain,Deflate};
	struct ZipEntry {
		uint32_t filesize;
		uint32_t fileoffset;
		bool isDirectory;
	};

	struct StreamPoolEntry {
		Filesystem_Stream::InputStream stream;
		bool used;
	};
	class StorageIStreambuf;
	class DeflateIStreambuf;
	//No standard Constructor
	ZIPFilesystem() = delete;

	static bool FindCentralDirectory(std::istream & stream, uint32_t & offset, uint32_t & size, uint16_t & numberOfEntries);
	static bool ReadCentralDirectoryEntry(std::istream & zipfile, std::vector<char> & filepath, uint32_t & offset, uint32_t & uncompressed_size);
	static bool ReadLocalHeader(std::istream & zipfile, uint32_t & offset, StorageMethod & method,uint32_t & compressedSize);

	bool m_isValid;
	FilesystemView source_fs;
	std::string fs_path;
	mutable std::vector<StreamPoolEntry*> m_InputPool;
	std::unordered_map<std::string, ZipEntry> m_zipContent;
};

#endif
