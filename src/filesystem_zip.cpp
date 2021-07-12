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

	ZipEntry entry = {};
	entry.is_directory = false;
	std::string filepath;
	std::string filepath_cp437;
	bool is_utf8;

	encoding = ToString(enc);
	if (FindCentralDirectory(zipfile, central_directory_offset, central_directory_size, central_directory_entries)) {
		if (encoding.empty()) {
			zipfile.seekg(central_directory_offset);
			std::stringstream filename_guess;

			// Guess the encoding first
			int items = 0;
			while (ReadCentralDirectoryEntry(zipfile, filepath, entry.fileoffset, entry.filesize, is_utf8)) {
				// Only consider Non-ASCII & Non-UTF8 for encoding detection
				// Skip directories, files already contain the paths
				if (is_utf8 || filepath.back() == '/' || Utils::StringIsAscii(filepath)) {
					continue;
				}
				// Codepath will be only entered by Windows "compressed folder" ZIPs (uses local encoding) and
				// 7zip (uses CP932 for Western European filenames)

				auto pos = filepath.find_last_of('/');
				if (pos == std::string::npos) {
					filename_guess << filepath;
				} else {
					filename_guess << filepath.substr(pos + 1);
				}

				++items;

				if (items == 10) {
					break;
				}
			}

			if (items == 0) {
				// Only ASCII or UTF-8 flags set
				encoding = "UTF-8";
			} else {
				std::vector<std::string> encodings = lcf::ReaderUtil::DetectEncodings(filename_guess.str());
				for (const auto &enc_ : encodings) {
					std::string enc_test = lcf::ReaderUtil::Recode("\\", enc_);
					if (enc_test.empty()) {
						// Bad encoding
						Output::Debug("Bad encoding: {}. Trying next.", enc_);
						continue;
					}
					encoding = enc_;
					break;
				}
			}
			Output::Debug("Detected ZIP encoding: {}", encoding);
		}
		bool enc_is_utf8 = encoding == "UTF-8";

		zipfile.clear();
		zipfile.seekg(central_directory_offset);

		std::vector<std::string> paths;
		while (ReadCentralDirectoryEntry(zipfile, filepath, entry.fileoffset, entry.filesize, is_utf8)) {
			if (is_utf8 || enc_is_utf8 || Utils::StringIsAscii(filepath)) {
				// No reencoding necessary
				filepath_cp437.clear();
			} else {
				// also store CP437 to ensure files inside 7zip zip archives are found
				filepath_cp437 = lcf::ReaderUtil::Recode(filepath, "437");
				filepath = lcf::ReaderUtil::Recode(filepath, encoding);
			}

			// check if the entry is an directory or not (indicated by trailing /)
			// this will fail when the (game) directory has cp437, but the users can rename it before
			if (filepath.back() == '/') {
				filepath = filepath.substr(0, filepath.size() - 1);

				// Determine intermediate directories
				while (!filepath.empty()) {
					paths.push_back(filepath);
					filepath = std::get<0>(FileFinder::GetPathAndFilename(filepath));
				}
			} else {
				zip_entries.emplace_back(filepath, entry);
				if (!filepath_cp437.empty()) {
					zip_entries_cp437.emplace_back(filepath_cp437, entry);
				}
			}
		}
		// Build directories
		entry.fileoffset = 0;
		entry.filesize = 0;
		entry.is_directory = true;

		// add root path
		paths.emplace_back("");

		std::sort(paths.begin(), paths.end());
		auto del = std::unique(paths.begin(), paths.end());
		paths.erase(del, paths.end());
		for (const auto& e : paths) {
			zip_entries.emplace_back(e, entry);
		}

		std::sort(zip_entries.begin(), zip_entries.end(), [](auto& a, auto& b) {
			return a.first < b.first;
		});
		std::sort(zip_entries_cp437.begin(), zip_entries_cp437.end(), [](auto& a, auto& b) {
			return a.first < b.first;
		});
	} else {
		Output::Warning("ZipFS: {} is not a valid archive", GetPath());
	}
}

