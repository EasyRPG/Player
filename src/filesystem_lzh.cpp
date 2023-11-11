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

#include "system.h"

#ifdef HAVE_LHASA

#include "filesystem_lzh.h"
#include "filefinder.h"
#include "output.h"
#include "utils.h"

#include <lcf/encoder.h>
#include <lcf/reader_util.h>
#include <lcf/scope_guard.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <fmt/core.h>

#include "lhasa.h"

static std::string normalize_path(StringView path) {
	if (path == "." || path == "/" || path.empty()) {
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

static int vio_read_func(void* handle, void* buf, size_t buf_len) {
	auto* f = reinterpret_cast<Filesystem_Stream::InputStream*>(handle);
	if (buf_len == 0) return 0;
	return f->read(reinterpret_cast<char*>(buf), buf_len).gcount();
}

static int vio_skip_func(void* handle, size_t bytes) {
	auto* f = reinterpret_cast<Filesystem_Stream::InputStream*>(handle);
	f->seekg(bytes, std::ios_base::cur);
	return 1;
}

static size_t vio_read_dec_func(void* buf, size_t buf_len, void* user_data) {
	auto* f = reinterpret_cast<Filesystem_Stream::InputStream*>(user_data);
	if (buf_len == 0) return 0;
	f->read(reinterpret_cast<char*>(buf), buf_len);
	return f->gcount();
}

static LHAInputStreamType vio = {
	vio_read_func,
	vio_skip_func,
	nullptr // close not supported by istream interface
};

LzhFilesystem::LzhFilesystem(std::string base_path, FilesystemView parent_fs, StringView enc) :
	Filesystem(base_path, parent_fs) {
	is = parent_fs.OpenInputStream(GetPath());
	if (!is) {
		return;
	}

	lha_is.reset(lha_input_stream_new(&vio, &is));

	lha_reader.reset(lha_reader_new(lha_is.get()));

	if (!lha_reader) {
		Output::Debug("LzhFS: {} is not a valid archive", GetPath());
		return;
	}

	encoding = ToString(enc);
	LHAFileHeader* header;

	LzhEntry entry;
	std::vector<std::string> paths;

	// Compressed data offset is manually calculated to reduce calls to tellg()
	auto last_offset = is.tellg();

	// Guess the encoding
	if (encoding.empty()) {
		std::stringstream filename_guess;
		int items = 0;

		while ((header = lha_reader_next_file(lha_reader.get())) != nullptr) {
			std::string filepath;

			// Only consider Non-ASCII and skip directories
			if (strcmp(header->compress_method, LHA_COMPRESS_TYPE_DIR) != 0) {
				filepath = header->filename;
				if (Utils::StringIsAscii(filepath)) {
					continue;
				}
				filename_guess << filepath;

				++items;
				if (items == 10) {
					break;
				}
			}
		}

		if (items == 0) {
			// Only ASCII text (UTF-8 compatible)
			encoding = "UTF-8";
		} else {
			std::vector<std::string> encodings = lcf::ReaderUtil::DetectEncodings(filename_guess.str());
			for (const auto &enc_ : encodings) {
				lcf::Encoder lcf_encoder(enc_);
				if (!lcf_encoder.IsOk()) {
					// Bad encoding
					Output::Debug("Bad encoding: {}. Trying next.", enc_);
					continue;
				}
				encoding = enc_;
				break;
			}
		}
		Output::Debug("Detected LZH encoding: {}", encoding);

		is.clear();
		is.seekg(last_offset);

		// Cannot figure out how to rewind the reader: Creating a new one instead
		lha_reader.reset(lha_reader_new(lha_is.get()));

		if (!lha_reader) {
			Output::Debug("LzhFS: {} is not a valid archive", GetPath());
			return;
		}
	}

	// Read the archive
	lcf::Encoder lzh_encoder(encoding);

	while ((header = lha_reader_next_file(lha_reader.get())) != nullptr) {
		std::string filepath;

		if (!strcmp(header->compress_method, LHA_COMPRESS_TYPE_DIR)) {
			last_offset += header->raw_data_len;

			filepath = header->path;
			lzh_encoder.Encode(filepath);
			if (filepath.back() == '/') {
				filepath.pop_back();
			}
			paths.push_back(filepath);
		} else {
			entry.uncompressed_size = header->length;
			entry.compressed_size = header->compressed_length;
			entry.fileoffset = last_offset + static_cast<std::streamoff>(header->raw_data_len);
			last_offset = entry.fileoffset + entry.compressed_size;

			entry.is_directory = false;
			entry.compress_method = header->compress_method;
			if (header->path != nullptr) {
				// File is not in the root
				filepath = header->path;
				lzh_encoder.Encode(filepath);

				// Safety check: Directories should end with a /
				if (filepath.back() == '/') {
					paths.push_back(filepath);
					paths.back().pop_back();
				} else {
					paths.push_back(filepath);
					filepath += '/';
				}
			}
			std::string fname = header->filename;
			lzh_encoder.Encode(fname);
			filepath += fname;

			lzh_entries.emplace_back(filepath, entry);
		}

		// Determine intermediate directories
		for (;;) {
			filepath = std::get<0>(FileFinder::GetPathAndFilename(filepath));
			if (filepath.empty()) {
				break;
			}
			paths.push_back(filepath);
		}
	}

	// Build directories
	entry = {};
	entry.is_directory = true;

	// add root path
	paths.emplace_back("");

	std::sort(paths.begin(), paths.end());
	auto paths_del_it = std::unique(paths.begin(), paths.end());
	paths.erase(paths_del_it, paths.end());
	for (const auto& e : paths) {
		lzh_entries.emplace_back(e, entry);
	}

	// entries can be duplicated in the archive, e.g. when creating a game disk the RTP is embedded, followed by
	// the game entries. Use a stable sort to preserve this order.
	std::stable_sort(lzh_entries.begin(), lzh_entries.end(), [](auto& a, auto& b) {
		return a.first < b.first;
	});

	// Then remove all duplicates but keep the last
	auto entries_del_it = std::unique(lzh_entries.rbegin(), lzh_entries.rend(), [](auto& a, auto& b) {
		return a.first == b.first;
	});
	lzh_entries.erase(lzh_entries.begin(), entries_del_it.base());
}

bool LzhFilesystem::IsFile(StringView path) const {
	std::string path_normalized = normalize_path(path);
	auto entry = Find(path);
	if (entry) {
		return !entry->is_directory;
	}
	return false;
}

bool LzhFilesystem::IsDirectory(StringView path, bool) const {
	std::string path_normalized = normalize_path(path);
	auto entry = Find(path);
	if (entry) {
		return entry->is_directory;
	}
	return false;
}

bool LzhFilesystem::Exists(StringView path) const {
	std::string path_normalized = normalize_path(path);
	auto entry = Find(path);
	return entry != nullptr;
}

int64_t LzhFilesystem::GetFilesize(StringView path) const {
	std::string path_normalized = normalize_path(path);
	auto entry = Find(path);
	if (entry) {
		return entry->uncompressed_size;
	}
	return 0;
}

std::streambuf* LzhFilesystem::CreateInputStreambuffer(StringView path, std::ios_base::openmode) const {
	std::string path_normalized = normalize_path(path);
	auto entry = Find(path);
	if (entry && !entry->is_directory) {
		// Determine compression method
		auto* decoder_type = lha_decoder_for_name(const_cast<char*>(entry->compress_method.c_str()));

		if (!decoder_type) {
			Output::Warning("LzhFS: Unsupported compression method {} for {}", entry->compress_method, path_normalized);
			return nullptr;
		}

		// Seek to the compressed data
		is.clear();
		is.seekg(entry->fileoffset, std::ios_base::beg);

		// Create a suitable decoder for the compression method
		std::unique_ptr<LHADecoder, LhasaDeleter> decoder;
		decoder.reset(lha_decoder_new(decoder_type, vio_read_dec_func, &is, entry->uncompressed_size));

		// Decompress
		auto dec_buf = std::vector<uint8_t>(entry->uncompressed_size);
		size_t res = lha_decoder_read(decoder.get(), dec_buf.data(), dec_buf.size());

		if (res != entry->uncompressed_size) {
			Output::Warning("LzhFS: Less data compressed than expected ({})", path_normalized);
			return nullptr;
		}

		return new Filesystem_Stream::InputMemoryStreamBuf(std::move(dec_buf));
	}

	return nullptr;
}

bool LzhFilesystem::GetDirectoryContent(StringView path, std::vector<DirectoryTree::Entry>& entries) const {
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

	for (const auto& it : lzh_entries) {
		check(it);
	}

	return true;
}

const LzhFilesystem::LzhEntry* LzhFilesystem::Find(StringView what) const {
	auto it = std::lower_bound(lzh_entries.begin(), lzh_entries.end(), what, [](const auto& e, const auto& w) {
		return e.first < w;
	});
	if (it != lzh_entries.end() && it->first == what) {
		return &it->second;
	}

	return nullptr;
}

std::string LzhFilesystem::Describe() const {
	return fmt::format("[LZH] {} ({})", GetPath(), encoding);
}

#endif
