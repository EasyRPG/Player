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

#include "filesystem_stream.h"

#include <utility>

#ifdef USE_CUSTOM_FILEBUF
#  include <unistd.h>
#endif

Filesystem_Stream::InputStream::InputStream(std::streambuf* sb, std::string name) :
	std::istream(sb), name(std::move(name)) {}

Filesystem_Stream::InputStream::~InputStream() {
	Close();
}

Filesystem_Stream::InputStream::InputStream(InputStream&& is) noexcept : std::istream(std::move(is)) {
	set_rdbuf(is.rdbuf());
	is.set_rdbuf(nullptr);
	name = std::move(is.name);
}

Filesystem_Stream::InputStream& Filesystem_Stream::InputStream::operator=(InputStream&& is) noexcept {
	if (this == &is) return *this;
	set_rdbuf(is.rdbuf());
	is.set_rdbuf(nullptr);
	name = std::move(is.name);
	std::istream::operator=(std::move(is));
	return *this;
}

StringView Filesystem_Stream::InputStream::GetName() const {
	return name;
}

std::streampos Filesystem_Stream::InputStream::GetSize() const {
	if (!size_cached) {
		size_cached = true;
		auto cur_pos = rdbuf()->pubseekoff(0, std::ios_base::cur, std::ios_base::in);
		size = rdbuf()->pubseekoff(0, std::ios_base::end, std::ios_base::in);
		rdbuf()->pubseekoff(cur_pos, std::ios_base::beg, std::ios_base::in);
	}
	return size;
}

void Filesystem_Stream::InputStream::Close() {
	delete rdbuf();
	set_rdbuf(nullptr);
}

Filesystem_Stream::OutputStream::OutputStream(std::streambuf* sb, FilesystemView fs, std::string name) :
	std::ostream(sb), fs(std::move(fs)), name(std::move(name)) {};

Filesystem_Stream::OutputStream::~OutputStream() {
	Close();
	if (fs) {
		fs.ClearCache();
	}
}

Filesystem_Stream::OutputStream::OutputStream(OutputStream&& os) noexcept : std::ostream(std::move(os)) {
	set_rdbuf(os.rdbuf());
	os.set_rdbuf(nullptr);
	name = std::move(os.name);
}

Filesystem_Stream::OutputStream& Filesystem_Stream::OutputStream::operator=(OutputStream&& os) noexcept {
	if (this == &os) return *this;
	set_rdbuf(os.rdbuf());
	os.set_rdbuf(nullptr);
	name = std::move(os.name);
	std::ostream::operator=(std::move(os));
	return *this;
}

StringView Filesystem_Stream::OutputStream::GetName() const {
	return name;
}

void Filesystem_Stream::OutputStream::Close() {
	delete rdbuf();
	set_rdbuf(nullptr);
}

Filesystem_Stream::InputMemoryStreamBufView::InputMemoryStreamBufView(Span<uint8_t> buffer_view)
		: std::streambuf(), buffer_view(buffer_view) {
	char* cbuffer = reinterpret_cast<char*>(buffer_view.data());
	setg(cbuffer, cbuffer, cbuffer + buffer_view.size());
}

std::streambuf::pos_type Filesystem_Stream::InputMemoryStreamBufView::seekoff(std::streambuf::off_type offset, std::ios_base::seekdir dir, std::ios_base::openmode mode) {
	std::streambuf::pos_type off;
	if (dir == std::ios_base::beg) {
		off = offset;
	} else if (dir == std::ios_base::cur) {
		off = gptr() - eback() + offset;
	} else {
		off = buffer_view.size() + offset;
	}
	return seekpos(off, mode);
}

std::streambuf::pos_type Filesystem_Stream::InputMemoryStreamBufView::seekpos(std::streambuf::pos_type pos, std::ios_base::openmode) {
	auto off = Utils::Clamp<std::streambuf::pos_type>(pos, 0, buffer_view.size());
	setg(eback(), eback() + off, egptr());
	return off;
}

