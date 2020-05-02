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

#include "filesystem.h"

Filesystem::vfs_istream::vfs_istream(std::streambuf *sb, std::streamsize size) :
	size(size), std::istream(sb) {
	// no-op
}

Filesystem::vfs_istream::~vfs_istream() {
	delete rdbuf();
}

std::streamsize Filesystem::vfs_istream::get_size() const {
	return size;
}

Filesystem::vfs_ostream::vfs_ostream(std::streambuf *sb) :
	std::ostream(sb) {
}

Filesystem::vfs_ostream::~vfs_ostream() {
	delete rdbuf();
}
