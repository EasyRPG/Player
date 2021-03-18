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

#include "filesystem_zip.h"
#include "filefinder.h"
#include "output.h"
#include "utils.h"

#include <zlib.h>
#include <lcf/reader_util.h>
#include <iostream>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <fmt/core.h>

constexpr uint32_t end_of_central_directory = 0x06054b50;
constexpr int32_t end_of_central_directory_size = 22;

constexpr uint32_t central_directory_entry = 0x02014b50;
constexpr uint32_t local_header = 0x04034b50;
constexpr uint32_t local_header_size = 30;

static std::string normalize_path(StringView path) {
	if (path == "." || path == "/" || path == "") {
		return "";
	};
	std::string inner_path = FileFinder::MakeCanonical(path, 1);
	std::replace(inner_path.begin(), inner_path.end(), '\\', '/');
	if (inner_path.front() == '.') {
		inner_path = inner_path.substr(1, inner_path.size() - 1);
	}
	if (inner_path.front() == '/') {
		inner_path = inner_path.substr(1, inner_path.size() - 1);
	}
	return inner_path;
}

ZipFilesystem::ZipFilesystem(std::string base_path, FilesystemView parent_fs, StringView enc) :
	Filesystem(base_path, parent_fs) {
	auto zipfile = parent_fs.OpenInputStream(GetPath());
	if (!zipfile) {
		return;
	}

	uint16_t central_directory_entries = 0;
	uint32_t central_directory_size = 0;
	uint32_t central_directory_offset = 0;

	ZipEntry entry;
	std::vector<char> filepath_arr;
	std::string filepath;

	encoding = ToString(enc);
	if (FindCentralDirectory(zipfile, central_directory_offset, central_directory_size, central_directory_entries)) {
		if (encoding.empty()) {
			zipfile.seekg(central_directory_offset);
			std::stringstream filename_guess;

			// Guess the encoding first
			int items = 0;
			while (ReadCentralDirectoryEntry(zipfile, filepath_arr, entry.fileoffset, entry.filesize)) {
				if (items == 100) {
					break;
				}

				filepath = filepath_arr.data();

				// check if the entry is an directory or not (indicated by trailing /)
				// skip if it is, only filenames are usually non-ASCII
				if (filepath.back() == '/') {
					continue;
				}

				filename_guess << filepath;
				++items;
			}

			std::vector<std::string> encodings = lcf::ReaderUtil::DetectEncodings(filename_guess.str());
			for (auto enc : encodings) {
				std::string enc_test = lcf::ReaderUtil::Recode("\\", enc);
				if (enc_test.empty()) {
					// Bad encoding
					Output::Debug("Bad encoding: {}. Trying next.", enc);
					continue;
				}
				encoding = enc;
				break;
			}
			Output::Debug("Detected ZIP encoding: {}", encoding);
		}

		zipfile.clear();
		zipfile.seekg(central_directory_offset);
		while (ReadCentralDirectoryEntry(zipfile, filepath_arr, entry.fileoffset, entry.filesize)) {
			filepath = filepath_arr.data();
			filepath = lcf::ReaderUtil::Recode(filepath, encoding);
			// check if the entry is an directory or not (indicated by trailing /)
			if (filepath.back() == '/') {
				entry.is_directory = true;
				filepath = filepath.substr(0, filepath.size() - 1);
			}
			else {
				entry.is_directory = false;
			}

			zip_entries.insert(std::pair<std::string, ZipEntry>(filepath, entry));
		}

		// Insert root path into zip_entries
		entry.is_directory = true;
		entry.fileoffset = 0;
		entry.filesize = 0;
		zip_entries.insert(std::pair<std::string, ZipEntry>("", entry));
	} else {
		Output::Warning("ZipFS: {} is not a valid archive", GetPath());
	}
}

bool ZipFilesystem::FindCentralDirectory(std::istream& zipfile, uint32_t& offset, uint32_t& size, uint16_t& num_entries) {
	uint32_t magic = 0;
	bool found = false;

	// seek to the first position where the end_of_central_directory Signature may occur
	zipfile.seekg(-end_of_central_directory_size, std::ios_base::end);

	// The only variable length field in the end of central directory is the comment which has a maximum length of UINT16_MAX - so if we seek longer, this is no zip file
	for (size_t i = 0; i < UINT16_MAX && zipfile.good() && !found; i++) {
		zipfile.read(reinterpret_cast<char*>(&magic), sizeof(magic));
		Utils::SwapByteOrder(magic); // Take care of big endian systems
		if (magic == end_of_central_directory) {
			found = true;
		}
		else {
			// if not yet found the magic number step one byte back in the file
			zipfile.seekg(-(sizeof(magic) + 1), std::ios_base::cur);
		}
	}

	if (found) {
		zipfile.seekg(6, std::ios_base::cur); // Jump over multiarchive related fields
		zipfile.read(reinterpret_cast<char*>(&num_entries), sizeof(uint16_t));
		Utils::SwapByteOrder(num_entries);
		zipfile.read(reinterpret_cast<char*>(&size), sizeof(uint32_t));
		Utils::SwapByteOrder(size);
		zipfile.read(reinterpret_cast<char*>(&offset), sizeof(uint32_t));
		Utils::SwapByteOrder(offset);
		return true;
	}
	else {
		return false;
	}
}

