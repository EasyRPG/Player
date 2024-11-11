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
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <vector>
#include "output.h"
#include "image_bmp.h"

static uint16_t get_2(const uint8_t *&p, const uint8_t* e) {
	if (e - p < 2) {
		p = e;
		return 0;
	}
	auto value = static_cast<uint16_t>(p[0] | (p[1] << 8));
	p += 2;
	return value;
}

static uint32_t get_4(const uint8_t *&p, const uint8_t* e) {
	if (e - p < 4) {
		p = e;
		return 0;
	}
	auto value = static_cast<uint32_t>(p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24));
	p+= 4;
	return value;
}

ImageBMP::BitmapHeader ImageBMP::ParseHeader(const uint8_t*& ptr, uint8_t const* const e) {
	BitmapHeader hdr;

	auto* hdr_start = ptr;

	auto size = get_4(ptr, e);

	hdr.size = size;
	if (size == 12) {
		// BITMAPCOREHEADER
		hdr.w = get_2(ptr, e);
		hdr.h = get_2(ptr, e);
		hdr.planes = get_2(ptr, e);
		hdr.depth = get_2(ptr, e);
		hdr.num_colors = (1 << hdr.depth);
		hdr.palette_size = 3;
	} else {
		// BITMAPINFOHEADER, BITMAPV4HEADER, or BITMAPV5HEADER
		hdr.w = get_4(ptr, e);
		hdr.h = get_4(ptr, e);
		hdr.planes = get_2(ptr, e);
		hdr.depth = get_2(ptr, e);
		hdr.compression = get_4(ptr, e);
		ptr += 12;
		hdr.num_colors = std::min(uint32_t(256), get_4(ptr, e));
		hdr.palette_size = 4;
		if (hdr.num_colors == 0) {
			// When 0 number of colors is the maximum allowed
			if (hdr.depth == 4) {
				hdr.num_colors = 16;
			} else if (hdr.depth == 8) {
				hdr.num_colors = 256;
			}
		}
	}

	ptr = hdr_start + size;
	return hdr;
}

bool ImageBMP::Read(const uint8_t* data, unsigned len, bool transparent, ImageOut& output) {
	output.pixels = nullptr;

	if (len < 64) {
		Output::Warning("Not a valid BMP file.");
		return false;
	}
	auto* ptr = data;
	auto* e = data + len;

	static const unsigned BITMAPFILEHEADER_SIZE = 14;

	ptr += BITMAPFILEHEADER_SIZE - 4;
	const unsigned bits_offset = get_4(ptr, e);

	auto hdr = ParseHeader(ptr, e);

	bool vflip = hdr.h > 0;
	if (!vflip)
		hdr.h = -hdr.h;

	if (hdr.planes != 1) {
		Output::Warning("BMP planes is not 1.");
		return false;
	}

	if (hdr.depth != 8 && hdr.depth != 4) {
		Output::Warning("BMP image depth unsupported: {} bit.", hdr.depth);
		return false;
	}

	if (hdr.compression) {
		Output::Warning("BMP image is compressed.");
		return false;
	}

	auto* palette = ptr;

	auto get_palette = [&](int idx) {
		return palette + idx * hdr.palette_size;
	};

	// Ensure no palette entry is an exact duplicate of the transparent color at #0
	for (int i = 1; i < hdr.num_colors; i++) {
		auto* p = get_palette(i);
		if (p[0] == palette[0] && p[1] == palette[1] && p[2] == palette[2]) {
			// FIXME: Remove the need for this const_cast
			const_cast<uint8_t*>(p)[0] ^= 1;
		}
	}

	const uint8_t* src_pixels = &data[bits_offset];

	// bitmap scan lines need to be aligned to 32 bit boundaries, add padding if needed
	int line_width = (hdr.depth == 4) ? (hdr.w + 1) >> 1 : hdr.w;
	int padding = (-line_width)&3;

	output.pixels = malloc(hdr.w * hdr.h * 4);
	if (!output.pixels) {
		Output::Warning("Error allocating BMP pixel buffer.");
		return false;
	}

	uint8_t* dst = (uint8_t*) output.pixels;
	for (int y = 0; y < hdr.h; y++) {
		const uint8_t* src = src_pixels + (vflip ? hdr.h - 1 - y : y) * (line_width + padding);
		for (int x = 0; x < hdr.w; x += 2) {
			uint8_t pix = *src++;
			uint8_t pix2 = 0;
			const uint8_t* color;

			// split up packed pixel
			if (hdr.depth == 4) {
				pix2 = pix & 15;
				pix >>= 4;
			}

			color = get_palette(pix);
			*dst++ = color[2];
			*dst++ = color[1];
			*dst++ = color[0];
			*dst++ = (transparent && pix == 0) ? 0 : 255;

			// end of line
			if (x + 1 == hdr.w)
				break;

			pix = (hdr.depth == 8) ? *src++ : pix2;

			color = get_palette(pix);
			*dst++ = color[2];
			*dst++ = color[1];
			*dst++ = color[0];
			*dst++ = (transparent && pix == 0) ? 0 : 255;
		}
	}

	output.width = hdr.w;
	output.height = hdr.h;
	output.bpp = hdr.depth; // Currently only 4 and 8 bit (indexed) are supported
	return true;
}

bool ImageBMP::Read(Filesystem_Stream::InputStream& stream, bool transparent, ImageOut& output) {
	std::vector<uint8_t> buffer = Utils::ReadStream(stream);
	return Read(&buffer.front(), (unsigned) buffer.size(), transparent, output);
}