Filesystem_Stream::InputMemoryStreamBuf::InputMemoryStreamBuf(std::vector<uint8_t> buffer)
		: InputMemoryStreamBufView(buffer), buffer(std::move(buffer)) {

}

#ifdef USE_CUSTOM_FILEBUF

Filesystem_Stream::FdStreamBuf::FdStreamBuf(int fd, bool is_read) : fd(fd), is_read(is_read) {
	if (is_read) {
		clear_buffer();
	} else {
		setp(buffer.end(), buffer.end());
	}
}

Filesystem_Stream::FdStreamBuf::~FdStreamBuf() {
	if (!is_read) {
		sync();
	}

	close(fd);
}

Filesystem_Stream::FdStreamBuf::int_type Filesystem_Stream::FdStreamBuf::underflow() {
	assert(gptr() == egptr());

	auto bytes_read = read(fd, buffer.begin(), buffer.size());
	if (bytes_read <= 0) {
		return traits_type::eof();
	}
	file_offset += bytes_read;

	setg(buffer.begin(), buffer.begin(), buffer.begin() + bytes_read);

	return traits_type::to_int_type(*gptr());
}

std::streambuf::pos_type Filesystem_Stream::FdStreamBuf::seekoff(std::streambuf::off_type offset, std::ios_base::seekdir dir, std::ios_base::openmode mode) {
	// Not implemented for writing
	assert(is_read);

	if (dir == std::ios_base::beg) {
		offset = offset - file_offset + bytes_remaining();
		dir = std::ios_base::cur;
	}

	if (dir == std::ios_base::cur) {
		if (offset < 0) {
			auto dist = std::distance(gptr(), gptr() - offset);
			if (gptr() + offset < eback()) {
				// Not cached: Outside of the buffer: Reposition the stream
				file_offset = lseek(fd, -dist - bytes_remaining(), SEEK_CUR);
				clear_buffer();
			} else {
				setg(buffer.begin(), gptr() - dist, egptr());
			}
		} else if (offset > 0) {
			auto dist = std::distance(gptr(), gptr() + offset);
			if (gptr() + offset > egptr()) {
				// Not cached: Outside of the buffer: Reposition the stream
				file_offset = lseek(fd, dist - bytes_remaining(), SEEK_CUR);
				clear_buffer();
			} else {
				setg(buffer.begin(), gptr() + dist, egptr());
			}
		}
		return file_offset - bytes_remaining();
	} else {
		// Not cached: Seek to end
		clear_buffer();
		file_offset = lseek(fd, offset, SEEK_END);

		if (file_offset < 0) {
			file_offset = 0;
			return -1;
		}

		return file_offset;
	}

	assert(false);
}

std::streambuf::pos_type Filesystem_Stream::FdStreamBuf::seekpos(std::streambuf::pos_type pos, std::ios_base::openmode mode) {
	return seekoff(pos, std::ios_base::beg, mode);
}


Filesystem_Stream::FdStreamBuf::int_type Filesystem_Stream::FdStreamBuf::overflow(int c) {
	assert(pptr() == epptr());

	if (c == traits_type::eof() || sync() == -1) {
		return traits_type::eof();
	}

	*pptr() = traits_type::to_char_type(c);

	pbump(1);
	return c;
}

int Filesystem_Stream::FdStreamBuf::sync() {
	char *p = pbase();
	while (p < pptr()) {
		int written = write(fd, p, pptr() - p);
		if (written <= 0) {
			return -1;
		}
		p += written;
	}

	setp(buffer.begin(), buffer.end());
	return 0;
}

void Filesystem_Stream::FdStreamBuf::clear_buffer() {
	setg(buffer.begin(), buffer.end(), buffer.end());
}

ssize_t Filesystem_Stream::FdStreamBuf::bytes_remaining() const {
	return egptr() - gptr();
}

#endif
