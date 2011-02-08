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
#include "image_bmp.h"
#include "image_png.h"
#include "text.h"
#include "bitmap_utils.h"
#include "soft_bitmap.h"

////////////////////////////////////////////////////////////

DynamicFormat SoftBitmap::pixel_format;
DynamicFormat SoftBitmap::opaque_pixel_format;
DynamicFormat SoftBitmap::image_format;
DynamicFormat SoftBitmap::opaque_image_format;

////////////////////////////////////////////////////////////
void SoftBitmap::SetFormat(const DynamicFormat& format) {
	pixel_format = format;
	opaque_pixel_format = format;
	opaque_pixel_format.alpha_type = PF::NoAlpha;
	image_format = format_R8G8B8A8_a().format();
	opaque_image_format = format_R8G8B8A8_n().format();
}

////////////////////////////////////////////////////////////
DynamicFormat SoftBitmap::ChooseFormat(const DynamicFormat& format) {
	uint32 amask;
	PF::AlphaType atype;
	amask = (format.a.mask == 0)
		? ((~0U >> (32 - format.bits)) ^ (format.r.mask | format.g.mask | format.b.mask))
		: format.a.mask;
	atype = (amask == 0) ? PF::ColorKey : PF::Alpha;
		
	return DynamicFormat(format.bits,
						 format.r.mask, format.g.mask, format.b.mask,
						 amask, atype);
}

////////////////////////////////////////////////////////////
void SoftBitmap::Init(int width, int height, void *data = NULL, int pitch, bool _destroy) {
	w = width;
	h = height;
	_pitch = (pitch != 0) ? pitch : w * format.bytes;
	bitmap = (data != NULL)  ? data : malloc(h * _pitch);
	destroy = _destroy;

	if (bitmap == NULL) {
		Output::Error("Couldn't create %dx%d image.\n", w, h);
	}
}

////////////////////////////////////////////////////////////
void SoftBitmap::ConvertImage(int& width, int& height, void*& pixels, bool transparent) {
	Init(width, height, NULL);
	const DynamicFormat& img_format = transparent ? image_format : opaque_image_format;
	SoftBitmap src(pixels, width, height, 0, img_format);
	Clear();
	Blit(0, 0, &src, src.GetRect(), 255);
	free(pixels);
}

////////////////////////////////////////////////////////////
SoftBitmap::SoftBitmap(int width, int height, bool transparent) {
	format = transparent ? pixel_format : opaque_pixel_format;
	Init(width, height);
	Clear();
}

SoftBitmap::SoftBitmap(void *pixels, int width, int height, int pitch, const DynamicFormat& _format) {
	format = _format;
	Init(width, height, pixels, pitch, false);
}

SoftBitmap::SoftBitmap(const std::string& filename, bool transparent, uint32 flags) {
	format = transparent ? pixel_format : opaque_pixel_format;

	int namelen = (int) filename.size();
	if (namelen < 5 || filename[namelen - 4] != '.') {
		Output::Error("Invalid extension for image file %s", filename.c_str());
		return;
	}

	std::string ext = Utils::LowerCase(filename.substr(namelen - 3, 3));
	if (ext != "png" && ext != "xyz" && ext != "bmp") {
		Output::Error("Unsupported image file %s", filename.c_str());
		return;
	}

	FILE* stream = FileFinder::fopenUTF8(filename, "rb");
	if (!stream) {
		Output::Error("Couldn't open image file %s", filename.c_str());
		return;
	}

	int width, height;
	void* pixels;

	if (ext == "png")
		ImagePNG::ReadPNG(stream, (const void*) NULL, transparent, width, height, pixels);
	else if (ext == "xyz")
		ImageXYZ::ReadXYZ(stream, transparent, width, height, pixels);
	else if (ext == "bmp")
		ImageBMP::ReadBMP(stream, transparent, width, height, pixels);

	ConvertImage(width, height, pixels, transparent);

	fclose(stream);

	CheckPixels(flags);
}

SoftBitmap::SoftBitmap(const uint8* data, uint bytes, bool transparent, uint32 flags) {
	format = transparent ? pixel_format : opaque_pixel_format;

	int width, height;
	void* pixels;

	if (bytes > 4 && strncmp((char*) data, "XYZ1", 4) == 0)
		ImageXYZ::ReadXYZ(data, bytes, transparent, width, height, pixels);
	else if (bytes > 2 && strncmp((char*) data, "BM", 2) == 0)
		ImageBMP::ReadBMP(data, bytes, transparent, width, height, pixels);
	else
		ImagePNG::ReadPNG((FILE*) NULL, (const void*) data, transparent, width, height, pixels);

	ConvertImage(width, height, pixels, transparent);

	CheckPixels(flags);
}

SoftBitmap::SoftBitmap(Bitmap* source, Rect src_rect, bool transparent) {
	format = transparent ? pixel_format : opaque_pixel_format;
	Init(src_rect.width, src_rect.height);
	Clear();
	Blit(0, 0, source, src_rect, 255);
}

////////////////////////////////////////////////////////////
SoftBitmap::~SoftBitmap() {
	if (destroy)
		free(bitmap);
}

////////////////////////////////////////////////////////////
void* SoftBitmap::pixels() {
	return bitmap;
}

int SoftBitmap::width() const {
	return w;
}

int SoftBitmap::height() const {
	return h;
}

uint16 SoftBitmap::pitch() const {
	return _pitch;
}

#endif

