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
#include <cstring>
#include <csetjmp>
#include <vector>

#include "output.h"
#include "image_png.h"

static void read_data(png_structp png_ptr, png_bytep data, png_size_t length) {
    png_bytep* bufp = (png_bytep*) png_get_io_ptr(png_ptr);
	memcpy(data, *bufp, length);
	*bufp += length;
}

static void on_png_warning(png_structp, png_const_charp warn_msg) {
	Output::Debug("libpng: %s", warn_msg);
}

static void on_png_error(png_structp, png_const_charp error_msg) {
	Output::Warning("libpng: %s", error_msg);
}

static void ReadPalettedData(png_struct*, png_info*, png_uint_32, png_uint_32, bool, uint32_t*);
static void ReadGrayData(png_struct*, png_info*, png_uint_32, png_uint_32, bool, uint32_t*);
static void ReadGrayAlphaData(png_struct*, png_info*, png_uint_32, png_uint_32, uint32_t*);
static void ReadRGBData(png_struct*, png_info*, png_uint_32, png_uint_32, uint32_t*);
static void ReadRGBAData(png_struct*, png_info*, png_uint_32, png_uint_32, uint32_t*);

bool ImagePNG::ReadPNG(FILE* stream, const void* buffer, bool transparent,
					   int& width, int& height, void*& pixels) {
	pixels = nullptr;

	png_struct *png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, on_png_error, on_png_warning);
	if (png_ptr == NULL) {
		Output::Warning("Couldn't allocate PNG structure");
		return false;
	}

	png_info *info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		Output::Warning("Couldn't allocate PNG info structure");
		return false;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return false;
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

	pixels = malloc(w * h * 4);
	if (!pixels) {
		Output::Warning("Error allocating PNG pixel buffer.");
		return false;
	}

	switch (color_type) {
		case PNG_COLOR_TYPE_PALETTE:
			ReadPalettedData(png_ptr, info_ptr, w, h, transparent, (uint32_t*)pixels);
			break;
		case PNG_COLOR_TYPE_GRAY:
			ReadGrayData(png_ptr, info_ptr, w, h, transparent, (uint32_t*)pixels);
			break;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			ReadGrayAlphaData(png_ptr, info_ptr, w, h, (uint32_t*)pixels);
			break;
		case PNG_COLOR_TYPE_RGB:
			ReadRGBData(png_ptr, info_ptr, w, h, (uint32_t*)pixels);
			break;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			ReadRGBAData(png_ptr, info_ptr, w, h, (uint32_t*)pixels);
			break;
	}

	png_read_end(png_ptr, NULL);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	width = w;
	height = h;
	return true;
}

static void ReadPalettedData(
	png_struct* png_ptr, png_info* info_ptr,
	png_uint_32 w, png_uint_32 h,
	bool transparent,
	uint32_t* pixels
) {
	// For transparent images, all the colors are opaque, except the
	// color with index 0. So we'll need to do index->RGB conversion
	// on our own.
	if (transparent) {
		png_set_packing(png_ptr);
		png_read_update_info(png_ptr, info_ptr);

		if (!png_get_valid(png_ptr, info_ptr, PNG_INFO_PLTE)) {
			Output::Warning("Palette PNG without PLTE block");
			return;
		}

		png_colorp palette;
		int num_palette;
		png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);

		for (png_uint_32 y = 0; y < h; y++) {
			// We read the indices (w bytes) into the end of the pixel
			// data for this row (4w bytes), then scan over them
			// converting them into RGBA values. Putting them at the end
			// gives us enough room that we don't overwrite an index
			// we'll need later with an RGBA value.

			uint32_t* beginning_of_row = pixels + y * w;

			uint8_t* indices = (uint8_t*)beginning_of_row + w * 3;
			png_read_row(png_ptr, (png_bytep)indices, NULL);

			uint32_t* dst = beginning_of_row;
			for (png_uint_32 x = 0; x < w; x++, dst++) {
				uint8_t idx = indices[x];
				png_color& color = palette[idx];
				uint8_t alpha = idx == 0 ? 0 : 255;
				uint8_t rgba[4] = { color.red, color.green, color.blue, alpha };
				*dst = *(uint32_t*)rgba;
			}
		}
	}
	// Otherwise, libpng can convert to RGBA on its own
	else {
		png_set_palette_to_rgb(png_ptr);
		png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
		png_read_update_info(png_ptr, info_ptr);

		for (png_uint_32 y = 0; y < h; y++) {
			png_bytep dst = (png_bytep) pixels + y * w * 4;
			png_read_row(png_ptr, dst, NULL);
		}
	}
}