bool ZipFilesystem::FindCentralDirectory(std::istream& zipfile, uint32_t& offset, uint32_t& size, uint16_t& num_entries) const {
	uint32_t magic = 0;
	bool found = false;

	// seek to the first position where the end_of_central_directory Signature may occur
	zipfile.seekg(-end_of_central_directory_size, std::ios_base::end);

	// The only variable length field in the end of central directory is the comment which
	// has a maximum length of UINT16_MAX - so if we seek longer, this is no zip file
	for (size_t i = 0; i < UINT16_MAX && zipfile.good() && !found; i++) {
		zipfile.read(reinterpret_cast<char*>(&magic), sizeof(magic));
		Utils::SwapByteOrder(magic); // Take care of big endian systems
		if (magic == end_of_central_directory) {
			found = true;
		}
		else {
			// if not yet found the magic number step one byte back in the file
			zipfile.seekg(-(static_cast<int>(sizeof(magic)) + 1), std::ios_base::cur);
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

bool ZipFilesystem::ReadCentralDirectoryEntry(std::istream& zipfile, std::string& filename, uint32_t& offset, uint32_t& uncompressed_size, bool& is_utf8) const {
	uint32_t magic = 0;
	uint16_t flags;
	uint16_t filepath_length;
	uint16_t extra_field_length;
	uint16_t comment_length;

	zipfile.read(reinterpret_cast<char*>(&magic), sizeof(magic));
	Utils::SwapByteOrder(magic); // Take care of big endian systems
	if (magic != central_directory_entry) return false;
	zipfile.seekg(4, std::ios_base::cur); // Jump over currently not needed entries
	zipfile.read(reinterpret_cast<char*>(&flags), sizeof(uint16_t));
	Utils::SwapByteOrder(flags);
	is_utf8 = (flags & 0x800) == 0x800;
	zipfile.seekg(14, std::ios_base::cur); // Jump over currently not needed entries
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
	if (filename_buffer.capacity() < filepath_length + 1) {
		filename_buffer.resize(filepath_length + 1);
	}
	zipfile.read(reinterpret_cast<char*>(filename_buffer.data()), filepath_length);
	filename = std::string(filename_buffer.data(), filepath_length);
	zipfile.seekg(comment_length + extra_field_length, std::ios_base::cur); // Jump over currently not needed entries
	return true;
}

bool ZipFilesystem::ReadLocalHeader(std::istream& zipfile, uint32_t& offset, StorageMethod& method, uint32_t& compressedSize) const {
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
	auto entry = Find(path);
	if (entry) {
		return !entry->is_directory;
	}
	return false;
}

bool ZipFilesystem::IsDirectory(StringView path, bool) const {
	std::string path_normalized = normalize_path(path);
	auto entry = Find(path);
	if (entry) {
		return entry->is_directory;
	}
	return false;
}

bool ZipFilesystem::Exists(StringView path) const {
	std::string path_normalized = normalize_path(path);
	auto entry = Find(path);
	return entry != nullptr;
}

int64_t ZipFilesystem::GetFilesize(StringView path) const {
	std::string path_normalized = normalize_path(path);
	auto entry = Find(path);
	if (entry) {
		return entry->filesize;
	}
	return 0;
}

std::streambuf* ZipFilesystem::CreateInputStreambuffer(StringView path, std::ios_base::openmode) const {
	std::string path_normalized = normalize_path(path);
	auto entry = Find(path);
	if (entry && !entry->is_directory) {
		auto pool_it = input_pool.find(path_normalized);
		if (pool_it != input_pool.end()) {
			return new Filesystem_Stream::InputMemoryStreamBuf(pool_it->second);
		}

		auto zip_file = GetParent().OpenInputStream(GetPath());
		zip_file.seekg(entry->fileoffset);
		StorageMethod method;
		uint32_t local_offset = 0;
		uint32_t compressed_size = 0;
		if (ReadLocalHeader(zip_file, local_offset, method, compressed_size)) {
			zip_file.seekg(entry->fileoffset + local_offset);
			if (method == StorageMethod::Plain) {
				auto data = std::vector<uint8_t>(entry->filesize);
				zip_file.read(reinterpret_cast<char*>(data.data()), data.size());
				input_pool[path_normalized] = std::move(data);
				return new Filesystem_Stream::InputMemoryStreamBuf(input_pool[path_normalized]);
			} else if (method == StorageMethod::Deflate) {
				std::vector<uint8_t> comp_buf;
				comp_buf.resize(compressed_size);
				zip_file.read(reinterpret_cast<char*>(comp_buf.data()), comp_buf.size());
				auto dec_buf = std::vector<uint8_t>(entry->filesize);
				z_stream zlib_stream = {};
				zlib_stream.next_in = reinterpret_cast<Bytef*>(comp_buf.data());
				zlib_stream.avail_in = static_cast<uInt>(comp_buf.size());
				zlib_stream.next_out = reinterpret_cast<Bytef*>(dec_buf.data());
				zlib_stream.avail_out = static_cast<uInt>(dec_buf.size());
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
	if (!path_normalized.empty() && path_normalized.back() != '/') {
		path_normalized += "/";
	}

	auto check = [&](auto& it) {
		if (StringView(it.first).starts_with(path_normalized) &&
			it.first.substr(path_normalized.size(), it.first.size() - path_normalized.size()).find_last_of('/') == std::string::npos) {
			// Everything that starts with the path but isn't the path and does contain no slash
			auto filename = it.first.substr(path_normalized.size(), it.first.size() - path_normalized.size());
			if (filename.empty()) {
				return;
			}

			entries.emplace_back(
					it.first.substr(path_normalized.size(), it.first.size() - path_normalized.size()),
					it.second.is_directory ? DirectoryTree::FileType::Directory : DirectoryTree::FileType::Regular);
		}
	};

	for (const auto& it : zip_entries) {
		check(it);
	}

	for (const auto& it : zip_entries_cp437) {
		check(it);
	}

	return true;
}

const ZipFilesystem::ZipEntry* ZipFilesystem::Find(StringView what) const {
	auto it = std::lower_bound(zip_entries.begin(), zip_entries.end(), what, [](const auto& e, const auto& w) {
		return e.first < w;
	});
	if (it != zip_entries.end() && it->first == what) {
		return &it->second;
	}

	it = std::lower_bound(zip_entries_cp437.begin(), zip_entries_cp437.end(), what, [](const auto& e, const auto& w) {
		return e.first < w;
	});
	if (it != zip_entries_cp437.end() && it->first == what) {
		return &it->second;
	}

	return nullptr;
}

std::string ZipFilesystem::Describe() const {
	return fmt::format("[Zip] {} ({})", GetPath(), encoding);
}
