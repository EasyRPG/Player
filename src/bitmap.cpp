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
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <iostream>

#include "utils.h"
#include "cache.h"
#include "bitmap.h"
#include "filefinder.h"
#include "options.h"
#include "data.h"
#include "output.h"
#include "image_xyz.h"
#include "image_bmp.h"
#include "image_png.h"
#include "transform.h"
#include "font.h"
#include "output.h"
#include "util_macro.h"
#include "bitmap_hslrgb.h"

const Opacity Opacity::opaque;

BitmapRef Bitmap::Create(int width, int height, const Color& color) {
    BitmapRef surface = Bitmap::Create(width, height, false);
	surface->Fill(color);
	return surface;
}

BitmapRef Bitmap::Create(const std::string& filename, bool transparent, uint32_t flags) {
	BitmapRef bmp = std::make_shared<Bitmap>(filename, transparent, flags);

	if (!bmp->pixels()) {
		return BitmapRef();
	}

	return bmp;
}

BitmapRef Bitmap::Create(const uint8_t* data, unsigned bytes, bool transparent, uint32_t flags) {
	BitmapRef bmp = std::make_shared<Bitmap>(data, bytes, transparent, flags);

	if (!bmp->pixels()) {
		return BitmapRef();
	}

	return bmp;
}

BitmapRef Bitmap::Create(Bitmap const& source, Rect const& src_rect, bool transparent) {
	return std::make_shared<Bitmap>(source, src_rect, transparent);
}

BitmapRef Bitmap::Create(int width, int height, bool transparent, int /* bpp */) {
	return std::make_shared<Bitmap>(width, height, transparent);
}

BitmapRef Bitmap::Create(void *pixels, int width, int height, int pitch, const DynamicFormat& format) {
	return std::make_shared<Bitmap>(pixels, width, height, pitch, format);
}

Bitmap::Bitmap(int width, int height, bool transparent) {
	font = Font::Default();
	format = (transparent ? pixel_format : opaque_pixel_format);
	pixman_format = find_format(format);
	Init(width, height, (void *) NULL);
}

Bitmap::Bitmap(void *pixels, int width, int height, int pitch, const DynamicFormat& _format) {
	font = Font::Default();
	format = _format;
	pixman_format = find_format(format);
	Init(width, height, pixels, pitch, false);
}

Bitmap::Bitmap(const std::string& filename, bool transparent, uint32_t flags) {
	font = Font::Default();
	format = (transparent ? pixel_format : opaque_pixel_format);
	pixman_format = find_format(format);

	FILE* stream = FileFinder::fopenUTF8(filename, "rb");
	if (!stream) {
		Output::Error("Couldn't open image file %s", filename.c_str());
		return;
	}

	int w = 0;
	int h = 0;
	void* pixels;

	char data[4];
	size_t bytes = fread(&data, 1, 4, stream);
	fseek(stream, 0, SEEK_SET);

	bool img_okay = false;

#ifdef SUPPORT_XYZ
	if (bytes >= 4 && strncmp((char*)data, "XYZ1", 4) == 0)
		img_okay = ImageXYZ::ReadXYZ(stream, transparent, w, h, pixels);
	else
#endif
#ifdef SUPPORT_BMP
	if (bytes > 2 && strncmp((char*)data, "BM", 2) == 0)
		img_okay = ImageBMP::ReadBMP(stream, transparent, w, h, pixels);
	else
#endif
#ifdef SUPPORT_PNG
	if (bytes >= 4 && strncmp((char*)(data + 1), "PNG", 3) == 0)
		img_okay = ImagePNG::ReadPNG(stream, (void*)NULL, transparent, w, h, pixels);
	else
#endif
		Output::Warning("Unsupported image file %s", filename.c_str());

	fclose(stream);

	if (!img_okay) {
		free(pixels);

		pixels = nullptr;

		return;
	}

	Init(w, h, (void *) NULL);

	ConvertImage(w, h, pixels, transparent);

	CheckPixels(flags);
}

Bitmap::Bitmap(const uint8_t* data, unsigned bytes, bool transparent, uint32_t flags) {
	font = Font::Default();
	format = (transparent ? pixel_format : opaque_pixel_format);
	pixman_format = find_format(format);

	int w = 0, h = 0;
	void* pixels;

	bool img_okay = false;

#ifdef SUPPORT_XYZ
	if (bytes > 4 && strncmp((char*) data, "XYZ1", 4) == 0)
		img_okay = ImageXYZ::ReadXYZ(data, bytes, transparent, w, h, pixels);
	else
#endif
#ifdef SUPPORT_BMP
	if (bytes > 2 && strncmp((char*) data, "BM", 2) == 0)
		img_okay = ImageBMP::ReadBMP(data, bytes, transparent, w, h, pixels);
	else
#endif
#ifdef SUPPORT_PNG
	if (bytes > 4 && strncmp((char*)(data + 1), "PNG", 3) == 0)
		img_okay = ImagePNG::ReadPNG((FILE*) NULL, (const void*) data, transparent, w, h, pixels);
	else
#endif
		Output::Warning("Unsupported image");

	if (!img_okay) {
		return;
	}

	Init(w, h, (void *) NULL);

	ConvertImage(w, h, pixels, transparent);

	CheckPixels(flags);
}

Bitmap::Bitmap(Bitmap const& source, Rect const& src_rect, bool transparent) {
	font = Font::Default();
	format = (transparent ? pixel_format : opaque_pixel_format);
	pixman_format = find_format(format);

	Init(src_rect.width, src_rect.height, (void *) NULL);

	Blit(0, 0, source, src_rect, Opacity::opaque);
}