static void ReadGrayData(
	png_struct* png_ptr, png_info* info_ptr,
	png_uint_32 w, png_uint_32 h,
	bool transparent,
	uint32_t* pixels
) {
	png_set_strip_16(png_ptr);
	png_set_expand(png_ptr);
	png_set_gray_to_rgb(png_ptr);
	png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
	png_read_update_info(png_ptr, info_ptr);

	for (png_uint_32 y = 0; y < h; y++) {
		png_bytep dst = (png_bytep) pixels + y * w * 4;
		png_read_row(png_ptr, dst, NULL);
	}

	// Black pixels are transparent
	if (transparent) {
		uint8_t ck1[4] = {0, 0, 0, 255};
		uint8_t ck2[4] = {0, 0, 0,   0};
		uint32_t srckey = *(uint32_t*)ck1;
		uint32_t dstkey = *(uint32_t*)ck2;
		uint32_t* p = (uint32_t*) pixels;
		for (unsigned i = 0; i < w * h; i++, p++)
			if (*p == srckey)
				*p = dstkey;
	}
}

static void ReadGrayAlphaData(
	png_struct* png_ptr, png_info* info_ptr,
	png_uint_32 w, png_uint_32 h,
	uint32_t* pixels
) {
	png_set_strip_16(png_ptr);
	png_set_gray_to_rgb(png_ptr);
	png_read_update_info(png_ptr, info_ptr);

	for (png_uint_32 y = 0; y < h; y++) {
		png_bytep dst = (png_bytep) pixels + y * w * 4;
		png_read_row(png_ptr, dst, NULL);
	}
}


static void ReadRGBData(
	png_struct* png_ptr, png_info* info_ptr,
	png_uint_32 w, png_uint_32 h,
	uint32_t* pixels
) {
	png_set_strip_16(png_ptr);
	png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
	png_read_update_info(png_ptr, info_ptr);

	for (png_uint_32 y = 0; y < h; y++) {
		png_bytep dst = (png_bytep) pixels + y * w * 4;
		png_read_row(png_ptr, dst, NULL);
	}
}

static void ReadRGBAData(
	png_struct* png_ptr, png_info* info_ptr,
	png_uint_32 w, png_uint_32 h,
	uint32_t* pixels
) {
	png_set_strip_16(png_ptr);
	png_read_update_info(png_ptr, info_ptr);

	for (png_uint_32 y = 0; y < h; y++) {
		png_bytep dst = (png_bytep) pixels + y * w * 4;
		png_read_row(png_ptr, dst, NULL);
	}
}

static void write_data(png_structp out_ptr, png_bytep data, png_size_t len) {
	reinterpret_cast<std::ostream*>(png_get_io_ptr(out_ptr))->write(reinterpret_cast<char const*>(data), len);
}
static void flush_stream(png_structp out_ptr) {
	reinterpret_cast<std::ostream*>(png_get_io_ptr(out_ptr))->flush();
}

bool ImagePNG::WritePNG(std::ostream& os, uint32_t width, uint32_t height, uint32_t* data) {
	for (size_t i = 0; i < width * height; ++i) {
		uint32_t const p = data[i];
		uint8_t* out = reinterpret_cast<uint8_t*>(&data[i]);
		uint8_t
			a = (p >> 24) & 0xff, r = (p >> 16) & 0xff,
			g = (p >>  8) & 0xff, b = (p >>  0) & 0xff;
		if(a != 0) {
			r = (r * 255) / a;
			g = (g * 255) / a;
			b = (b * 255) / a;
		}
		*out++ = r; *out++ = g; *out++ = b; *out++ = a;
	}

	png_structp write = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!write) {
		Output::Warning("Bitmap::WritePNG: error in png_create_write");
		return false;
	}

	png_infop info = png_create_info_struct(write);
	if (!info) {
		png_destroy_write_struct(&write, &info);
		Output::Warning("ImagePNG::WritePNG: error in png_create_info_struct");
		return false;
	}

	png_bytep* ptrs = new png_bytep[height];
	for (size_t i = 0; i < height; ++i) {
		ptrs[i] = reinterpret_cast<png_bytep>(&data[width*i]);
	}

	if (setjmp(png_jmpbuf(write))) {
		png_destroy_write_struct(&write, &info);
		delete [] ptrs;
		Output::Warning("ImagePNG::WritePNG: error writing PNG file");
		return false;
	}

	png_set_write_fn(write, &os, &write_data, &flush_stream);

	png_set_IHDR(write, info, width, height, 8,
				 PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
				 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_write_info(write, info);
	png_write_image(write, ptrs);
	png_write_end(write, NULL);

	png_destroy_write_struct(&write, &info);
	delete [] ptrs;

	return true;
}

#endif // SUPPORT_PNG
