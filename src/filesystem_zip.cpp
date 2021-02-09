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

static const uint32_t endOfCentralDirectory = 0x06054b50;
static const int32_t endOfCentralDirectorySize = 22;

static const uint32_t centralDirectoryEntry = 0x02014b50;
static const uint32_t localHeader = 0x04034b50;
static const uint32_t localHeaderSize = 30;

// Streambuffer for storage method
class ZIPFilesystem::StorageIStreambuf : public std::streambuf {
	public:
		StorageIStreambuf(StreamPoolEntry *backingStream, uint32_t fileoffset, uint32_t filelength):
		m_fileoffset(fileoffset),m_filelength(filelength),m_buffer(bufsize),m_remaining(filelength){
			assert(backingStream);
			assert(!backingStream->used);
			// Use the provided buffer
			m_backingStream = backingStream;
			m_backingStream->used = true;
			// seek to the beginning of the file
			m_backingStream->stream.rdbuf()->pubseekpos(fileoffset, std::ios_base::in);
			// the buffer is empty at the start
			setg(&m_buffer[0], &m_buffer[0] + bufsize, &m_buffer[0] + bufsize);
		}
		virtual ~StorageIStreambuf() {
			// unuse the provided buffer (free for another zipstream to use)
			m_backingStream->used = false;
		}
		StorageIStreambuf(StorageIStreambuf const& other) = delete;
		StorageIStreambuf const& operator=(StorageIStreambuf const& other) = delete;

protected:

	virtual std::streambuf::int_type  underflow() override {
		// either fill the buffer full, or with what is remaining
		size_t numberOfBytes = bufsize;
		if (m_remaining < bufsize) {
			numberOfBytes = m_remaining;
		}
		size_t bytesRead = m_backingStream->stream.rdbuf()->sgetn(&m_buffer[0], numberOfBytes);
		if (bytesRead > 0) {
			// Now there are some bytes less remaining
			m_remaining = m_remaining - bytesRead;
			// And reset the bufferpositions
			setg(&m_buffer[0], &m_buffer[0], &m_buffer[0] + bytesRead);
			return traits_type::to_int_type(*gptr());
		}
		else {
			return traits_type::eof();
		}
	}

	virtual std::streambuf::pos_type seekoff(std::streambuf::off_type offset, std::ios_base::seekdir way, std::ios_base::openmode mode) override{
		std::streambuf::off_type position = 0;
		switch (way) {
			case std::ios_base::cur:
				position = (m_filelength - (m_remaining + (egptr() - gptr()))) + offset;
				break;
			case std::ios_base::beg:
				position = offset;
				break;
			case std::ios_base::end:
				position = m_filelength + offset;
				break;
			default:
				// Fixes clang warning "enumeration value '_S_ios_seekdir_end' not handled in switch"
				assert(false);
				break;
		}
		if (position < 0) {
			position = 0;
		}
		else if (position >m_filelength) {
			position = m_filelength;
		}

		return seekpos(position, mode);
	}

	virtual std::streambuf::pos_type seekpos(std::streambuf::pos_type pos,  std::ios_base::openmode mode) override {
		std::streambuf::pos_type position = m_fileoffset+pos;

		if (position < m_fileoffset) {
			position = m_fileoffset;
		}
		else if (position > m_fileoffset + m_filelength) {
			position = m_fileoffset + m_filelength;
		}

		m_remaining = (m_fileoffset + m_filelength) - position;
		// empty buffer
		setg(&m_buffer[0], &m_buffer[0] + bufsize, &m_buffer[0] + bufsize);

		return m_backingStream->stream.rdbuf()->pubseekpos(position, mode)- m_fileoffset;
	}

	StreamPoolEntry *m_backingStream;
	std::streampos m_fileoffset;
	std::streamoff m_filelength;
private:
	static const int bufsize = 128;
	std::vector<char> m_buffer;
	std::streamoff m_remaining;

};