Bitmap::~Bitmap() {
	if (bitmap) {
		pixman_image_unref(bitmap);
	}
}

bool Bitmap::WritePNG(std::ostream& os) const {
#ifdef SUPPORT_PNG
	size_t const width = GetWidth(), height = GetHeight();
	size_t const stride = width * 4;

	std::vector<uint32_t> data(width * height);

	std::shared_ptr<pixman_image_t> dst
		(pixman_image_create_bits(PIXMAN_a8r8g8b8, width, height, &data.front(), stride),
		 pixman_image_unref);
	pixman_image_composite32(PIXMAN_OP_SRC, bitmap, NULL, dst.get(),
							 0, 0, 0, 0, 0, 0, width, height);

	return ImagePNG::WritePNG(os, width, height, &data.front());
#else
	return false;
#endif
}

int Bitmap::GetWidth() const {
	return width();
}

int Bitmap::GetHeight() const {
	return height();
}

Rect Bitmap::GetRect() const {
	return Rect(0, 0, width(), height());
}

bool Bitmap::GetTransparent() const {
	return format.alpha_type != PF::NoAlpha;
}

Bitmap::TileOpacity Bitmap::CheckOpacity(const Rect& rect) {
	bool all = true;
	bool any = false;

	DynamicFormat format(32,8,24,8,16,8,8,8,0,PF::Alpha);
	std::vector<uint32_t> pixels;
	pixels.resize(rect.width * rect.height);
	Bitmap bmp(reinterpret_cast<void*>(&pixels.front()), rect.width, rect.height, rect.width*4, format);
	bmp.Blit(0, 0, *this, rect, Opacity::opaque);

	for (std::vector<uint32_t>::const_iterator p = pixels.begin(); p != pixels.end(); ++p) {
		if ((*p & 0xFF) != 0)
			any = true;
		else
			all = false;
		if (any && !all)
			break;
	}

	return
		all ? Bitmap::Opaque :
		any ? Bitmap::Partial :
		Bitmap::Transparent;
}

void Bitmap::CheckPixels(uint32_t flags) {
	if (flags & Flag_System) {
		DynamicFormat format(32,8,24,8,16,8,8,8,0,PF::Alpha);
		uint32_t pixel;
		Bitmap bmp(reinterpret_cast<void*>(&pixel), 1, 1, 4, format);
		pixman_image_composite32(PIXMAN_OP_SRC, bitmap, (pixman_image_t*) NULL, bmp.bitmap,
								 0, 32,  0, 0,  0, 0,  1, 1);
		bg_color = Color((int)(pixel>>24)&0xFF, (int)(pixel>>16)&0xFF, (int)(pixel>>8)&0xFF, (int)pixel&0xFF);
		pixman_image_composite32(PIXMAN_OP_SRC, bitmap, (pixman_image_t*) NULL, bmp.bitmap,
								 16, 32,  0, 0,  0, 0,  1, 1);
		sh_color = Color((int)(pixel>>24)&0xFF, (int)(pixel>>16)&0xFF, (int)(pixel>>8)&0xFF, (int)pixel&0xFF);
	}

	if (flags & Flag_Chipset) {
		tile_opacity.clear();
		tile_opacity.resize(height() / 16);
		for (int row = 0; row < height() / 16; row++) {
			tile_opacity[row].resize(width() / 16);
			for (int col = 0; col < width() / 16; col++) {
				Rect rect(col * 16, row * 16, 16, 16);
				tile_opacity[row][col] = CheckOpacity(rect);
			}
		}
	}

	if (flags & Flag_ReadOnly) {
		read_only = true;

		opacity = CheckOpacity(GetRect());
	}
}

Bitmap::TileOpacity Bitmap::GetOpacity() const {
	return opacity;
}

Bitmap::TileOpacity Bitmap::GetTileOpacity(int row, int col) const {
	return !tile_opacity.empty() ? tile_opacity[row][col] : Partial;
}

Color Bitmap::GetBackgroundColor() const {
	return bg_color;
}

Color Bitmap::GetShadowColor() const {
	return sh_color;
}

void Bitmap::HueChangeBlit(int x, int y, Bitmap const& src, Rect const& src_rect_, double hue_) {
	Rect dst_rect(x, y, 0, 0), src_rect = src_rect_;

	if (!Rect::AdjustRectangles(src_rect, dst_rect, src.GetRect()))
		return;
	if (!Rect::AdjustRectangles(dst_rect, src_rect, GetRect()))
		return;

	int hue  = (int) (hue_ / 60.0 * 0x100);
	if (hue < 0)
		hue += ((-hue + 0x5FF) / 0x600) * 0x600;
	else if (hue > 0x600)
		hue -= (hue / 0x600) * 0x600;

	DynamicFormat format(32,8,24,8,16,8,8,8,0,PF::Alpha);
	std::vector<uint32_t> pixels;
	pixels.resize(src_rect.width * src_rect.height);
	Bitmap bmp(reinterpret_cast<void*>(&pixels.front()), src_rect.width, src_rect.height, src_rect.width * 4, format);
	bmp.Blit(0, 0, src, src_rect, Opacity::opaque);

	for (std::vector<uint32_t>::iterator p = pixels.begin(); p != pixels.end(); ++p) {
		uint32_t pixel = *p;
		uint8_t r = (pixel>>24) & 0xFF;
		uint8_t g = (pixel>>16) & 0xFF;
		uint8_t b = (pixel>> 8) & 0xFF;
		uint8_t a = pixel & 0xFF;
		if (a > 0)
			RGB_adjust_HSL(r, g, b, hue);
		*p = ((uint32_t) r << 24) | ((uint32_t) g << 16) | ((uint32_t) b << 8) | (uint32_t) a;
	}

	Blit(dst_rect.x, dst_rect.y, bmp, bmp.GetRect(), Opacity::opaque);
}

