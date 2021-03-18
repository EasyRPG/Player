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

Filesystem_Stream::InputStream::InputStream(std::streambuf* sb) : std::istream(sb) {}

Filesystem_Stream::InputStream::~InputStream() {
	delete rdbuf();
}

Filesystem_Stream::InputStream::InputStream(InputStream&& is) noexcept : std::istream(std::move(is)) {
	set_rdbuf(is.rdbuf());
	is.set_rdbuf(nullptr);
}

Filesystem_Stream::InputStream& Filesystem_Stream::InputStream::operator=(InputStream&& is) noexcept {
	if (this == &is) return is;
	std::istream::operator=(std::move(is));
	set_rdbuf(is.rdbuf());
	is.set_rdbuf(nullptr);
	return is;
}

Filesystem_Stream::OutputStream::OutputStream(std::streambuf* sb, FilesystemView fs) :
	std::ostream(sb), fs(fs) {};

Filesystem_Stream::OutputStream::~OutputStream() {
	delete rdbuf();
	if (fs) {
		fs.ClearCache();
	}
}

Filesystem_Stream::OutputStream::OutputStream(OutputStream&& os) noexcept : std::ostream(std::move(os)) {
	set_rdbuf(os.rdbuf());
	os.set_rdbuf(nullptr);
}

Filesystem_Stream::OutputStream& Filesystem_Stream::OutputStream::operator=(OutputStream&& os) noexcept {
	if (this == &os) return os;
	std::ostream::operator=(std::move(os));
	set_rdbuf(os.rdbuf());
	os.set_rdbuf(nullptr);
	return os;
}

Filesystem_Stream::InputMemoryStreamBuf::InputMemoryStreamBuf(Span<uint8_t> buffer)
		: std::streambuf(), buffer(buffer) {
	char* cbuffer = reinterpret_cast<char*>(buffer.data());
	setg(cbuffer, cbuffer, cbuffer + buffer.size());
}

std::streambuf::pos_type Filesystem_Stream::InputMemoryStreamBuf::seekoff(std::streambuf::off_type offset, std::ios_base::seekdir dir, std::ios_base::openmode mode) {
	std::streambuf::pos_type off;
	if (dir == std::ios_base::beg) {
		off = offset;
	} else if (dir == std::ios_base::cur) {
		off = gptr() - eback() + offset;
	} else {
		off = buffer.size() + offset;
	}
	return seekpos(off, mode);
}

std::streambuf::pos_type Filesystem_Stream::InputMemoryStreamBuf::seekpos(std::streambuf::pos_type pos, std::ios_base::openmode) {
	auto off = Utils::Clamp<std::streambuf::pos_type>(pos, 0, buffer.size());
	setg(eback(), eback() + off, egptr());
	return off;
}