// Streambuffer for deflate method
class ZIPFilesystem::DeflateIStreambuf : public std::streambuf {
public:
	DeflateIStreambuf(StreamPoolEntry *backingStream, uint32_t fileoffset, uint32_t filelength, uint32_t compressedLength) :
		m_fileoffset(fileoffset), m_filelength(filelength), m_compressedFilelength(compressedLength),
		m_inbuffer(bufsize), m_outbuffer(bufsize),
		m_remaining(filelength), m_remainingCompressed(compressedLength) {
		assert(backingStream);
		assert(!backingStream->used);
		// Use the provided buffer
		m_backingStream = backingStream;
		m_backingStream->used = true;
		// seek to the beginning of the file
		m_backingStream->stream.rdbuf()->pubseekpos(fileoffset);
		zlibstream.zalloc = Z_NULL;
		zlibstream.zfree = Z_NULL;
		zlibstream.opaque = Z_NULL;
		zlibstream.avail_in = 0;
		zlibstream.next_in = reinterpret_cast<Bytef*>(&m_inbuffer[0]);
		inflateInit2(&zlibstream, -MAX_WBITS);
		// the buffer is empty at the start
		setg(&m_outbuffer[0], &m_outbuffer[0] + bufsize, &m_outbuffer[0] + bufsize);
	}
	virtual ~DeflateIStreambuf() {
		// unuse the provided buffer (free for another zipstream to use)
		inflateEnd(&zlibstream);
		m_backingStream->used = false;
	}
	DeflateIStreambuf(DeflateIStreambuf const& other) = delete;
	DeflateIStreambuf const& operator=(StorageIStreambuf const& other) = delete;

protected:

	virtual std::streambuf::int_type underflow() override {
		// either fill the buffer full, or with what is remaining
		size_t numberOfBytes = bufsize;
		if (m_remaining < bufsize) {
			numberOfBytes = m_remaining;
		}
		zlibstream.avail_out = numberOfBytes; // We want to decompress bufsize bytes
		zlibstream.next_out = reinterpret_cast<Bytef *>(&m_outbuffer[0]); // Where to store decompressed data

		int zlib_error = Z_OK;
		while (zlibstream.avail_out > 0 && zlib_error == Z_OK) {
			if (zlibstream.avail_in==0) { // if there is no data to decompress
				size_t numberOfInBytes = bufsize;
				if (m_remainingCompressed < bufsize) {
					numberOfInBytes = m_remainingCompressed;
				}
				// fill the complete inbuffer
				size_t bytesRead = m_backingStream->stream.rdbuf()->sgetn(&m_inbuffer[0], numberOfInBytes);
				m_remainingCompressed -= bytesRead;
				// see how much we really filled
				zlibstream.avail_in = bytesRead;
				zlibstream.next_in = reinterpret_cast<Bytef*>(&m_inbuffer[0]);
			}
			zlib_error = inflate(&zlibstream, Z_NO_FLUSH);
		}

		// Get the point to which the outbuffer is filled
		size_t bytesRead = numberOfBytes - zlibstream.avail_out;

		if (zlib_error != Z_OK && zlib_error != Z_STREAM_END) {
			// TODO set the failbit somehow
		}

		if (bytesRead > 0) {
			// Now there are some bytes less remaining
			m_remaining = m_remaining - bytesRead;
			// And reset the bufferpositions
			setg(&m_outbuffer[0], &m_outbuffer[0], &m_outbuffer[0] + bytesRead);
			return traits_type::to_int_type(*gptr());
		}
		else {
			return traits_type::eof();
		}
	}

	std::streambuf::int_type pbackfail(int_type) override {
		return traits_type::eof();
	}

	virtual std::streambuf::pos_type seekoff(std::streambuf::off_type offset, std::ios_base::seekdir way, std::ios_base::openmode mode) override {
		std::streambuf::off_type position = 0;
		switch (way) {
			case std::ios_base::cur:
				position = ( m_filelength - (m_remaining+(egptr()-gptr()))) + offset;
				break;
			case std::ios_base::beg:
				position = offset;
				break;
			case std::ios_base::end:
				position = m_filelength + offset;
				break;
			default:
				// Fixes clang warning "enumeration value '_S_ios_seekdir_end' not handled in switch"
				assert(false);
				break;
		}
		if (position < 0) {
			position = 0;
		}
		else if (position >m_filelength) {
			position = m_filelength;
		}

		return seekpos(position,mode);
	}