FontRef const& Bitmap::GetFont() const {
	return font;
}

void Bitmap::SetFont(FontRef const& new_font) {
	font = new_font;
}

void Bitmap::TextDraw(int x, int y, int width, int /* height */, int color, std::string const& text, Text::Alignment align) {
	Rect rect = GetFont()->GetSize(text);
	int dx = rect.width - width;

	switch (align) {
	case Text::AlignLeft:
		TextDraw(x, y, color, text);
		break;
	case Text::AlignCenter:
		TextDraw(x + dx / 2, y, color, text);
		break;
	case Text::AlignRight:
		TextDraw(x + dx, y, color, text);
		break;
	default: assert(false);
	}
}

void Bitmap::TextDraw(Rect const& rect, int color, std::string const& text, Text::Alignment align) {
	TextDraw(rect.x, rect.y, rect.width, rect.height, color, text, align);
}

void Bitmap::TextDraw(int x, int y, int color, std::string const& text, Text::Alignment align) {
	Text::Draw(*this, x, y, color, text, align);
}

void Bitmap::TextDraw(int x, int y, int width, int /* height */, Color color, std::string const& text, Text::Alignment align) {
	Rect rect = GetFont()->GetSize(text);
	int dx = rect.width - width;

	switch (align) {
	case Text::AlignLeft:
		TextDraw(x, y, color, text);
		break;
	case Text::AlignCenter:
		TextDraw(x + dx / 2, y, color, text);
		break;
	case Text::AlignRight:
		TextDraw(x + dx, y, color, text);
		break;
	default: assert(false);
	}
}

void Bitmap::TextDraw(Rect const& rect, Color color, std::string const& text, Text::Alignment align) {
	TextDraw(rect.x, rect.y, rect.width, rect.height, color, text, align);
}

void Bitmap::TextDraw(int x, int y, Color color, std::string const& text) {
	Text::Draw(*this, x, y, color, text);
}

Rect Bitmap::TransformRectangle(const Transform& xform, const Rect& rect) {
	pixman_box16 bounds = {
		static_cast<int16_t>(rect.x),
		static_cast<int16_t>(rect.y),
		static_cast<int16_t>(rect.x + rect.width),
		static_cast<int16_t>(rect.y + rect.height)
	};

	pixman_transform_bounds(&xform.matrix, &bounds);
	return Rect(bounds.x1, bounds.y1, bounds.x2 - bounds.x1, bounds.y2 - bounds.y1);
}

bool Bitmap::formats_initialized = false;
std::map<int, pixman_format_code_t> Bitmap::formats_map;

void Bitmap::add_pair(pixman_format_code_t pcode, const DynamicFormat& format) {
	int dcode = format.code_alpha();
	formats_map[dcode] = pcode;
}

void Bitmap::initialize_formats() {
	if (formats_initialized)
		return;

	add_pair(PIXMAN_a8r8g8b8, DynamicFormat(32,8,16,8,8,8,0,8,24,PF::Alpha));
	add_pair(PIXMAN_x8r8g8b8, DynamicFormat(32,8,16,8,8,8,0,0,0,PF::NoAlpha));
	add_pair(PIXMAN_a8b8g8r8, DynamicFormat(32,8,0,8,8,8,16,8,24,PF::Alpha));
	add_pair(PIXMAN_x8b8g8r8, DynamicFormat(32,8,0,8,8,8,16,0,0,PF::NoAlpha));
	add_pair(PIXMAN_b8g8r8a8, DynamicFormat(32,8,8,8,16,8,24,8,0,PF::Alpha));
	add_pair(PIXMAN_b8g8r8x8, DynamicFormat(32,8,8,8,16,8,24,0,0,PF::NoAlpha));

	add_pair(PIXMAN_x14r6g6b6, DynamicFormat(32,6,12,6,6,6,0,0,0,PF::NoAlpha));
	add_pair(PIXMAN_x2r10g10b10, DynamicFormat(32,10,20,10,10,10,0,0,0,PF::NoAlpha));
	add_pair(PIXMAN_a2r10g10b10, DynamicFormat(32,10,20,10,10,10,0,2,30,PF::Alpha));
	add_pair(PIXMAN_x2b10g10r10, DynamicFormat(32,10,0,10,10,10,20,0,0,PF::NoAlpha));
	add_pair(PIXMAN_a2b10g10r10, DynamicFormat(32,10,0,10,10,10,20,2,30,PF::Alpha));

	add_pair(PIXMAN_r8g8b8a8, DynamicFormat(32,8,24,8,16,8,8,8,0,PF::Alpha));
	add_pair(PIXMAN_r8g8b8x8, DynamicFormat(32,8,24,8,16,8,8,8,0,PF::NoAlpha));

	add_pair(PIXMAN_r8g8b8, DynamicFormat(24,8,16,8,8,8,0,0,0,PF::NoAlpha));
	add_pair(PIXMAN_b8g8r8, DynamicFormat(24,8,0,8,8,8,16,0,0,PF::NoAlpha));

	add_pair(PIXMAN_r5g6b5, DynamicFormat(16,5,11,6,5,5,0,0,0,PF::NoAlpha));
	add_pair(PIXMAN_b5g6r5, DynamicFormat(16,5,0,6,5,5,11,0,0,PF::NoAlpha));
	add_pair(PIXMAN_a1r5g5b5, DynamicFormat(16,5,10,5,5,5,0,1,15,PF::Alpha));
	add_pair(PIXMAN_x1r5g5b5, DynamicFormat(16,5,10,5,5,5,0,0,0,PF::NoAlpha));
	add_pair(PIXMAN_a1b5g5r5, DynamicFormat(16,5,0,5,5,5,10,1,15,PF::Alpha));
	add_pair(PIXMAN_x1b5g5r5, DynamicFormat(16,5,0,5,5,5,10,0,0,PF::NoAlpha));
	add_pair(PIXMAN_a4r4g4b4, DynamicFormat(16,4,8,4,4,4,0,4,12,PF::Alpha));
	add_pair(PIXMAN_x4r4g4b4, DynamicFormat(16,4,8,4,4,4,0,0,0,PF::NoAlpha));
	add_pair(PIXMAN_a4b4g4r4, DynamicFormat(16,4,0,4,4,4,8,4,12,PF::Alpha));
	add_pair(PIXMAN_x4b4g4r4, DynamicFormat(16,4,0,4,4,4,8,0,0,PF::NoAlpha));
	add_pair(PIXMAN_g8, DynamicFormat(8,8,0,8,0,8,0,8,0,PF::Alpha));
	add_pair(PIXMAN_g8, DynamicFormat(8,8,0,8,0,8,0,0,0,PF::NoAlpha));

	formats_initialized = true;
}