bool ZipFilesystem::ReadCentralDirectoryEntry(std::istream& zipfile, std::vector<char>& filename, uint32_t& offset, uint32_t& uncompressed_size) {
	uint32_t magic = 0;
	uint16_t filepath_length;
	uint16_t extra_field_length;
	uint16_t comment_length;

	zipfile.read(reinterpret_cast<char*>(&magic), sizeof(magic));
	Utils::SwapByteOrder(magic); // Take care of big endian systems
	if (magic != central_directory_entry) return false;
	zipfile.seekg(20, std::ios_base::cur); // Jump over currently not needed entries
	zipfile.read(reinterpret_cast<char*>(&uncompressed_size), sizeof(uint32_t));
	Utils::SwapByteOrder(uncompressed_size);
	zipfile.read(reinterpret_cast<char*>(&filepath_length), sizeof(uint16_t));
	Utils::SwapByteOrder(filepath_length);
	zipfile.read(reinterpret_cast<char*>(&extra_field_length), sizeof(uint16_t));
	Utils::SwapByteOrder(extra_field_length);
	zipfile.read(reinterpret_cast<char*>(&comment_length), sizeof(uint16_t));
	Utils::SwapByteOrder(comment_length);
	zipfile.seekg(8, std::ios_base::cur); // Jump over currently not needed entries
	zipfile.read(reinterpret_cast<char*>(&offset), sizeof(uint32_t));
	Utils::SwapByteOrder(offset);
	if (filename.capacity() < filepath_length + 1) {
		filename.resize(filepath_length + 1);
	}
	filename.data()[filepath_length] = '\0';
	zipfile.read(reinterpret_cast<char*>(filename.data()), filepath_length);
	zipfile.seekg(comment_length + extra_field_length, std::ios_base::cur); // Jump over currently not needed entries
	std::string a = filename.data();
	return true;
}

bool ZipFilesystem::ReadLocalHeader(std::istream & zipfile, uint32_t & offset, StorageMethod & method, uint32_t & compressedSize) {
	uint32_t magic = 0;
	uint16_t filepath_length;
	uint16_t extra_field_length;
	uint16_t flags;
	uint16_t compression;

	zipfile.read(reinterpret_cast<char*>(&magic), sizeof(magic));
	Utils::SwapByteOrder(magic); // Take care of big endian systems
	if (magic != local_header) return false;
	zipfile.seekg(2, std::ios_base::cur); // Jump over currently not needed entries
	zipfile.read(reinterpret_cast<char*>(&flags), sizeof(uint16_t));
	Utils::SwapByteOrder(flags);
	zipfile.read(reinterpret_cast<char*>(&compression), sizeof(uint16_t));
	Utils::SwapByteOrder(compression);
	zipfile.seekg(8, std::ios_base::cur); // Jump over currently not needed entries
	zipfile.read(reinterpret_cast<char*>(&compressedSize), sizeof(uint32_t));
	Utils::SwapByteOrder(compressedSize);
	zipfile.seekg(4, std::ios_base::cur); // Jump over currently not needed entries
	zipfile.read(reinterpret_cast<char*>(&filepath_length), sizeof(uint16_t));
	Utils::SwapByteOrder(filepath_length);
	zipfile.read(reinterpret_cast<char*>(&extra_field_length), sizeof(uint16_t));
	Utils::SwapByteOrder(extra_field_length);

	switch (compression) {
	case 0:
		method = StorageMethod::Plain;
		break;
	case 8:
		method = StorageMethod::Deflate;
		break;
	default:
		method = StorageMethod::Unknown;
		break;
	}
	offset = local_header_size + filepath_length + extra_field_length;
	return true;
}

bool ZipFilesystem::IsFile(StringView path) const {
	std::string path_normalized = normalize_path(path);
	auto it = zip_entries.find(path_normalized);
	if (it != zip_entries.end()) {
		return !it->second.is_directory;
	}
	return false;
}