	virtual std::streambuf::pos_type seekpos(std::streambuf::pos_type pos, std::ios_base::openmode) override {
		// Random access won't work on compressed files -> we must decompress from the beginning to the absolute position
		std::streambuf::pos_type position = m_fileoffset + pos;

		if (position < m_fileoffset) {
			position = m_fileoffset;
		}
		else if (position > m_fileoffset + m_filelength) {
			position = m_fileoffset + m_filelength;
		}

		std::streamoff new_offset = position - m_fileoffset;
		std::streamoff old_offset = (m_filelength - (m_remaining + (egptr() - gptr())));
		std::streamoff first_buffer_entry = (m_filelength - (m_remaining + (egptr() - eback())));

		if (new_offset < old_offset) {
			if (new_offset >= first_buffer_entry) {
				setg(&m_outbuffer[0], &m_outbuffer[0] + (new_offset - first_buffer_entry), egptr());
				return new_offset;
			}
			else {
				// going back doesn't work so start from the beginning
				resetStream();
			}
		}

		// now underflow till the position we want to reach is inside the outbuffer
		while ((new_offset  ) >= (m_filelength - m_remaining) && !(m_remaining==0&& new_offset== m_filelength) ) {
			underflow();
		}
		first_buffer_entry = (m_filelength - (m_remaining + (egptr() - eback())));

		// Now set the current pointer to that position
		setg(&m_outbuffer[0], &m_outbuffer[0] + (new_offset - first_buffer_entry), egptr());

		return position - m_fileoffset;
	}

	StreamPoolEntry *m_backingStream;
	std::streampos m_fileoffset;
	std::streamoff m_filelength;
	std::streamoff m_compressedFilelength;
private:

	void resetStream() {
		m_remaining = m_filelength;
		m_remainingCompressed = m_compressedFilelength;
		m_backingStream->stream.rdbuf()->pubseekpos(m_fileoffset);
		zlibstream.next_in = reinterpret_cast<Bytef*>(&m_inbuffer[0]);
		zlibstream.avail_in = 0;
		inflateReset(&zlibstream);
		setg(&m_outbuffer[0], &m_outbuffer[0] + bufsize, &m_outbuffer[0] + bufsize);
	}

	static const int bufsize = 128;
	std::vector<char> m_inbuffer;
	std::vector<char> m_outbuffer;
	std::streamoff m_remaining;
	std::streamoff m_remainingCompressed;

	z_stream zlibstream;

};

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