pixman_format_code_t Bitmap::find_format(const DynamicFormat& format) {
	initialize_formats();
	int dcode = format.code_alpha();
	int pcode = formats_map[dcode];
	if (pcode == 0) {
		// To fix add a pair to initialize_formats that maps the outputted
		// DynamicFormat to a pixman format
		Output::Error("%s\nDynamicFormat(%d, %d, %d, %d, %d, %d, %d, %d, %d, %s)",
		"Couldn't find Pixman format for",
		format.bits,
		format.r.bits, format.r.shift,
		format.g.bits, format.g.shift,
		format.b.bits, format.b.shift,
		format.a.bits, format.a.shift,
		format.alpha_type == PF::Alpha ? "PF::Alpha" : "PF::NoAlpha");
	}
	return (pixman_format_code_t) pcode;
}

DynamicFormat Bitmap::pixel_format;
DynamicFormat Bitmap::opaque_pixel_format;
DynamicFormat Bitmap::image_format;
DynamicFormat Bitmap::opaque_image_format;

void Bitmap::SetFormat(const DynamicFormat& format) {
	pixel_format = format;
	opaque_pixel_format = format;
	opaque_pixel_format.alpha_type = PF::NoAlpha;
	image_format = format_R8G8B8A8_a().format();
	opaque_image_format = format_R8G8B8A8_n().format();
}

DynamicFormat Bitmap::ChooseFormat(const DynamicFormat& format) {
	uint32_t amask;
	amask = (format.a.mask == 0)
		? ((~0U >> (32 - format.bits)) ^ (format.r.mask | format.g.mask | format.b.mask))
		: format.a.mask;
	if (amask != 0)
		return DynamicFormat(format.bits,
							 format.r.mask, format.g.mask, format.b.mask,
							 amask, PF::Alpha);
	switch (format.bits) {
		case 16:
			return (format.r.shift > format.b.shift)
				? DynamicFormat(16,5,10,5,5,5,0,1,15,PF::Alpha)
				: DynamicFormat(16,5,0,5,5,5,10,1,15,PF::Alpha);
		case 24:
			return (format.r.shift > format.b.shift)
				? DynamicFormat(32,8,16,8,8,8,0,8,24,PF::Alpha)
				: DynamicFormat(32,8,0,8,8,8,16,8,24,PF::Alpha);
		default:
			return format_B8G8R8A8_a().format();
	}
}

static void destroy_func(pixman_image_t * /* image */, void *data) {
	free(data);
}

static pixman_indexed_t palette;
static bool palette_initialized = false;

static void initialize_palette() {
	if (palette_initialized)
		return;
	palette.color = false;
	palette.rgba[0] = 0U;
	for (int i = 1; i < PIXMAN_MAX_INDEXED; i++)
		palette.rgba[i] = ~0U;
	palette_initialized = true;
}

void Bitmap::Init(int width, int height, void* data, int pitch, bool destroy) {
	if (!pitch)
		pitch = width * format.bytes;

	bitmap = pixman_image_create_bits(pixman_format, width, height, (uint32_t*) data, pitch);

	if (bitmap == NULL) {
		Output::Error("Couldn't create %dx%d image.", width, height);
	}

	if (format.bits == 8) {
		initialize_palette();
		pixman_image_set_indexed(bitmap, &palette);
	}

	if (data != NULL && destroy)
		pixman_image_set_destroy_function(bitmap, destroy_func, data);
}

void Bitmap::ConvertImage(int& width, int& height, void*& pixels, bool transparent) {
	const DynamicFormat& img_format = transparent ? image_format : opaque_image_format;

	// premultiply alpha
	for (int y = 0; y < height; y++) {
		uint8_t* dst = (uint8_t*) pixels + y * width * 4;
		for (int x = 0; x < width; x++) {
			uint8_t &r = *dst++;
			uint8_t &g = *dst++;
			uint8_t &b = *dst++;
			uint8_t &a = *dst++;
			MultiplyAlpha(r, g, b, a);
		}
	}

	Bitmap src(pixels, width, height, 0, img_format);
	Clear();
	Blit(0, 0, src, src.GetRect(), Opacity::opaque);
	free(pixels);
}

