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
#include "system.h"
#ifdef SUPPORT_BMP

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <vector>
#include "output.h"
#include "image_bmp.h"

static uint16_t get_2(const uint8_t *p) {
	return (uint16_t) p[0] | (p[1] << 8);
}

static uint32_t get_4(const uint8_t *p) {
	return (uint32_t) p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

bool ImageBMP::ReadBMP(const uint8_t* data, unsigned len, bool transparent,
					   int& width, int& height, void*& pixels) {
	pixels = nullptr;

	if (len < 64) {
		Output::Warning("Not a valid BMP file.");
		return false;
	}

	static const unsigned BITMAPFILEHEADER_SIZE = 14;
	const unsigned bits_offset = get_4(&data[BITMAPFILEHEADER_SIZE - 4]);

	unsigned int w = get_4(&data[BITMAPFILEHEADER_SIZE + 4]);
	unsigned int h = get_4(&data[BITMAPFILEHEADER_SIZE + 8]);

	bool vflip = h > 0;
	if (!vflip)
		h = -h;

	const int planes = get_2(&data[BITMAPFILEHEADER_SIZE + 12]);
	if (planes != 1) {
		Output::Warning("BMP planes is not 1.");
		return false;
	}

	const int depth = get_2(&data[BITMAPFILEHEADER_SIZE + 14]);
	if (depth != 8 && depth != 4) {
		Output::Warning("BMP image depth unsupported: %i bit.", depth);
		return false;
	}

	const int compression = get_4(&data[BITMAPFILEHEADER_SIZE + 16]);
	if (compression) {
		Output::Warning("BMP image is compressed.");
		return false;
	}

	int num_colors = std::min((uint32_t) 256, get_4(&data[BITMAPFILEHEADER_SIZE + 32]));
	if (num_colors == 0) // 0 means default, i.e. max.
		num_colors = depth << 2;

	uint8_t (*palette)[4] = (uint8_t(*)[4]) &data[BITMAPFILEHEADER_SIZE +
		get_4(&data[BITMAPFILEHEADER_SIZE + 0])];

	// Ensure no palette entry is an exact duplicate of the transparent color at #0
	for (int i = 1; i < num_colors; i++) {
		if (palette[i][0] == palette[0][0] &&
			palette[i][1] == palette[0][1] &&
			palette[i][2] == palette[0][2]) {
			palette[i][0] ^= 1;
		}
	}

	const uint8_t* src_pixels = &data[bits_offset];

	// bitmap scan lines need to be aligned to 32 bit boundaries, add padding if needed
	int line_width = (depth == 4) ? (w + 1) >> 1 : w;
	int padding = (-line_width)&3;

	pixels = malloc(w * h * 4);
	if (!pixels) {
		Output::Warning("Error allocating BMP pixel buffer.");
		return false;
	}

	uint8_t* dst = (uint8_t*) pixels;
	for (unsigned int y = 0; y < h; y++) {
		const uint8_t* src = src_pixels + (vflip ? h - 1 - y : y) * (line_width + padding);
		for (unsigned int x = 0; x < w; x += 2) {
			uint8_t pix = *src++;
			uint8_t pix2 = 0;
			const uint8_t* color;

			// split up packed pixel
			if (depth == 4) {
				pix2 = pix & 15;
				pix >>= 4;
			}

			color = palette[pix];
			*dst++ = color[2];
			*dst++ = color[1];
			*dst++ = color[0];
			*dst++ = (transparent && pix == 0) ? 0 : 255;

			// end of line
			if (x + 1 == w)
				break;

			pix = (depth == 8) ? *src++ : pix2;

			color = palette[pix];
			*dst++ = color[2];
			*dst++ = color[1];
			*dst++ = color[0];
			*dst++ = (transparent && pix == 0) ? 0 : 255;
		}
	}

	width = w;
	height = h;
	return true;
}

bool ImageBMP::ReadBMP(FILE* stream, bool transparent,
					int& width, int& height, void*& pixels) {
	fseek(stream, 0, SEEK_END);
	long size = ftell(stream);
	fseek(stream, 0, SEEK_SET);
	std::vector<uint8_t> buffer(size);
	long size_read = fread((void*) &buffer.front(), 1, size, stream);
	if (size_read != size) {
		Output::Warning("Error reading BMP file.");
		return false;
	}
	return ReadBMP(&buffer.front(), (unsigned) size, transparent, width, height, pixels);
}

#endif // SUPPORT_BMP
