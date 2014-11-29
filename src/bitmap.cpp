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

#include "system.h"
#include "utils.h"
#include "cache.h"
#include "bitmap.h"
#include "text.h"
#include "filefinder.h"
#include "options.h"
#include "data.h"
#include "output.h"
#include "image_xyz.h"
#include "image_bmp.h"
#include "image_png.h"
#include "pixel_format.h"
#include "font.h"
#include "output.h"
#include "util_macro.h"
#include "bitmap_hslrgb.h"

BitmapRef Bitmap::Create(int width, int height, const Color& color) {
    BitmapRef surface = Bitmap::Create(width, height, false);
	surface->Fill(color);
	return surface;
}

BitmapRef Bitmap::Create(const std::string& filename, bool transparent, uint32_t flags) {
	return EASYRPG_MAKE_SHARED<Bitmap>(filename, transparent, flags);
}

BitmapRef Bitmap::Create(const uint8_t* data, unsigned bytes, bool transparent, uint32_t flags) {
	return EASYRPG_MAKE_SHARED<Bitmap>(data, bytes, transparent, flags);
}

BitmapRef Bitmap::Create(Bitmap const& source, Rect const& src_rect, bool transparent) {
	return EASYRPG_MAKE_SHARED<Bitmap>(source, src_rect, transparent);
}

void Bitmap::InitBitmap() {
	editing = false;
	font = Font::Default();
}

Bitmap::Bitmap() {
	InitBitmap();
}

Bitmap::~Bitmap() {
	pixman_image_unref(bitmap);
}