ZIPFilesystem::ZIPFilesystem(std::string base_path, FilesystemView parent_fs, StringView encoding) :
	Filesystem(base_path, parent_fs) {
	// Open first entry of the input filebuffer pool
	m_isValid = false;
	StreamPoolEntry* initialEntry = new StreamPoolEntry();
	initialEntry->used = true;
	initialEntry->stream = parent_fs.OpenInputStream(GetPath());
	assert(initialEntry->stream);

	uint16_t centralDirectoryEntries = 0;
	uint32_t centralDirectorySize = 0;
	uint32_t centralDirectoryOffset = 0;

	ZipEntry entry;
	std::vector<char> filepath_arr;
	std::string filepath = "";

	std::istream& zipfile = initialEntry->stream; // Take the first streambuffer of the pool

	std::string zip_encoding = ToString(encoding);
	if (FindCentralDirectory(zipfile, centralDirectoryOffset, centralDirectorySize, centralDirectoryEntries)) {
		if (zip_encoding.empty()) {
			zipfile.seekg(centralDirectoryOffset); // Seek to the start of the central directory
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
					//Output::Debug("Bad encoding: {}. Trying next.", enc);
					continue;
				}
				zip_encoding = enc;
				break;
			}
			//Output::Debug("Detected ZIP encoding: {}", zip_encoding);
		}

		zipfile.clear();
		zipfile.seekg(centralDirectoryOffset); // Seek to the start of the central directory
		while (ReadCentralDirectoryEntry(zipfile, filepath_arr, entry.fileoffset, entry.filesize)) {
			filepath = filepath_arr.data();
			filepath = lcf::ReaderUtil::Recode(filepath, zip_encoding);
			// remove the offset directory from the front of the path

			// check if the entry is an directory or not (indicated by trailing /)
			if (filepath.back() == '/') {
				entry.isDirectory = true;
				filepath = filepath.substr(0, filepath.size() - 1);
			}
			else {
				entry.isDirectory = false;
			}

			m_zipContent.insert(std::pair<std::string, ZipEntry>(filepath, entry));
		}

		// Insert root path into m_zipContent
		entry.isDirectory = true;
		entry.fileoffset = 0;
		entry.filesize = 0;
		m_zipContent.insert(std::pair<std::string, ZipEntry>("", entry));

		zipfile.seekg(0);
		initialEntry->used = false;
		m_InputPool.push_back(initialEntry);
		m_isValid = true;
	}
}

