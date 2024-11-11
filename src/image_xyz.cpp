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

// Headers
#include <cstdlib>
#include <cstring>
#include <istream>
#include <zlib.h>
#include <vector>
#include "output.h"
#include "image_xyz.h"

bool ImageXYZ::Read(const uint8_t* data, unsigned len, bool transparent, ImageOut& output) {
	output.pixels = nullptr;

	if (len < 8) {
		Output::Warning("Not a valid XYZ file.");
		return false;
	}

	uint16_t w = data[4] + (data[5] << 8);
	uint16_t h = data[6] + (data[7] << 8);
	uLongf src_size = len - 8;
	Bytef* src_buffer = (Bytef*)&data[8];
	uLongf dst_size = 768 + (w * h);
	std::vector<Bytef> dst_buffer(dst_size);

	int status = uncompress(&dst_buffer.front(), &dst_size, src_buffer, src_size);
	if (status != Z_OK) {
		Output::Warning("Error decompressing XYZ file.");
		return false;
	}
	const uint8_t (*palette)[3] = (const uint8_t(*)[3]) &dst_buffer.front();

	output.pixels = malloc(w * h * 4);
	if (!output.pixels) {
		Output::Warning("Error allocating XYZ pixel buffer.");
		return false;
	}

	uint8_t* dst = (uint8_t*) output.pixels;
	const uint8_t* src = (const uint8_t*) &dst_buffer[768];
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			uint8_t pix = *src++;
			const uint8_t* color = palette[pix];
			*dst++ = color[0];
			*dst++ = color[1];
			*dst++ = color[2];
			*dst++ = (transparent && pix == 0) ? 0 : 255;
		}
	}

	output.width = w;
	output.height = h;
	output.bpp = 8;

	return true;
}

bool ImageXYZ::Read(Filesystem_Stream::InputStream& stream, bool transparent, ImageOut& output) {
	std::vector<uint8_t> buffer = Utils::ReadStream(stream);
	return Read(&buffer.front(), (unsigned) buffer.size(), transparent, output);
}