bool Bitmap::WritePNG(std::ostream& os) const {
	size_t const width = GetWidth(), height = GetHeight();
	size_t const stride = width * 4;

	std::vector<uint32_t> data(width * height);

	EASYRPG_SHARED_PTR<pixman_image_t> dst
		(pixman_image_create_bits(PIXMAN_a8r8g8b8, width, height, &data.front(), stride),
		 pixman_image_unref);
	pixman_image_composite32(PIXMAN_OP_SRC, bitmap, NULL, dst.get(),
							 0, 0, 0, 0, 0, 0, width, height);

	return ImagePNG::WritePNG(os, width, height, &data.front());
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

Color Bitmap::GetTransparentColor() const {
	return Color();
}

void Bitmap::SetTransparentColor(Color /* color */) {
}

Bitmap::TileOpacity Bitmap::CheckOpacity(const Rect& rect) {
	bool all = true;
	bool any = false;

	DynamicFormat format(32,8,24,8,16,8,8,8,0,PF::Alpha);
	std::vector<uint32_t> pixels;
	pixels.resize(rect.width * rect.height);
	Bitmap bmp(reinterpret_cast<void*>(&pixels.front()), rect.width, rect.height, rect.width*4, format);
	bmp.Blit(0, 0, *this, rect, 255);

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
	if (flags & System) {
		DynamicFormat format(32,8,24,8,16,8,8,8,0,PF::Alpha);
		uint32_t pixel;
		Bitmap bmp(reinterpret_cast<void*>(&pixel), 1, 1, 4, format);
		bmp.Blit(0, 0, *this, Rect(0, 32, 1, 1), 255);
		bg_color = Color((pixel>>24)&0xFF, (pixel>>16)&0xFF, (pixel>>8)&0xFF, pixel&0xFF);
		bmp.Blit(0, 0, *this, Rect(16, 32, 1, 1), 255);
		sh_color = Color((pixel>>24)&0xFF, (pixel>>16)&0xFF, (pixel>>8)&0xFF, pixel&0xFF);
	}

	if (flags & Chipset) {
		opacity.reset(new opacity_type());
		for (int row = 0; row < 16; row++) {
			for (int col = 0; col < 30; col++) {
				Rect rect(col * 16, row * 16, 16, 16);
				(*opacity)[row][col] = CheckOpacity(rect);
			}
		}
	}
}

Bitmap::TileOpacity Bitmap::GetTileOpacity(int row, int col) {
	return opacity? (*opacity)[row][col] : Partial;
}

Color Bitmap::GetBackgroundColor() {
	return bg_color;
}

Color Bitmap::GetShadowColor() {
	return sh_color;
}

int Bitmap::bytes() const {
	return format.bytes;
}

uint8_t* Bitmap::pointer(int x, int y) {
	return (uint8_t*) pixels() + y * pitch() + x * bytes();
}

uint8_t const* Bitmap::pointer(int x, int y) const {
	return (uint8_t const*) pixels() + y * pitch() + x * bytes();
}

BitmapRef Bitmap::Create(int width, int height, bool transparent, int /* bpp */) {
	return EASYRPG_MAKE_SHARED<Bitmap>(width, height, transparent);
}

BitmapRef Bitmap::Create(void *pixels, int width, int height, int pitch, const DynamicFormat& format) {
	return EASYRPG_MAKE_SHARED<Bitmap>(pixels, width, height, pitch, format);
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
	bmp.Blit(0, 0, src, src_rect, 255);

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

	Blit(dst_rect.x, dst_rect.y, bmp, bmp.GetRect(), 255);

	RefreshCallback();
}

void Bitmap::RefreshCallback() {
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
	RefreshCallback();
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
	RefreshCallback();
}

Rect Bitmap::TransformRectangle(const Matrix& m, const Rect& rect) {
	int sx0 = rect.x;
	int sy0 = rect.y;
	int sx1 = rect.x + rect.width;
	int sy1 = rect.y + rect.height;

	double x0, y0, x1, y1, x2, y2, x3, y3;
	m.Transform(sx0, sy0, x0, y0);
	m.Transform(sx1, sy0, x1, y1);
	m.Transform(sx1, sy1, x2, y2);
	m.Transform(sx0, sy1, x3, y3);

	double xmin = std::min(std::min(x0, x1), std::min(x2, x3));
	double ymin = std::min(std::min(y0, y1), std::min(y2, y3));
	double xmax = std::max(std::max(x0, x1), std::max(x2, x3));
	double ymax = std::max(std::max(y0, y1), std::max(y2, y3));

	int dx0 = (int) floor(xmin);
	int dy0 = (int) floor(ymin);
	int dx1 = (int) ceil(xmax);
	int dy1 = (int) ceil(ymax);

	return Rect(dx0, dy0, dx1 - dx0, dy1 - dy0);
}

Matrix Matrix::Setup(double angle,
					 double scale_x, double scale_y,
					 int src_pos_x, int src_pos_y,
					 int dst_pos_x, int dst_pos_y) {
	Matrix m = Matrix::Translation(-src_pos_x, -src_pos_y);
	m = m.PreMultiply(Matrix::Scale(scale_x, scale_y));
	m = m.PreMultiply(Matrix::Rotation(angle));
	m = m.PreMultiply(Matrix::Translation(dst_pos_x, dst_pos_y));
	return m;
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
		pitch = width * bytes();

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
	Blit(0, 0, src, src.GetRect(), 255);
	free(pixels);
}

Bitmap::Bitmap(int width, int height, bool transparent) {
	InitBitmap();

	format = (transparent ? pixel_format : opaque_pixel_format);
	pixman_format = find_format(format);
	Init(width, height, (void *) NULL);
}

Bitmap::Bitmap(void *pixels, int width, int height, int pitch, const DynamicFormat& _format) {
	InitBitmap();

	format = _format;
	pixman_format = find_format(format);
	Init(width, height, pixels, pitch, false);
}

Bitmap::Bitmap(const std::string& filename, bool transparent, uint32_t flags) {
	InitBitmap();

	format = (transparent ? pixel_format : opaque_pixel_format);
	pixman_format = find_format(format);

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

	int w = 0;
	int h = 0;
	void* pixels;

	char data[4];
	size_t bytes = fread(&data, 1, 4, stream);
	fseek(stream, 0, SEEK_SET);

	if (bytes >= 4 && strncmp((char*)data, "XYZ1", 4) == 0)
		ImageXYZ::ReadXYZ(stream, transparent, w, h, pixels);
	else if (bytes > 2 && strncmp((char*)data, "BM", 2) == 0)
		ImageBMP::ReadBMP(stream, transparent, w, h, pixels);
	else if (bytes >= 4 && strncmp((char*)(data + 1), "PNG", 3) == 0)
		ImagePNG::ReadPNG(stream, (void*)NULL, transparent, w, h, pixels);
	else
		Output::Error("Unsupported image file %s", filename.c_str());

	fclose(stream);

	Init(w, h, (void *) NULL);
	ConvertImage(w, h, pixels, transparent);

	CheckPixels(flags);
}

Bitmap::Bitmap(const uint8_t* data, unsigned bytes, bool transparent, uint32_t flags) {
	InitBitmap();

	format = (transparent ? pixel_format : opaque_pixel_format);
	pixman_format = find_format(format);

	int w, h;
	void* pixels;

	if (bytes > 4 && strncmp((char*) data, "XYZ1", 4) == 0)
		ImageXYZ::ReadXYZ(data, bytes, transparent, w, h, pixels);
	else if (bytes > 2 && strncmp((char*) data, "BM", 2) == 0)
		ImageBMP::ReadBMP(data, bytes, transparent, w, h, pixels);
	else if (bytes > 4 && strncmp((char*)(data + 1), "PNG", 3) == 0)
		ImagePNG::ReadPNG((FILE*) NULL, (const void*) data, transparent, w, h, pixels);
	else
		Output::Error("Unsupported image");

	Init(w, h, (void *) NULL);
	ConvertImage(w, h, pixels, transparent);

	CheckPixels(flags);
}

Bitmap::Bitmap(Bitmap const& source, Rect const& src_rect, bool transparent) {
	InitBitmap();

	format = (transparent ? pixel_format : opaque_pixel_format);
	pixman_format = find_format(format);

	Init(src_rect.width, src_rect.height, (void *) NULL);

	Blit(0, 0, source, src_rect, 255);
}

void* Bitmap::pixels() {
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

uint32_t Bitmap::rmask() const {
	return pixel_format.r.mask;
}

uint32_t Bitmap::gmask() const {
	return pixel_format.g.mask;
}

uint32_t Bitmap::bmask() const {
	return pixel_format.b.mask;
}

uint32_t Bitmap::amask() const {
	return pixel_format.a.mask;
}

BitmapRef Bitmap::Resample(int scale_w, int scale_h, const Rect& src_rect) const {
	BitmapRef dst(new Bitmap(scale_w, scale_h, GetTransparent()));

	double zoom_x = (double)src_rect.width  / scale_w;
	double zoom_y = (double)src_rect.height / scale_h;

	pixman_transform_t xform;
	pixman_transform_init_scale(&xform,
								pixman_double_to_fixed(zoom_x),
								pixman_double_to_fixed(zoom_y));

	pixman_image_set_transform(bitmap, &xform);

	pixman_image_composite32(PIXMAN_OP_SRC,
							 bitmap, (pixman_image_t*) NULL, dst->bitmap,
							 src_rect.x * scale_w / src_rect.width, src_rect.y *scale_h / src_rect.height,
							 0, 0,
							 0, 0,
							 scale_w, scale_h);

	pixman_transform_init_identity(&xform);
	pixman_image_set_transform(bitmap, &xform);

	return dst;
}

void Bitmap::Blit(int x, int y, Bitmap const& src, Rect const& src_rect, int opacity) {
	if (opacity < 0)
		return;

	if (opacity > 255) opacity = 255;

	pixman_image_t* mask;
	if (opacity < 255) {
		pixman_color_t tcolor = {0, 0, 0, static_cast<uint16_t>(opacity << 8)};
		mask = pixman_image_create_solid_fill(&tcolor);
	}
	else
		mask = (pixman_image_t*) NULL;

	pixman_image_composite32(PIXMAN_OP_OVER,
							 src.bitmap,
							 mask, bitmap,
							 src_rect.x, src_rect.y,
							 0, 0,
							 x, y,
							 src_rect.width, src_rect.height);

	if (mask != NULL)
		pixman_image_unref(mask);

	RefreshCallback();
}

pixman_image_t* Bitmap::GetSubimage(Bitmap const& src, const Rect& src_rect) {
	uint8_t* pixels = (uint8_t*) src.pixels() + src_rect.x * src.bpp() + src_rect.y * src.pitch();
	return pixman_image_create_bits(src.pixman_format, src_rect.width, src_rect.height,
									(uint32_t*) pixels, src.pitch());
}

void Bitmap::TiledBlit(Rect const& src_rect, Bitmap const& src, Rect const& dst_rect, int opacity) {
	TiledBlit(0, 0, src_rect, src, dst_rect, opacity);
}

void Bitmap::TiledBlit(int ox, int oy, Rect const& src_rect, Bitmap const& src, Rect const& dst_rect, int opacity) {
	if (opacity < 0)
		return;

	if (opacity > 255) opacity = 255;

	if (ox >= src_rect.width)	ox %= src_rect.width;
	if (oy >= src_rect.height)	ox %= src_rect.height;
	if (ox < 0) ox += src_rect.width  * ((-ox + src_rect.width  - 1) / src_rect.width);
	if (oy < 0) oy += src_rect.height * ((-oy + src_rect.height - 1) / src_rect.height);

	pixman_image_t* src_bm = GetSubimage(src, src_rect);

	pixman_image_t* mask;
	if (opacity < 255) {
		pixman_color_t tcolor = {0, 0, 0, static_cast<uint16_t>(opacity << 8)};
		mask = pixman_image_create_solid_fill(&tcolor);
	}
	else
		mask = (pixman_image_t*) NULL;

	pixman_image_set_repeat(src_bm, PIXMAN_REPEAT_NORMAL);

	pixman_transform_t xform;
	pixman_transform_init_translate(&xform,
									pixman_int_to_fixed(ox),
									pixman_int_to_fixed(oy));

	pixman_image_set_transform(src_bm, &xform);

	pixman_image_composite32(PIXMAN_OP_OVER,
							 src_bm, mask, bitmap,
							 0, 0,
							 0, 0,
							 dst_rect.x, dst_rect.y,
							 dst_rect.width, dst_rect.height);

	pixman_image_unref(src_bm);

	if (mask != NULL)
		pixman_image_unref(mask);

	RefreshCallback();
}

void Bitmap::StretchBlit(Bitmap const&  src, Rect const& src_rect, int opacity) {
	StretchBlit(GetRect(), src, src_rect, opacity);
}

void Bitmap::StretchBlit(Rect const& dst_rect, Bitmap const& src, Rect const& src_rect, int opacity) {
	if (opacity < 0)
		return;

	pixman_image_t* mask = (pixman_image_t*) NULL;
	if (opacity < 255) {
		pixman_color_t tcolor = {0, 0, 0, static_cast<uint16_t>(opacity << 8)};
		mask = pixman_image_create_solid_fill(&tcolor);
	}

	double zoom_x = (double)src_rect.width  / dst_rect.width;
	double zoom_y = (double)src_rect.height / dst_rect.height;

	pixman_transform_t xform;
	pixman_transform_init_scale(&xform,
								pixman_double_to_fixed(zoom_x),
								pixman_double_to_fixed(zoom_y));

	pixman_image_set_transform(src.bitmap, &xform);

	pixman_image_composite32(PIXMAN_OP_OVER,
							 src.bitmap, mask, bitmap,
							 src_rect.x * dst_rect.width / src_rect.width, src_rect.y * dst_rect.height / src_rect.height,
							 0, 0,
							 dst_rect.x, dst_rect.y,
							 dst_rect.width, dst_rect.height);

	pixman_transform_init_identity(&xform);
	pixman_image_set_transform(src.bitmap, &xform);

	if (mask != NULL)
		pixman_image_unref(mask);

	RefreshCallback();
}

void Bitmap::TransformBlit(Rect const& dst_rect, Bitmap const& src, Rect const& /* src_rect */, const Matrix& inv, int /* opacity */) {
	pixman_transform_t xform = {{
		{ pixman_double_to_fixed(inv.xx), pixman_double_to_fixed(inv.xy), pixman_double_to_fixed(inv.x0) },
		{ pixman_double_to_fixed(inv.yx), pixman_double_to_fixed(inv.yy), pixman_double_to_fixed(inv.y0) },
		{ pixman_double_to_fixed(0.0),    pixman_double_to_fixed(0.0),    pixman_double_to_fixed(1.0) }
		}};

	pixman_image_set_transform(src.bitmap, &xform);

	pixman_image_composite32(PIXMAN_OP_OVER,
							 src.bitmap, (pixman_image_t*) NULL, bitmap,
							 dst_rect.x, dst_rect.y,
							 0, 0,
							 dst_rect.x, dst_rect.y,
							 dst_rect.width, dst_rect.height);

	pixman_transform_init_identity(&xform);
	pixman_image_set_transform(src.bitmap, &xform);
}

void Bitmap::WaverBlit(int x, int y, Bitmap const& src, Rect const& src_rect, int depth, double phase, int opacity) {
	if (opacity < 0)
		return;

	if (opacity > 255) opacity = 255;

	pixman_image_t* mask;
	if (opacity < 255) {
		pixman_color_t tcolor = {0, 0, 0, static_cast<uint16_t>(opacity << 8)};
		mask = pixman_image_create_solid_fill(&tcolor);
	}
	else
		mask = (pixman_image_t*) NULL;

	for (int i = 0; i < src_rect.height; i++) {
		int offset = (int) (depth * (1 + sin((phase + i * 20) * 3.14159 / 180)));

		pixman_image_composite32(PIXMAN_OP_OVER,
								 src.bitmap, mask, bitmap,
								 src_rect.x, src_rect.y + i,
								 0, 0,
								 x + offset, y + i,
								 src_rect.width, 1);
	}

	if (mask != NULL)
		pixman_image_unref(mask);

	RefreshCallback();
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
	pixman_rectangle16_t rect = {
	0, 0, static_cast<uint16_t>(width()), static_cast<uint16_t>(height())};

	pixman_image_fill_rectangles(PIXMAN_OP_OVER, bitmap, &pcolor, 1, &rect);

	RefreshCallback();
}

void Bitmap::FillRect(Rect const& dst_rect, const Color &color) {
	pixman_color_t pcolor = PixmanColor(color);
	pixman_rectangle16_t rect = {
	static_cast<int16_t>(dst_rect.x),
	static_cast<int16_t>(dst_rect.y),
	static_cast<uint16_t>(dst_rect.width),
	static_cast<uint16_t>(dst_rect.height), };

	pixman_image_fill_rectangles(PIXMAN_OP_OVER, bitmap, &pcolor, 1, &rect);

	RefreshCallback();
}

void Bitmap::Clear() {
	pixman_color_t pcolor = {0, 0, 0, 0};
	pixman_rectangle16_t rect = {
	0, 0, static_cast<uint16_t>(width()), static_cast<uint16_t>(height())};

	pixman_image_fill_rectangles(PIXMAN_OP_CLEAR, bitmap, &pcolor, 1, &rect);

	RefreshCallback();
}

void Bitmap::ClearRect(Rect const& dst_rect) {
	pixman_color_t pcolor = {0, 0, 0, 0};
	pixman_rectangle16_t rect = {
    static_cast<int16_t>(dst_rect.x),
    static_cast<int16_t>(dst_rect.y),
    static_cast<uint16_t>(dst_rect.width),
    static_cast<uint16_t>(dst_rect.height), };

	pixman_image_fill_rectangles(PIXMAN_OP_CLEAR, bitmap, &pcolor, 1, &rect);

	RefreshCallback();
}

void Bitmap::ToneBlit(int x, int y, Bitmap const& src, Rect const& src_rect, const Tone &tone) {
	if (tone == Tone(128,128,128,128)) {
		if (&src != this) {
			Blit(x, y, src, src_rect, 255);
		}
		return;
	}

	if (&src != this)
		pixman_image_composite32(PIXMAN_OP_SRC,
								 src.bitmap, (pixman_image_t*) NULL, bitmap,
								 src_rect.x, src_rect.y,
								 0, 0,
								 x, y,
								 src_rect.width, src_rect.height);

	// FIXME: Saturation looks incorrect (compared to RPG_RT) for values > 128
	if (tone.gray != 128) {
		pixman_color_t gcolor = {
			static_cast<uint16_t>(tone.gray << 8),
			0,
			0,
			0xFFFF};

		pixman_image_t *gimage = pixman_image_create_solid_fill(&gcolor);

		pixman_image_composite32(PIXMAN_OP_HSL_SATURATION,
			gimage, src.bitmap, bitmap,
			src_rect.x, src_rect.y,
			0, 0,
			x, y,
			src_rect.width, src_rect.height);

		pixman_image_unref(gimage);
	}

	if (tone.red != 128 || tone.green != 128 || tone.blue != 128) {
		pixman_color_t tcolor = {
			static_cast<uint16_t>(tone.red << 8),
			static_cast<uint16_t>(tone.green << 8),
			static_cast<uint16_t>(tone.blue << 8), 0xFFFF};

		pixman_image_t *timage = pixman_image_create_solid_fill(&tcolor);

		pixman_image_composite32(PIXMAN_OP_HARD_LIGHT,
			timage, src.bitmap, bitmap,
			src_rect.x, src_rect.y,
			0, 0,
			x, y,
			src_rect.width, src_rect.height);

		pixman_image_unref(timage);
	}

	RefreshCallback();
}

void Bitmap::BlendBlit(int x, int y, Bitmap const& src, Rect const& src_rect, const Color& color) {
	if (color.alpha == 0) {
		if (&src != this)
			Blit(x, y, src, src_rect, 255);
		return;
	}

	if (&src != this)
		pixman_image_composite32(PIXMAN_OP_SRC,
								 src.bitmap, (pixman_image_t*) NULL, bitmap,
								 src_rect.x, src_rect.y,
								 0, 0,
								 x, y,
								 src_rect.width, src_rect.height);

	pixman_color_t tcolor = PixmanColor(color);
	pixman_image_t* timage = pixman_image_create_solid_fill(&tcolor);

	pixman_image_composite32(PIXMAN_OP_OVER,
							 timage, src.bitmap, bitmap,
							 0, 0,
							 src_rect.x, src_rect.y,
							 x, y,
							 src_rect.width, src_rect.height);

	pixman_image_unref(timage);

	RefreshCallback();
}

void Bitmap::FlipBlit(int x, int y, Bitmap const& src, Rect const& src_rect, bool horizontal, bool vertical) {
	if (!horizontal && !vertical) {
		Blit(x, y, src, src_rect, 255);
		return;
	}

	pixman_transform_t xform;
	pixman_transform_init_scale(&xform,
								pixman_int_to_fixed(horizontal ? -1 : 1),
								pixman_int_to_fixed(vertical ? -1 : 1));

	pixman_transform_translate((pixman_transform_t*) NULL, &xform,
							   pixman_int_to_fixed(horizontal ? src.GetWidth() : 0),
							   pixman_int_to_fixed(vertical ? src.GetHeight() : 0));

	pixman_image_set_transform(src.bitmap, &xform);

	pixman_image_composite32(PIXMAN_OP_SRC,
							 src.bitmap, (pixman_image_t*) NULL, bitmap,
							 horizontal ? src.GetWidth() - src_rect.x - src_rect.width : src_rect.x,
							 vertical ? src.GetHeight() - src_rect.y - src_rect.height : src_rect.y,
							 0, 0,
							 x, y,
							 src_rect.width, src_rect.height);

	pixman_transform_init_identity(&xform);
	pixman_image_set_transform(src.bitmap, &xform);

	RefreshCallback();
}

void Bitmap::Flip(const Rect& dst_rect, bool horizontal, bool vertical) {
	if (!horizontal && !vertical)
		return;

	BitmapRef resampled(new Bitmap(dst_rect.width, dst_rect.height, GetTransparent()));

	resampled->FlipBlit(0, 0, *this, dst_rect, horizontal, vertical);

	pixman_image_composite32(PIXMAN_OP_SRC,
							 resampled->bitmap, (pixman_image_t*) NULL, bitmap,
							 0, 0,
							 0, 0,
							 dst_rect.x, dst_rect.y,
							 dst_rect.width, dst_rect.height);

	RefreshCallback();
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

	RefreshCallback();
}

void Bitmap::MaskedBlit(Rect const& dst_rect, Bitmap const& mask, int mx, int my, Bitmap const& src, int sx, int sy) {
	pixman_image_composite32(PIXMAN_OP_OVER,
							 src.bitmap, mask.bitmap, bitmap,
							 sx, sy,
							 mx, my,
							 dst_rect.x, dst_rect.y,
							 dst_rect.width, dst_rect.height);

	RefreshCallback();
}

void Bitmap::Blit2x(Rect const& dst_rect, Bitmap const& src, Rect const& src_rect) {
	pixman_transform_t xform;
	pixman_transform_init_scale(&xform,
								pixman_double_to_fixed(0.5),
								pixman_double_to_fixed(0.5));

	pixman_image_set_transform(src.bitmap, &xform);

	pixman_image_composite32(PIXMAN_OP_SRC,
							 src.bitmap, (pixman_image_t*) NULL, bitmap,
							 src_rect.x, src_rect.y,
							 0, 0,
							 dst_rect.x, dst_rect.y,
							 dst_rect.width, dst_rect.height);

	pixman_transform_init_identity(&xform);
	pixman_image_set_transform(src.bitmap, &xform);

	RefreshCallback();
}

Color Bitmap::GetColor(uint32_t uint32_color) const {
	uint8_t r, g, b, a;
	GetColorComponents(uint32_color, r, g, b, a);
	return Color(r, g, b, a);
}

uint32_t Bitmap::GetUint32Color(const Color &color) const {
	return GetUint32Color(color.red, color.green, color.blue, color.alpha);
}

uint32_t Bitmap::GetUint32Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) const {
	MultiplyAlpha(r, g, b, a);
	return pixel_format.rgba_to_uint32_t(r, g, b, a);
}

void Bitmap::GetColorComponents(uint32_t color, uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a) const {
	pixel_format.uint32_to_rgba(color, r, g, b, a);
	DivideAlpha(r, g, b, a);
}
