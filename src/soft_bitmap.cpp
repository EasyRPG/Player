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

#include "system.h"
#ifdef USE_SOFT_BITMAP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <cstdlib>
#include <iostream>
#include "cache.h"
#include "filefinder.h"
#include "options.h"
#include "data.h"
#include "output.h"
#include "utils.h"
#include "image_xyz.h"
#include "image_png.h"
#include "text.h"
#include "bitmap_utils.h"
#include "soft_bitmap.h"

////////////////////////////////////////////////////////////

const DynamicFormat SoftBitmap::dynamic_format = SoftBitmap::pixel_format::Format(DynamicFormat());

////////////////////////////////////////////////////////////
void SoftBitmap::Init(int width, int height) {
	w = width;
	h = height;
	bitmap = calloc(h, w * 4);

	if (bitmap == NULL) {
		Output::Error("Couldn't create %dx%d image.\n", w, h);
	}
}

////////////////////////////////////////////////////////////
void SoftBitmap::ConvertImage(int& width, int& height, void*& pixels) {
	uint8* dst = (uint8*) pixels;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			uint8 r, g, b, a;
			image_format::get_rgba(dynamic_format, dst, r, g, b, a);
			pixel_format::set_rgba(dynamic_format, dst, r, g, b, a);
			dst += 4;
		}
	}
}

////////////////////////////////////////////////////////////
SoftBitmap::SoftBitmap(int width, int height, bool itransparent) {
	transparent = itransparent;
	bm_utils = new BitmapUtilsT<pixel_format>(dynamic_format);
	Init(width, height);
}

SoftBitmap::SoftBitmap(const std::string& filename, bool itransparent, uint32 flags) {
	transparent = itransparent;
	bm_utils = new BitmapUtilsT<pixel_format>(dynamic_format);

	int namelen = (int) filename.size();
	if (namelen < 5 || filename[namelen - 4] != '.') {
		Output::Error("Invalid extension for image file %s", filename.c_str());
		return;
	}

	std::string ext = Utils::LowerCase(filename.substr(namelen - 3, 3));
	if (ext != "png" && ext != "xyz") {
		Output::Error("Unsupported image file %s", filename.c_str());
		return;
	}

	FILE* stream = FileFinder::fopenUTF8(filename, "rb");
	if (!stream) {
		Output::Error("Couldn't open image file %s", filename.c_str());
		return;
	}
	if (ext == "png")
		ImagePNG::ReadPNG(stream, (const void*) NULL, transparent, w, h, bitmap);
	else if (ext == "xyz")
		ImageXYZ::ReadXYZ(stream, transparent, w, h, bitmap);

	ConvertImage(w, h, bitmap);

	fclose(stream);

	CheckPixels(flags);
}

SoftBitmap::SoftBitmap(const uint8* data, uint bytes, bool itransparent, uint32 flags) {
	transparent = itransparent;
	bm_utils = new BitmapUtilsT<pixel_format>(dynamic_format);

	if (bytes > 4 && strncmp((char*) data, "XYZ1", 4) == 0)
		ImageXYZ::ReadXYZ(data, bytes, transparent, w, h, bitmap);
	else
		ImagePNG::ReadPNG((FILE*) NULL, (const void*) data, transparent, w, h, bitmap);

	ConvertImage(w, h, bitmap);

	CheckPixels(flags);
}

SoftBitmap::SoftBitmap(Bitmap* source, Rect src_rect, bool itransparent) {
	transparent = itransparent;
	bm_utils = new BitmapUtilsT<pixel_format>(dynamic_format);

	Init(src_rect.width, src_rect.height);

	Blit(0, 0, source, src_rect, 255);
}

////////////////////////////////////////////////////////////
SoftBitmap::~SoftBitmap() {
	free(bitmap);
	delete bm_utils;
}

////////////////////////////////////////////////////////////
void* SoftBitmap::pixels() {
	return bitmap;
}

uint8 SoftBitmap::bpp() const {
	return pixel_format::bytes;
}

int SoftBitmap::width() const {
	return w;
}

int SoftBitmap::height() const {
	return h;
}

uint16 SoftBitmap::pitch() const {
	return width() * bpp();
}

uint32 SoftBitmap::rmask() const {
	return pixel_format::r_mask(dynamic_format);
}

uint32 SoftBitmap::gmask() const {
	return pixel_format::g_mask(dynamic_format);
}

uint32 SoftBitmap::bmask() const {
	return pixel_format::b_mask(dynamic_format);
}

uint32 SoftBitmap::amask() const {
	return pixel_format::a_mask(dynamic_format);
}

uint32 SoftBitmap::colorkey() const {
	return 0;
}

////////////////////////////////////////////////////////////
void SoftBitmap::SetTransparentColor(Color color) {
}

////////////////////////////////////////////////////////////
Color SoftBitmap::GetColor(uint32 uint32_color) const {
	uint8 r, g, b, a;
	pixel_format::uint32_to_rgba(dynamic_format, uint32_color, r, g, b, a);
	return Color(r, g, b, a);
}

uint32 SoftBitmap::GetUint32Color(const Color &color) const {
	return pixel_format::rgba_to_uint32(dynamic_format, color.red, color.green, color.blue, color.alpha);
}

uint32 SoftBitmap::GetUint32Color(uint8 r, uint8 g, uint8 b, uint8 a) const {
	return pixel_format::rgba_to_uint32(dynamic_format, r, g, b, a);
}

void SoftBitmap::GetColorComponents(uint32 color, uint8 &r, uint8 &g, uint8 &b, uint8 &a) const {
	pixel_format::uint32_to_rgba(dynamic_format, color, r, g, b, a);
}

////////////////////////////////////////////////////////////
void SoftBitmap::Lock() {
}

void SoftBitmap::Unlock() {
}

#endif

