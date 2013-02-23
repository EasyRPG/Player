/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////

#include "system.h"
#ifdef SUPPORT_BMP

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <vector>
#include "output.h"
#include "image_bmp.h"

////////////////////////////////////////////////////////////
static uint16_t get_2(const uint8_t *p)
{
    return
		((uint16_t) p[0] << 0) |
		((uint16_t) p[1] << 8);
}

static uint32_t get_4(const uint8_t *p)
{
    return
		((uint32_t) p[0] <<  0) |
		((uint32_t) p[1] <<  8) |
		((uint32_t) p[2] << 16) |
		((uint32_t) p[3] << 24);
}

////////////////////////////////////////////////////////////
void ImageBMP::ReadBMP(const uint8_t* data, unsigned len, bool transparent,
					   int& width, int& height, void*& pixels) {
	pixels = NULL;

	// BITMAPFILEHEADER structure
	//
	// 0	2	signature: 'B','M'
	// 2	4	file size
	// 6	2	reserved
	// 8	2	reserved
	// 10	4	offset to bitmap data
	static const uint BITMAPFILEHEADER_SIZE = 14;

	if (len < 64 || strncmp((char*) &data[0], "BM", 2) != 0) {
		Output::Error("Not a valid BMP file.");
		return;
	}

	const unsigned file_size = get_4(&data[2]);
	if (file_size != len) {
		Output::Error("Incorrect BMP file size.");
		return;
	}

	const unsigned bits_offset = get_4(&data[10]);

	// BITMAPINFOHEADER structure
	//
	// 0	4	BITMAPINFOHEADER size
	// 4	4	width
	// 8	4	height (+ve => bottom-up, -ve => top-down)
	// 12	2	number of planes (must be 1)
	// 14	2	bits per pixel
	// 16	4	compression
	// 20	4	image size
	// 24	4	X pixels per meter
	// 28	4	Y pixels per meter
	// 32	4	number of palette colors used
	// 36	4	number of important palette colors
	// 40 ... palette

	static const unsigned BITMAPINFOHEADER_SIZE = 40;
	if (get_4(&data[BITMAPFILEHEADER_SIZE + 0]) != BITMAPINFOHEADER_SIZE) {
		Output::Error("Incorrect BMP header size.");
		return;
	}

	width = (int) get_4(&data[BITMAPFILEHEADER_SIZE + 4]);
	height = (int) get_4(&data[BITMAPFILEHEADER_SIZE + 8]);

	bool vflip = height > 0;
	if (!vflip)
		height = -height;

	const int planes = (int) get_2(&data[BITMAPFILEHEADER_SIZE + 12]);
	if (planes != 1) {
		Output::Error("BMP planes is not 1.");
		return;
	}

	const int depth = (int) get_2(&data[BITMAPFILEHEADER_SIZE + 14]);
	if (depth != 8) {
		Output::Error("BMP image is not 8-bit.");
		return;
	}

	const int compression = get_4(&data[BITMAPFILEHEADER_SIZE + 16]);
	static const int BI_RGB = 0;
	if (compression != BI_RGB) {
		Output::Error("BMP image is compressed.");
		return;
	}

	int image_size = get_4(&data[BITMAPFILEHEADER_SIZE + 20]);
	if (image_size != 0 && image_size != width * height) {
		Output::Error("Invalid BMP image size.");
		return;
	}

	int num_colors = std::min(256U, get_4(&data[BITMAPFILEHEADER_SIZE + 36]));
	uint8_t (*palette)[4] = (uint8_t(*)[4]) &data[BITMAPFILEHEADER_SIZE + 40];
	const uint8_t* src_pixels = &data[bits_offset];

	// Ensure no palette entry is an exact duplicate of #0
	for (int i = 1; i < num_colors; i++) {
		if (palette[i][0] == palette[0][0] &&
			palette[i][1] == palette[0][1] &&
			palette[i][2] == palette[0][2]) {
			palette[i][0] ^= 1;
		}
	}

	pixels = malloc(width * height * 4);

	uint8_t* dst = (uint8_t*) pixels;
	for (int y = 0; y < height; y++) {
		const uint8_t* src = src_pixels + (vflip ? height - 1 - y : y) * width;
		for (int x = 0; x < width; x++) {
			const uint8_t& pix = *src++;
			const uint8_t* color = palette[pix];
			*dst++ = color[2];
			*dst++ = color[1];
			*dst++ = color[0];
			*dst++ = (transparent && pix == 0) ? 0 : 255;
		}
	}
}

////////////////////////////////////////////////////////////
void ImageBMP::ReadBMP(FILE* stream, bool transparent,
					int& width, int& height, void*& pixels) {
	fseek(stream, 0, SEEK_END);
	long size = ftell(stream);
	fseek(stream, 0, SEEK_SET);
	std::vector<uint8_t> buffer(size);
	fread((void*) &buffer.front(), 1, size, stream);
	ReadBMP(&buffer.front(), (unsigned) size, transparent, width, height, pixels);
}

#endif // SUPPORT_BMP