bool ZIPFilesystem::FindCentralDirectory(std::istream & zipfile, uint32_t & offset, uint32_t & size, uint16_t & numberOfEntries) {
	uint32_t magic = 0;
	bool found = false;

	// seek to the first position where the endOfCentralDirectory Signature may occur
	zipfile.seekg(-endOfCentralDirectorySize, std::ios_base::end);

	// The only variable length field in the end of central directory is the comment which has a maximum length of UINT16_MAX - so if we seek longer, this is no zip file
	for (size_t i = 0; i < UINT16_MAX&&zipfile.good() && !found; i++) {
		zipfile.read(reinterpret_cast<char*>(&magic), sizeof(magic));
		Utils::SwapByteOrder(magic); //Take care of big endian systems
		if (magic == endOfCentralDirectory) {
			found = true;
		}
		else {
			// if not yet found the magic number step one byte back in the file
			zipfile.seekg(-(sizeof(magic) + 1), std::ios_base::cur);
		}
	}

	if (found) {
		zipfile.seekg(6, std::ios_base::cur); // Jump over multiarchive related fields
		zipfile.read(reinterpret_cast<char*>(&numberOfEntries), sizeof(uint16_t));
		Utils::SwapByteOrder(numberOfEntries);
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

bool ZIPFilesystem::ReadCentralDirectoryEntry(std::istream & zipfile, std::vector<char> & filename, uint32_t & offset, uint32_t & uncompressed_size) {
	uint32_t magic = 0;
	uint16_t filepath_length;
	uint16_t extra_field_length;
	uint16_t comment_length;

	zipfile.read(reinterpret_cast<char*>(&magic), sizeof(magic));
	Utils::SwapByteOrder(magic); // Take care of big endian systems
	if (magic != centralDirectoryEntry) return false;
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
	if (filename.capacity() < filepath_length+1) {
		filename.resize(filepath_length + 1);
	}
	filename.data()[filepath_length] = '\0';
	zipfile.read(reinterpret_cast<char*>(filename.data()),filepath_length);
	zipfile.seekg(comment_length+ extra_field_length, std::ios_base::cur); // Jump over currently not needed entries
	std::string a = filename.data();
	return true;
}

bool ZIPFilesystem::ReadLocalHeader(std::istream & zipfile, uint32_t & offset, StorageMethod & method, uint32_t & compressedSize) {
	uint32_t magic = 0;
	uint16_t filepath_length;
	uint16_t extra_field_length;
	uint16_t flags;
	uint16_t compression;

	zipfile.read(reinterpret_cast<char*>(&magic), sizeof(magic));
	Utils::SwapByteOrder(magic); //Take care of big endian systems
	if (magic != localHeader) return false;
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
	offset = localHeaderSize + filepath_length+ extra_field_length;
	return true;
}


ZIPFilesystem::~ZIPFilesystem() {
	for (auto it = m_InputPool.begin(); it != m_InputPool.end(); it++) {
		delete (*it);
	}
}

bool ZIPFilesystem::IsFile(StringView path) const {
	std::string path_normalized = normalize_path(path);
	auto it = m_zipContent.find(path_normalized);
	if (it != m_zipContent.end()) {
		return !it->second.isDirectory;
	}
	else {
		return false;
	}
}

bool ZIPFilesystem::IsDirectory(StringView path, bool) const {
	std::string path_normalized = normalize_path(path);
	auto it = m_zipContent.find(path_normalized);
	if (it != m_zipContent.end()) {
		return it->second.isDirectory;
	}
	else {
		return false;
	}
}

bool ZIPFilesystem::Exists(StringView path) const {
	std::string path_normalized = normalize_path(path);
	auto it = m_zipContent.find(path_normalized);
	return (it != m_zipContent.end());
}

int64_t ZIPFilesystem::GetFilesize(StringView path) const {
	std::string path_normalized = normalize_path(path);

	auto it = m_zipContent.find(path_normalized);
	if (it != m_zipContent.end()) {
		return it->second.filesize;
	}
	else {
		return 0;
	}
}

std::streambuf* ZIPFilesystem::CreateInputStreambuffer(StringView path, std::ios_base::openmode) const {
	if (!m_isValid) return nullptr;

	std::string path_normalized = normalize_path(path);

	auto it = m_zipContent.find(path_normalized);
	if (it != m_zipContent.end() && !it->second.isDirectory) {
		StreamPoolEntry * inputStream = nullptr;
		// search for an unused stream in our streampool
		for (size_t i = 0; i < m_InputPool.size(); i++) {
			if (!m_InputPool[i]->used) {
				inputStream = m_InputPool[i];
			}
		}
		// If theres no unused stream in the pool - create a new one ;)
		if (inputStream == nullptr) {
			StreamPoolEntry* newEntry = new StreamPoolEntry();
			newEntry->stream = GetParent().OpenInputStream(GetPath());
			newEntry->used = false;
			m_InputPool.push_back(newEntry);
			inputStream = m_InputPool.back();
		}

		// now seek to the file and check it's local header
		std::istream& file = inputStream->stream;
		file.seekg(it->second.fileoffset);
		StorageMethod method;
		uint32_t localOffset = 0;
		uint32_t compressedSize = 0;
		if (ReadLocalHeader(file, localOffset, method, compressedSize)) {

			// The instantiated Streambuffer will set the pool entry to used
			switch (method) {
			case StorageMethod::Plain:
				return new StorageIStreambuf(inputStream, it->second.fileoffset + localOffset, it->second.filesize);
			case StorageMethod::Deflate:
				return new DeflateIStreambuf(inputStream, it->second.fileoffset + localOffset, it->second.filesize, compressedSize);
			default:
				break;
			}
		}
	}
	return nullptr;
}

bool ZIPFilesystem::GetDirectoryContent(StringView path, std::vector<DirectoryTree::Entry>& entries) const {
	if (!m_isValid) {
		return false;
	}

	std::string path_normalized = normalize_path(path);
	if (path_normalized.size() != 0 && path_normalized.back() != '/') {
		path_normalized += "/";
	}

	for (const auto &it : m_zipContent) {
		if (StringView(it.first).starts_with(path_normalized) &&
			it.first.substr(path_normalized.size(), it.first.size() - path_normalized.size()).find_last_of('/') == std::string::npos) {
			// Everything that starts with the path but isn't the path and does contain no slash
			entries.emplace_back(
						it.first.substr(path_normalized.size(), it.first.size() - path_normalized.size()),
						it.second.isDirectory ? DirectoryTree::FileType::Directory : DirectoryTree::FileType::Regular);
		}
	}

	return true;
}