void* Bitmap::pixels() {
	if (!bitmap) {
		return nullptr;
	}

	return (void*) pixman_image_get_data(bitmap);
}
void const* Bitmap::pixels() const {
	return (void const*) pixman_image_get_data(bitmap);
}

int Bitmap::bpp() const {
	return (pixman_image_get_depth(bitmap) + 7) / 8;
}

int Bitmap::width() const {
	return pixman_image_get_width(bitmap);
}

int Bitmap::height() const {
	return pixman_image_get_height(bitmap);
}

int Bitmap::pitch() const {
	return pixman_image_get_stride(bitmap);
}

namespace {
	pixman_image_t *CreateMask(Opacity const& opacity, Rect const& src_rect, Transform const* pxform = nullptr) {
		if (opacity.IsOpaque())
			return (pixman_image_t*) NULL;

		if (!opacity.IsSplit()) {
			pixman_color_t tcolor = {0, 0, 0, static_cast<uint16_t>(opacity.Value() << 8)};
			return pixman_image_create_solid_fill(&tcolor);
		}

		pixman_image_t *mask = pixman_image_create_bits(PIXMAN_a8, 1, 2, (uint32_t*) NULL, 4);
		uint32_t* pixels = pixman_image_get_data(mask);
		*reinterpret_cast<uint8_t*>(&pixels[0]) = (opacity.top & 0xFF);
		*reinterpret_cast<uint8_t*>(&pixels[1]) = (opacity.bottom & 0xFF);

		Transform xform = Transform::Scale(1.0 / src_rect.width, 1.0 / src_rect.height);
		xform *= Transform::Translation(0, opacity.split);

		if (pxform)
			xform *= *pxform;

		pixman_image_set_transform(mask, &xform.matrix);

		return mask;
	}
} // anonymous namespace

void Bitmap::Blit(int x, int y, Bitmap const& src, Rect const& src_rect, Opacity const& opacity) {
	if (opacity.IsTransparent())
		return;

	pixman_image_t* mask = CreateMask(opacity, src_rect);

	pixman_image_composite32(src.GetOperator(mask),
							 src.bitmap,
							 mask, bitmap,
							 src_rect.x, src_rect.y,
							 0, 0,
							 x, y,
							 src_rect.width, src_rect.height);

	if (mask != NULL)
		pixman_image_unref(mask);
}

void Bitmap::BlitFast(int x, int y, Bitmap const & src, Rect const & src_rect, Opacity const & opacity) {
	if (opacity.IsTransparent())
		return;

	pixman_image_composite32(PIXMAN_OP_SRC,
		src.bitmap,
		nullptr, bitmap,
		src_rect.x, src_rect.y,
		0, 0,
		x, y,
		src_rect.width, src_rect.height);
}

pixman_image_t* Bitmap::GetSubimage(Bitmap const& src, const Rect& src_rect) {
	uint8_t* pixels = (uint8_t*) src.pixels() + src_rect.x * src.bpp() + src_rect.y * src.pitch();
	return pixman_image_create_bits(src.pixman_format, src_rect.width, src_rect.height,
									(uint32_t*) pixels, src.pitch());
}

void Bitmap::TiledBlit(Rect const& src_rect, Bitmap const& src, Rect const& dst_rect, Opacity const& opacity) {
	TiledBlit(0, 0, src_rect, src, dst_rect, opacity);
}

void Bitmap::TiledBlit(int ox, int oy, Rect const& src_rect, Bitmap const& src, Rect const& dst_rect, Opacity const& opacity) {
	if (opacity.IsTransparent())
		return;

	if (ox >= src_rect.width)	ox %= src_rect.width;
	if (oy >= src_rect.height)	oy %= src_rect.height;
	if (ox < 0) ox += src_rect.width  * ((-ox + src_rect.width  - 1) / src_rect.width);
	if (oy < 0) oy += src_rect.height * ((-oy + src_rect.height - 1) / src_rect.height);

	pixman_image_t* src_bm = GetSubimage(src, src_rect);

	pixman_image_set_repeat(src_bm, PIXMAN_REPEAT_NORMAL);

	Transform xform = Transform::Translation(ox, oy);

	pixman_image_set_transform(src_bm, &xform.matrix);

	pixman_image_t* mask = CreateMask(opacity, src_rect, &xform);

	pixman_image_composite32(src.GetOperator(mask),
							 src_bm, mask, bitmap,
							 0, 0,
							 0, 0,
							 dst_rect.x, dst_rect.y,
							 dst_rect.width, dst_rect.height);

	pixman_image_unref(src_bm);

	if (mask != NULL)
		pixman_image_unref(mask);
}

void Bitmap::StretchBlit(Bitmap const&  src, Rect const& src_rect, Opacity const& opacity) {
	StretchBlit(GetRect(), src, src_rect, opacity);
}

void Bitmap::StretchBlit(Rect const& dst_rect, Bitmap const& src, Rect const& src_rect, Opacity const& opacity) {
	if (opacity.IsTransparent())
		return;

	double zoom_x = (double)src_rect.width  / dst_rect.width;
	double zoom_y = (double)src_rect.height / dst_rect.height;

	Transform xform = Transform::Scale(zoom_x, zoom_y);

	pixman_image_set_transform(src.bitmap, &xform.matrix);

	pixman_image_t* mask = CreateMask(opacity, src_rect, &xform);

	pixman_image_composite32(src.GetOperator(mask),
							 src.bitmap, mask, bitmap,
							 src_rect.x / zoom_x, src_rect.y / zoom_y,
							 0, 0,
							 dst_rect.x, dst_rect.y,
							 dst_rect.width, dst_rect.height);

	pixman_image_set_transform(src.bitmap, nullptr);

	if (mask != NULL)
		pixman_image_unref(mask);
}

