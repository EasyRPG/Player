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

#ifndef EP_IMAGE_BMP_H
#define EP_IMAGE_BMP_H

#include <cstdint>
#include "filesystem_stream.h"

namespace ImageBMP {
	struct BitmapHeader {
		int size = 0;
		int w = 0;
		int h = 0;
		int planes = 0;
		int depth = 0;
		int compression = 0;
		int num_colors = 0;
		int palette_size = 0;
	};

	bool ReadBMP(const uint8_t* data, unsigned len, bool transparent, int& width, int& height, void*& pixels);
	bool ReadBMP(Filesystem_Stream::InputStream& stream, bool transparent, int& width, int& height, void*& pixels);

	BitmapHeader ParseHeader(const uint8_t*& ptr, uint8_t const* e);
}

#endif
