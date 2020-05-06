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

#ifndef EP_FILESYSTEM_H
#define EP_FILESYSTEM_H

// Headers
#include <cassert>
#include <istream>
#include <ostream>
#include "system.h"

class Filesystem {
public:
	class vfs_istream;
	class vfs_ostream;

	using InputStreamRaw = vfs_istream;
	using OutputStreamRaw = vfs_ostream;

	using InputStream = std::shared_ptr<InputStreamRaw>;
	using OutputStream = std::shared_ptr<OutputStreamRaw>;

	class vfs_istream : public std::istream {
	public:
		explicit vfs_istream(std::streambuf* sb, std::streamsize size);
		~vfs_istream() override;

		std::streamsize get_size() const;

	private:
		std::streamsize size;
	};

	class vfs_ostream : public std::ostream {
	public:
		explicit vfs_ostream(std::streambuf *sb);
		~vfs_ostream() override;
	};

	static constexpr std::ios_base::seekdir CSeekdirToCppSeekdir(int origin);

	static constexpr int CppSeekdirToCSeekdir(std::ios_base::seekdir origin);
};

constexpr std::ios_base::seekdir Filesystem::CSeekdirToCppSeekdir(int origin) {
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

constexpr int Filesystem::CppSeekdirToCSeekdir(std::ios_base::seekdir origin) {
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
