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

#include "system.h"
#ifdef SUPPORT_PNG

// Headers
#include <png.h>
#include <cstdlib>
#include "output.h"
#include "image_png.h"

static void read_data(png_structp png_ptr, png_bytep data, png_size_t length) {
    png_bytep* bufp = (png_bytep*) png_get_io_ptr(png_ptr);
	memcpy(data, *bufp, length);
	*bufp += length;
}

void ImagePNG::ReadPNG(FILE* stream, const void* buffer, bool transparent,
					int& width, int& height, void*& pixels) {
	pixels = NULL;

	png_struct *png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		Output::Error("Couldn't allocate PNG structure");
		return;
	}

	png_info *info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		Output::Error("Couldn't allocate PNG info structure");
		return;
	}

	if (stream != NULL)
		png_init_io(png_ptr, stream);
	else
		png_set_read_fn(png_ptr, (png_voidp) &buffer, read_data);

	png_read_info(png_ptr, info_ptr);

	png_uint_32 w, h;
	int bit_depth, color_type;
	png_get_IHDR(png_ptr, info_ptr, &w, &h,
				 &bit_depth, &color_type, NULL, NULL, NULL);

	png_color black = {0,0,0};
	png_colorp palette = NULL;
	int num_palette = 0;

	switch (color_type) {
		case PNG_COLOR_TYPE_PALETTE:
			if (!png_get_valid(png_ptr, info_ptr, PNG_INFO_PLTE)) {
				Output::Error("Palette PNG without PLTE block");
			}
			if (transparent) {
				png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);
			}
			png_set_strip_alpha(png_ptr);
			png_set_palette_to_rgb(png_ptr);
			png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
			break;
		case PNG_COLOR_TYPE_GRAY:
			png_set_gray_to_rgb(png_ptr);
			if (bit_depth < 8)
				png_set_expand_gray_1_2_4_to_8(png_ptr);
			png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
			if (transparent) {
				palette = &black;
				num_palette = 1;
			}
			break;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			png_set_gray_to_rgb(png_ptr);
			if (bit_depth < 8)
				png_set_expand_gray_1_2_4_to_8(png_ptr);
			break;
		case PNG_COLOR_TYPE_RGB:
			png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
			break;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			break;
	}

	if (bit_depth < 8)
		png_set_packing(png_ptr);

	if (bit_depth == 16)
		png_set_strip_16(png_ptr);

	png_read_update_info(png_ptr, info_ptr);

	width = w;
	height = h;
	pixels = malloc(w * h * 4);

	for (png_uint_32 y = 0; y < h; y++) {
		png_bytep dst = (png_bytep) pixels + y * w * 4;
		png_read_row(png_ptr, dst, NULL);
	}

	if (transparent && num_palette > 0) {
		png_color& ck = palette[0];
		uint8_t ck1[4] = {ck.red, ck.green, ck.blue, 255};
		uint8_t ck2[4] = {ck.red, ck.green, ck.blue,   0};
		uint32_t srckey = *(uint32_t*)ck1;
		uint32_t dstkey = *(uint32_t*)ck2;
		uint32_t* p = (uint32_t*) pixels;
		for (unsigned i = 0; i < w * h; i++, p++)
			if (*p == srckey)
				*p = dstkey;
	}

	png_read_end(png_ptr, NULL);

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
}

#endif // SUPPORT_PNG