bool ZipFilesystem::IsDirectory(StringView path, bool) const {
	std::string path_normalized = normalize_path(path);
	auto it = zip_entries.find(path_normalized);
	if (it != zip_entries.end()) {
		return it->second.is_directory;
	}
	return false;
}

bool ZipFilesystem::Exists(StringView path) const {
	std::string path_normalized = normalize_path(path);
	auto it = zip_entries.find(path_normalized);
	return (it != zip_entries.end());
}

int64_t ZipFilesystem::GetFilesize(StringView path) const {
	std::string path_normalized = normalize_path(path);

	auto it = zip_entries.find(path_normalized);
	if (it != zip_entries.end()) {
		return it->second.filesize;
	}
	return 0;
}

std::streambuf* ZipFilesystem::CreateInputStreambuffer(StringView path, std::ios_base::openmode) const {
	std::string path_normalized = normalize_path(path);

	auto it = zip_entries.find(path_normalized);
	if (it != zip_entries.end() && !it->second.is_directory) {
		auto pool_it = input_pool.find(path_normalized);
		if (pool_it != input_pool.end()) {
			return new Filesystem_Stream::InputMemoryStreamBuf(pool_it->second);
		}

		auto zip_file = GetParent().OpenInputStream(GetPath());
		zip_file.seekg(it->second.fileoffset);
		StorageMethod method;
		uint32_t local_offset = 0;
		uint32_t compressed_size = 0;
		if (ReadLocalHeader(zip_file, local_offset, method, compressed_size)) {
			zip_file.seekg(it->second.fileoffset + local_offset);
			if (method == StorageMethod::Plain) {
				auto data = std::vector<uint8_t>(it->second.filesize);
				zip_file.read(reinterpret_cast<char*>(data.data()), data.size());
				input_pool[path_normalized] = std::move(data);
				return new Filesystem_Stream::InputMemoryStreamBuf(input_pool[path_normalized]);
			} else if (method == StorageMethod::Deflate) {
				std::vector<uint8_t> comp_buf;
				comp_buf.resize(compressed_size);
				zip_file.read(reinterpret_cast<char*>(comp_buf.data()), comp_buf.size());
				auto dec_buf = std::vector<uint8_t>(it->second.filesize);
				z_stream zlib_stream = {};
				zlib_stream.next_in = reinterpret_cast<Bytef*>(comp_buf.data());
				zlib_stream.avail_in = comp_buf.size();
				zlib_stream.next_out = reinterpret_cast<Bytef*>(dec_buf.data());
				zlib_stream.avail_out = dec_buf.size();
				inflateInit2(&zlib_stream, -MAX_WBITS);

				int zlib_error = inflate(&zlib_stream, Z_NO_FLUSH);
				if (zlib_error == Z_OK) {
					Output::Warning("ZipFS: zlib failed for {}: More data available (Archive corrupted?)", path_normalized);
					return nullptr;
				}
				else if (zlib_error != Z_STREAM_END) {
					Output::Warning("ZipFS: zlib failed for {}: {}", path_normalized, zlib_stream.msg);
					return nullptr;
				}
				input_pool[path_normalized] = std::move(dec_buf);
				return new Filesystem_Stream::InputMemoryStreamBuf(input_pool[path_normalized]);
			} else {
				Output::Warning("ZipFS: {} has unsupported compression format. Only Deflate is supported", path_normalized);
				return nullptr;
			}
		}
	}
	return nullptr;
}

bool ZipFilesystem::GetDirectoryContent(StringView path, std::vector<DirectoryTree::Entry>& entries) const {
	if (!IsDirectory(path, false)) {
		return false;
	}

	std::string path_normalized = normalize_path(path);
	if (path_normalized.size() != 0 && path_normalized.back() != '/') {
		path_normalized += "/";
	}

	for (const auto& it : zip_entries) {
		if (StringView(it.first).starts_with(path_normalized) &&
			it.first.substr(path_normalized.size(), it.first.size() - path_normalized.size()).find_last_of('/') == std::string::npos) {
			// Everything that starts with the path but isn't the path and does contain no slash
			entries.emplace_back(
				it.first.substr(path_normalized.size(), it.first.size() - path_normalized.size()),
				it.second.is_directory ? DirectoryTree::FileType::Directory : DirectoryTree::FileType::Regular);
		}
	}

	return true;
}

std::string ZipFilesystem::Describe() const {
	return fmt::format("[Zip] {} ({})", GetPath(), encoding);
}
