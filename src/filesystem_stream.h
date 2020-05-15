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

#ifndef EP_FILESYSTEM_STREAM_H
#define EP_FILESYSTEM_STREAM_H

// Headers
#include <cassert>
#include <istream>
#include <ostream>
#include "utils.h"

namespace Filesystem_Stream {
	class InputStream final : public std::istream {
	public:
		explicit InputStream(): std::istream(nullptr) {}
		explicit InputStream(std::streambuf* sb, std::streamsize size) : std::istream(sb), size(size) {}
		~InputStream() override {
			delete rdbuf();
		}
		InputStream(const InputStream&) = delete;
		InputStream& operator=(const InputStream&) = delete;
		InputStream(InputStream&& is) : std::istream(std::move(is)) {
			set_rdbuf(is.rdbuf());
			is.set_rdbuf(nullptr);
			size = is.size;
			is.size = 0;
		}
		InputStream& operator=(InputStream&& is) {
			if (this == &is) return is;
			std::istream::operator=(std::move(is));
			set_rdbuf(is.rdbuf());
			is.set_rdbuf(nullptr);
			size = is.size;
			is.size = 0;
			return is;
		}

		std::streamsize GetSize() const;

		template <typename T>
		bool ReadIntoObj(T& obj);

	private:
		template <typename T>
		bool Read0(T& obj);

		std::streamsize size = 0;
	};

	class OutputStream final : public std::ostream {
	public:
		explicit OutputStream(): std::ostream(nullptr) {}
		explicit OutputStream(std::streambuf* sb) : std::ostream(sb) {};
		~OutputStream() override {
			delete rdbuf();
		}
		OutputStream(const OutputStream&) = delete;
		OutputStream& operator=(const OutputStream&) = delete;
		OutputStream(OutputStream&& os) noexcept : std::ostream(std::move(os)) {
			set_rdbuf(os.rdbuf());
			os.set_rdbuf(nullptr);
		}
		OutputStream& operator=(OutputStream&& os) noexcept {
			if (this == &os) return os;
			std::ostream::operator=(std::move(os));
			set_rdbuf(os.rdbuf());
			os.set_rdbuf(nullptr);
			return os;
		}
	};

	static constexpr std::ios_base::seekdir CSeekdirToCppSeekdir(int origin);

	static constexpr int CppSeekdirToCSeekdir(std::ios_base::seekdir origin);
};

inline std::streamsize Filesystem_Stream::InputStream::GetSize() const {
	return size;
}

template<typename T>
inline bool Filesystem_Stream::InputStream::Read0(T& obj) {
	return read(reinterpret_cast<char*>(&obj), sizeof(obj)).gcount() == sizeof(obj);
}

template<typename T>
inline bool Filesystem_Stream::InputStream::ReadIntoObj(T& obj) {
	return Read0(obj);
}

template <>
inline bool Filesystem_Stream::InputStream::ReadIntoObj(int16_t& obj) {
	bool success = Read0(obj);
	uint16_t uobj = obj;
	Utils::SwapByteOrder(uobj);
	obj = uobj;
	return success;
}

template <>
inline bool Filesystem_Stream::InputStream::ReadIntoObj(uint16_t& obj) {
	bool success = Read0(obj);
	Utils::SwapByteOrder(obj);
	return success;
}

template <>
inline bool Filesystem_Stream::InputStream::ReadIntoObj(int32_t& obj) {
	bool success = Read0(obj);
	uint32_t uobj = obj;
	Utils::SwapByteOrder(uobj);
	obj = uobj;
	return success;
}

template <>
inline bool Filesystem_Stream::InputStream::ReadIntoObj(uint32_t& obj) {
	bool success = Read0(obj);
	Utils::SwapByteOrder(obj);
	return success;
}

constexpr std::ios_base::seekdir Filesystem_Stream::CSeekdirToCppSeekdir(int origin) {
	switch (origin) {
		case SEEK_SET:
			return std::ios_base::beg;
		case SEEK_CUR:
			return std::ios_base::cur;
		case SEEK_END:
			return std::ios_base::end;
		default:
			assert(false);
			return std::ios_base::beg;
	}
}

constexpr int Filesystem_Stream::CppSeekdirToCSeekdir(std::ios_base::seekdir origin) {
	switch (origin) {
		case std::ios_base::beg:
			return SEEK_SET;
		case std::ios_base::cur:
			return SEEK_CUR;
		case std::ios_base::end:
			return SEEK_END;
		default:
			assert(false);
			return SEEK_SET;
	}
}

#endif