void Bitmap::TransformBlit(Rect const& dst_rect, Bitmap const& src, Rect const& /* src_rect */, const Transform& xform, Opacity const& opacity) {
	if (opacity.IsTransparent())
		return;

	pixman_image_set_transform(src.bitmap, &xform.matrix);

	pixman_image_t* mask = CreateMask(opacity, src.GetRect(), &xform);

	pixman_image_composite32(PIXMAN_OP_OVER,
							 src.bitmap, mask, bitmap,
							 dst_rect.x, dst_rect.y,
							 dst_rect.x, dst_rect.y,
							 dst_rect.x, dst_rect.y,
							 dst_rect.width, dst_rect.height);

	pixman_image_set_transform(src.bitmap, nullptr);

	if (mask != NULL)
		pixman_image_unref(mask);
}

void Bitmap::WaverBlit(int x, int y, double zoom_x, double zoom_y, Bitmap const& src, Rect const& src_rect, int depth, double phase, Opacity const& opacity) {
	if (opacity.IsTransparent())
		return;

	Transform xform = Transform::Scale(1.0 / zoom_x, 1.0 / zoom_y);

	pixman_image_set_transform(src.bitmap, &xform.matrix);

	pixman_image_t* mask = CreateMask(opacity, src_rect, &xform);

	int height = static_cast<int>(std::floor(src_rect.height * zoom_y));
	int width  = static_cast<int>(std::floor(src_rect.width * zoom_x));
	for (int i = 0; i < height; i++) {
		int dy = y + i;
		if (dy < 0)
			continue;
		if (dy >= this->height())
			break;
		int sy = static_cast<int>(std::floor((i+0.5) / zoom_y));
		int offset = (int) (2 * zoom_x * depth * sin((phase + (src_rect.y + sy) * 11.2) * 3.14159 / 180));

		pixman_image_composite32(src.GetOperator(mask),
								 src.bitmap, mask, bitmap,
								 src_rect.x, i,
								 src_rect.x, i,
								 x + offset, dy,
								 width, 1);
	}

	pixman_image_set_transform(src.bitmap, nullptr);

	if (mask != NULL)
		pixman_image_unref(mask);
}

static pixman_color_t PixmanColor(const Color &color) {
	pixman_color_t pcolor;
	pcolor.red = color.red * color.alpha;
	pcolor.green = color.green * color.alpha;
	pcolor.blue = color.blue * color.alpha;
	pcolor.alpha = color.alpha << 8;
	return pcolor;
}

void Bitmap::Fill(const Color &color) {
	pixman_color_t pcolor = PixmanColor(color);
	Rect src_rect(0, 0, static_cast<uint16_t>(width()), static_cast<uint16_t>(height()));

	pixman_image_t* timage = pixman_image_create_solid_fill(&pcolor);

	pixman_image_composite32(PIXMAN_OP_SRC,
		timage, (pixman_image_t*)NULL, bitmap,
		src_rect.x, src_rect.y,
		0, 0,
		0, 0,
		src_rect.width, src_rect.height);

	pixman_image_unref(timage);
}

void Bitmap::FillRect(Rect const& dst_rect, const Color &color) {
	pixman_color_t pcolor = PixmanColor(color);
	pixman_rectangle16_t rect = {
	static_cast<int16_t>(dst_rect.x),
	static_cast<int16_t>(dst_rect.y),
	static_cast<uint16_t>(dst_rect.width),
	static_cast<uint16_t>(dst_rect.height)};

	pixman_image_fill_rectangles(PIXMAN_OP_OVER, bitmap, &pcolor, 1, &rect);
}

void Bitmap::Clear() {
	memset(pixels(), '\0', height() * pitch());
}

void Bitmap::ClearRect(Rect const& dst_rect) {
	pixman_color_t pcolor = {0, 0, 0, 0};
	pixman_rectangle16_t rect = {
		static_cast<int16_t>(dst_rect.x),
		static_cast<int16_t>(dst_rect.y),
		static_cast<uint16_t>(dst_rect.width),
		static_cast<uint16_t>(dst_rect.height)
	};

	pixman_image_fill_rectangles(PIXMAN_OP_CLEAR, bitmap, &pcolor, 1, &rect);
}

// Hard light lookup table mapping source color to destination color
static uint8_t hard_light_lookup[256][256];

static void make_hard_light_lookup() {
	for (int i = 0; i < 256; ++i) {
		for (int j = 0; j < 256; ++j) {
			int res = 0;
			if (i <= 128)
				res = (2 * i * j) / 255;
			else
				res = 255 - 2 * (255 - i) * (255 - j) / 255;
			hard_light_lookup[i][j] = res > 255 ? 255 : res < 0 ? 0 : res;
		}
	}
}

