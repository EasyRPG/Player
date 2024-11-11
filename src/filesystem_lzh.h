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

#ifndef EP_FILESYSTEM_LZH_H
#define EP_FILESYSTEM_LZH_H

#include "system.h"

#ifdef HAVE_LHASA

#include "filesystem.h"
#include "filesystem_stream.h"
#include <fstream>
#include <memory>
#include <unordered_map>
#include <vector>

#include <lhasa.h>

/**
 * A virtual filesystem that allows file/directory operations inside a LZH archive.
 */
class LzhFilesystem : public Filesystem {
public:
	/**
	 * Initializes a filesystem inside the given LZH File
	 *
	 * @param base_path Path passed to parent_fs to open the LZH file
	 * @param parent_fs Filesystem used to create handles on the LZH file
	 * @param encoding Encoding to use, use empty string for autodetection
	 */
	LzhFilesystem(std::string base_path, FilesystemView parent_fs, StringView encoding = "");

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
	bool GetDirectoryContent(StringView path, std::vector<DirectoryTree::Entry>& entries) const override;
	std::string Describe() const override;
	/** @} */

private:
	struct LzhEntry {
		size_t compressed_size;
		size_t uncompressed_size;
		std::streamoff fileoffset;
		std::string compress_method;
		bool is_directory;
	};

	const LzhEntry* Find(StringView what) const;

	std::vector<std::pair<std::string, LzhEntry>> lzh_entries;
	std::string encoding;
	mutable std::vector<char> filename_buffer;

	struct LhasaDeleter {
		void operator()(LHAInputStream* o) const {
			lha_input_stream_free(o);
		}

		void operator()(LHAReader* o) const {
			lha_reader_free(o);
		}

		void operator()(LHADecoder* o) const {
			lha_decoder_free(o);
		}
	};

	mutable Filesystem_Stream::InputStream is;
	mutable std::unique_ptr<LHAInputStream, LhasaDeleter> lha_is;
	mutable std::unique_ptr<LHAReader, LhasaDeleter> lha_reader;
};

#endif

#endif