void Bitmap::ToneBlit(int x, int y, Bitmap const& src, Rect const& src_rect, const Tone &tone, Opacity const& opacity) {
	if (tone == Tone(128,128,128,128)) {
		if (&src != this) {
			Blit(x, y, src, src_rect, opacity);
		}
		return;
	}

	// Only needed here, other codepaths are sanity checked by pixman
	if (x < 0 || y < 0 || x >= width() || y >= height()) {
		return;
	}

	if (&src != this)
		pixman_image_composite32(src.GetOperator(),
		src.bitmap, (pixman_image_t*)NULL, bitmap,
		src_rect.x, src_rect.y,
		0, 0,
		x, y,
		src_rect.width, src_rect.height);

	if (tone.gray != 128) {
		uint32_t* pixels = (uint32_t*)this->pixels();

		int sat;
		if (tone.gray > 128) {
			sat = 1024 + (tone.gray - 128) * 16;
		}
		else {
			sat = tone.gray * 8;
		}

		int as = pixel_format.a.shift;
		int rs = pixel_format.r.shift;
		int gs = pixel_format.g.shift;
		int bs = pixel_format.b.shift;

		// Move according to x/y value
		pixels = pixels + (y * pitch() / sizeof(uint32_t) + x) - (pitch() / sizeof(uint32_t));

		// Algorithm from OpenPDN (MIT license)
		// Transformation in Y'CbCr color space
		for (int i = 0; i < src_rect.height && i < height(); ++i) {
			// Advance one pixel row
			pixels += pitch() / sizeof(uint32_t);

			for (int j = 0; j < src_rect.width && j < width(); ++j) {
				uint32_t pixel = pixels[j];
				uint8_t a = (pixel >> as) & 0xFF;
				// &src != this works around a corner case with opacity split (character in a bush)
				// in that case a == 0 and the effect is not applied
				if (a == 0 && &src != this) {
					continue;
				}
				uint8_t r = (pixel >> rs) & 0xFF;
				uint8_t g = (pixel >> gs) & 0xFF;
				uint8_t b = (pixel >> bs) & 0xFF;
				// Y' = 0.299 R' + 0.587 G' + 0.114 B'
				uint8_t lum = (7471 * b + 38470 * g + 19595 * r) >> 16;
				// Scale Cb/Cr by scale factor "sat"
				int red = ((lum * 1024 + (r - lum) * sat) >> 10);
				red = red > 255 ? 255 : red < 0 ? 0 : red;
				int green = ((lum * 1024 + (g - lum) * sat) >> 10);
				green = green > 255 ? 255 : green < 0 ? 0 : green;
				int blue = ((lum * 1024 + (b - lum) * sat) >> 10);
				blue = blue > 255 ? 255 : blue < 0 ? 0 : blue;
				pixels[j] = ((uint32_t) red << rs) | ((uint32_t) green << gs) | ((uint32_t) blue << bs) |
							((uint32_t) a << as);
			}
		}
	}

	if (tone.red != 128 || tone.green != 128 || tone.blue != 128) {
		static bool index_made = false;
		if (!index_made) {
			make_hard_light_lookup();
			index_made = true;
		}

		int as = pixel_format.a.shift;
		int rs = pixel_format.r.shift;
		int gs = pixel_format.g.shift;
		int bs = pixel_format.b.shift;

		uint32_t* pixels = (uint32_t*)this->pixels();
		// Move according to x/y value
		pixels = pixels + (y * pitch() / sizeof(uint32_t) + x) - (pitch() / sizeof(uint32_t));

		for (int i = 0; i < src_rect.height && i < height(); ++i) {
			// Advance one pixel row
			pixels += pitch() / sizeof(uint32_t);

			for (int j = 0; j < src_rect.width && j < width(); ++j) {
				uint32_t pixel = pixels[j];
				uint8_t a = (pixel >> as) & 0xFF;
				if (a == 0 && &src != this) {
					continue;
				}
				uint8_t r = (pixel >> rs) & 0xFF;
				uint8_t g = (pixel >> gs) & 0xFF;
				uint8_t b = (pixel >> bs) & 0xFF;

				int red = hard_light_lookup[tone.red][r];
				int green = hard_light_lookup[tone.green][g];
				int blue = hard_light_lookup[tone.blue][b];
				pixels[j] = ((uint32_t) red << rs) | ((uint32_t) green << gs) | ((uint32_t) blue << bs) |
							((uint32_t) a << as);
			}
		}
	}
}

void Bitmap::BlendBlit(int x, int y, Bitmap const& src, Rect const& src_rect, const Color& color, Opacity const& opacity) {
	if (color.alpha == 0) {
		if (&src != this)
			Blit(x, y, src, src_rect, opacity);
		return;
	}

	if (&src != this)
		pixman_image_composite32(src.GetOperator(),
								 src.bitmap, (pixman_image_t*) NULL, bitmap,
								 src_rect.x, src_rect.y,
								 0, 0,
								 x, y,
								 src_rect.width, src_rect.height);

	pixman_color_t tcolor = PixmanColor(color);
	pixman_image_t* timage = pixman_image_create_solid_fill(&tcolor);

	pixman_image_composite32(src.GetOperator(),
							 timage, src.bitmap, bitmap,
							 0, 0,
							 src_rect.x, src_rect.y,
							 x, y,
							 src_rect.width, src_rect.height);

	pixman_image_unref(timage);
}

void Bitmap::FlipBlit(int x, int y, Bitmap const& src, Rect const& src_rect, bool horizontal, bool vertical, Opacity const& opacity) {
	if (!horizontal && !vertical) {
		Blit(x, y, src, src_rect, opacity);
		return;
	}

	Transform xform = Transform::Scale(horizontal ? -1 : 1, vertical ? -1 : 1);
	xform *= Transform::Translation(horizontal ? -src.GetWidth() : 0, vertical ? -src.GetHeight() : 0);

	pixman_image_set_transform(src.bitmap, &xform.matrix);

	pixman_image_composite32(src.GetOperator(),
							 src.bitmap, (pixman_image_t*) NULL, bitmap,
							 horizontal ? src.GetWidth() - src_rect.x - src_rect.width : src_rect.x,
							 vertical ? src.GetHeight() - src_rect.y - src_rect.height : src_rect.y,
							 0, 0,
							 x, y,
							 src_rect.width, src_rect.height);

	pixman_image_set_transform(src.bitmap, nullptr);
}

void Bitmap::Flip(const Rect& dst_rect, bool horizontal, bool vertical) {
	if (!horizontal && !vertical)
		return;

	BitmapRef resampled(new Bitmap(dst_rect.width, dst_rect.height, GetTransparent()));

	resampled->FlipBlit(0, 0, *this, dst_rect, horizontal, vertical, Opacity::opaque);

	pixman_image_composite32(GetOperator(),
							 resampled->bitmap, (pixman_image_t*) NULL, bitmap,
							 0, 0,
							 0, 0,
							 dst_rect.x, dst_rect.y,
							 dst_rect.width, dst_rect.height);
}

void Bitmap::MaskedBlit(Rect const& dst_rect, Bitmap const& mask, int mx, int my, Color const& color) {
	pixman_color_t tcolor = {
		static_cast<uint16_t>(color.red << 8),
		static_cast<uint16_t>(color.green << 8),
		static_cast<uint16_t>(color.blue << 8),
		static_cast<uint16_t>(color.alpha << 8)};

	pixman_image_t* source = pixman_image_create_solid_fill(&tcolor);

	pixman_image_composite32(PIXMAN_OP_OVER,
							 source, mask.bitmap, bitmap,
							 0, 0,
							 mx, my,
							 dst_rect.x, dst_rect.y,
							 dst_rect.width, dst_rect.height);

	pixman_image_unref(source);
}

void Bitmap::MaskedBlit(Rect const& dst_rect, Bitmap const& mask, int mx, int my, Bitmap const& src, int sx, int sy) {
	pixman_image_composite32(PIXMAN_OP_OVER,
							 src.bitmap, mask.bitmap, bitmap,
							 sx, sy,
							 mx, my,
							 dst_rect.x, dst_rect.y,
							 dst_rect.width, dst_rect.height);
}

void Bitmap::Blit2x(Rect const& dst_rect, Bitmap const& src, Rect const& src_rect) {
	Transform xform = Transform::Scale(0.5, 0.5);

	pixman_image_set_transform(src.bitmap, &xform.matrix);

	pixman_image_composite32(PIXMAN_OP_SRC,
							 src.bitmap, (pixman_image_t*) NULL, bitmap,
							 src_rect.x, src_rect.y,
							 0, 0,
							 dst_rect.x, dst_rect.y,
							 dst_rect.width, dst_rect.height);

	pixman_image_set_transform(src.bitmap, nullptr);
}

void Bitmap::EffectsBlit(int x, int y, int ox, int oy,
						 Bitmap const& src, Rect const& src_rect,
						 Opacity const& opacity,
						 double zoom_x, double zoom_y, double angle,
						 int waver_depth, double waver_phase) {
	if (opacity.IsTransparent())
		return;

	bool rotate = angle != 0.0;
	bool scale = zoom_x != 1.0 || zoom_y != 1.0;
	bool waver = waver_depth != 0;

	if (waver) {
		WaverBlit(x - ox * zoom_x, y - oy * zoom_y, zoom_x, zoom_y, src, src_rect,
				  waver_depth, waver_phase, opacity);
	}
	else if (rotate) {
		Transform fwd = Transform::Translation(x, y);
		fwd *= Transform::Rotation(angle);
		if (scale)
			fwd *= Transform::Scale(zoom_x, zoom_y);
		fwd *= Transform::Translation(-ox, -oy);

		RotateZoomOpacityBlit(fwd, src, src_rect, opacity);
	}
	else if (scale) {
		ZoomOpacityBlit(x, y, ox, oy, src, src_rect, zoom_x, zoom_y, opacity);
	}
	else {
		Blit(x - ox, y - oy, src, src_rect, opacity);
	}
}

void Bitmap::RotateZoomOpacityBlit(const Transform &fwd, Bitmap const& src, Rect const& src_rect, Opacity const& opacity) {
	Rect dst_rect = TransformRectangle(fwd, src_rect);
	dst_rect.Adjust(GetRect());
	if (dst_rect.IsEmpty())
		return;

	Transform inv = fwd.Inverse();

	TransformBlit(dst_rect, src, src_rect, inv, opacity);
}

void Bitmap::ZoomOpacityBlit(int x, int y, int ox, int oy,
							 Bitmap const& src, Rect const& src_rect,
							 double zoom_x, double zoom_y,
							 Opacity const& opacity) {
	Rect dst_rect(
		x - static_cast<int>(std::floor(ox * zoom_x)),
		y - static_cast<int>(std::floor(oy * zoom_y)),
		static_cast<int>(std::floor(src_rect.width * zoom_x)),
		static_cast<int>(std::floor(src_rect.height * zoom_y)));
	StretchBlit(dst_rect, src, src_rect, opacity);
}

pixman_op_t Bitmap::GetOperator(pixman_image_t* mask) const {
	if (!mask && (!GetTransparent() || GetOpacity() == Opaque)) {
		return PIXMAN_OP_SRC;
	}

	return PIXMAN_OP_OVER;
}
