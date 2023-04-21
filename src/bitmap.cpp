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
#include <unordered_map>

#include "utils.h"
#include "cache.h"
#include "bitmap.h"
#include "filefinder.h"
#include "options.h"
#include <lcf/data.h>
#include "output.h"
#include "image_xyz.h"
#include "image_bmp.h"
#include "image_png.h"
#include "transform.h"
#include "font.h"
#include "output.h"
#include "util_macro.h"
#include "bitmap_hslrgb.h"
#include <iostream>

BitmapRef Bitmap::Create(int width, int height, const Color& color) {
	BitmapRef surface = Bitmap::Create(width, height, true);
	surface->Fill(color);
	return surface;
}

BitmapRef Bitmap::Create(Filesystem_Stream::InputStream stream, bool transparent, uint32_t flags) {
	BitmapRef bmp = std::make_shared<Bitmap>(std::move(stream), transparent, flags);

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
	format = (transparent ? pixel_format : opaque_pixel_format);
	pixman_format = find_format(format);
	Init(width, height, (void *) NULL);
}

Bitmap::Bitmap(void *pixels, int width, int height, int pitch, const DynamicFormat& _format) {
	format = _format;
	pixman_format = find_format(format);
	Init(width, height, pixels, pitch, false);
}

Bitmap::Bitmap(Filesystem_Stream::InputStream stream, bool transparent, uint32_t flags) {
	format = (transparent ? pixel_format : opaque_pixel_format);
	pixman_format = find_format(format);

	if (!stream) {
		Output::Error("Couldn't read image file {}", stream.GetName());
		return;
	}

	int w = 0;
	int h = 0;
	void* pixels = nullptr;

	uint8_t data[4] = {};
	size_t bytes = stream.read(reinterpret_cast<char*>(data),  4).gcount();
	stream.seekg(0, std::ios::ios_base::beg);

	bool img_okay = false;

	if (bytes >= 4 && strncmp((char*)data, "XYZ1", 4) == 0)
		img_okay = ImageXYZ::ReadXYZ(stream, transparent, w, h, pixels);
	else if (bytes > 2 && strncmp((char*)data, "BM", 2) == 0)
		img_okay = ImageBMP::ReadBMP(stream, transparent, w, h, pixels);
	else if (bytes >= 4 && strncmp((char*)(data + 1), "PNG", 3) == 0)
		img_okay = ImagePNG::ReadPNG(stream, transparent, w, h, pixels);
	else
		Output::Warning("Unsupported image file {} (Magic: {:02X})", stream.GetName(), *reinterpret_cast<uint32_t*>(data));

	if (!img_okay) {
		free(pixels);
		pixels = nullptr;
		return;
	}

	Init(w, h, nullptr);

	ConvertImage(w, h, pixels, transparent);

	CheckPixels(flags);

	filename = ToString(stream.GetName());
}

Bitmap::Bitmap(const uint8_t* data, unsigned bytes, bool transparent, uint32_t flags) {
	format = (transparent ? pixel_format : opaque_pixel_format);
	pixman_format = find_format(format);

	int w = 0, h = 0;
	void* pixels = nullptr;

	bool img_okay = false;

	if (bytes > 4 && strncmp((char*) data, "XYZ1", 4) == 0)
		img_okay = ImageXYZ::ReadXYZ(data, bytes, transparent, w, h, pixels);
	else if (bytes > 2 && strncmp((char*) data, "BM", 2) == 0)
		img_okay = ImageBMP::ReadBMP(data, bytes, transparent, w, h, pixels);
	else if (bytes > 4 && strncmp((char*)(data + 1), "PNG", 3) == 0)
		img_okay = ImagePNG::ReadPNG((const void*) data, transparent, w, h, pixels);
	else
		Output::Warning("Unsupported image (Magic: {:02X})", bytes >= 4 ? *reinterpret_cast<const uint32_t*>(data) : 0);

	if (!img_okay) {
		free(pixels);
		pixels = nullptr;
		return;
	}

	Init(w, h, nullptr);

	ConvertImage(w, h, pixels, transparent);

	CheckPixels(flags);
}

Bitmap::Bitmap(Bitmap const& source, Rect const& src_rect, bool transparent) {
	format = (transparent ? pixel_format : opaque_pixel_format);
	pixman_format = find_format(format);

	Init(src_rect.width, src_rect.height, (void *) NULL);

	Blit(0, 0, source, src_rect, Opacity::Opaque());
}

bool Bitmap::WritePNG(Filesystem_Stream::OutputStream& os) const {
	size_t const width = GetWidth(), height = GetHeight();
	size_t const stride = width * 4;

	std::vector<uint32_t> data(width * height);

	auto dst = PixmanImagePtr{pixman_image_create_bits(PIXMAN_b8g8r8, width, height, &data.front(), stride)};
	pixman_image_composite32(PIXMAN_OP_SRC, bitmap.get(), NULL, dst.get(),
							 0, 0, 0, 0, 0, 0, width, height);

	return ImagePNG::WritePNG(os, width, height, &data.front());
}

size_t Bitmap::GetSize() const {
	if (!bitmap) {
		return 0;
	}

	return pitch() * height();
}

ImageOpacity Bitmap::ComputeImageOpacity() const {
	bool all_opaque = true;
	bool all_transp = true;
	bool alpha_1bit = true;

	auto* p = reinterpret_cast<const uint32_t*>(pixels());
	const auto mask = pixel_format.rgba_to_uint32_t(0, 0, 0, 0xFF);

	int n = GetSize() / sizeof(uint32_t);
	for (int i = 0; i < n; ++i ) {
		auto px = p[i] & mask;
		bool transp = (px == 0);
		bool opaque = (px == mask);
		all_transp &= transp;
		all_opaque &= opaque;
		alpha_1bit &= (transp | opaque);
	}

	return
		all_transp ? ImageOpacity::Transparent :
		all_opaque ? ImageOpacity::Opaque :
		alpha_1bit ? ImageOpacity::Alpha_1Bit :
		ImageOpacity::Alpha_8Bit;
}

ImageOpacity Bitmap::ComputeImageOpacity(Rect rect) const {
	bool all_opaque = true;
	bool all_transp = true;
	bool alpha_1bit = true;

	const auto full_rect = GetRect();
	rect = full_rect.GetSubRect(rect);

	auto* p = reinterpret_cast<const uint32_t*>(pixels());
	const int stride = pitch() / sizeof(uint32_t);
	const auto mask = pixel_format.rgba_to_uint32_t(0, 0, 0, 0xFF);

	int xend = (rect.x + rect.width);
	int yend = (rect.y + rect.height);
	for (int y = rect.y * stride; y < yend * stride; y += stride) {
		for (int x = rect.x; x < xend; ++x) {
			auto px = p[x + y] & mask;
			bool transp = (px == 0);
			bool opaque = (px == mask);
			all_transp &= transp;
			all_opaque &= opaque;
			alpha_1bit &= (transp | opaque);
		}
	}

	return
		all_transp ? ImageOpacity::Transparent :
		all_opaque ? ImageOpacity::Opaque :
		alpha_1bit ? ImageOpacity::Alpha_1Bit :
		ImageOpacity::Alpha_8Bit;
}

void Bitmap::CheckPixels(uint32_t flags) {
	if (flags & Flag_System) {
		DynamicFormat format(32,8,24,8,16,8,8,8,0,PF::Alpha);
		uint32_t pixel;
		Bitmap bmp(reinterpret_cast<void*>(&pixel), 1, 1, 4, format);
		pixman_image_composite32(PIXMAN_OP_SRC, bitmap.get(), (pixman_image_t*) NULL, bmp.bitmap.get(),
								 0, 32,  0, 0,  0, 0,  1, 1);
		bg_color = Color((int)(pixel>>24)&0xFF, (int)(pixel>>16)&0xFF, (int)(pixel>>8)&0xFF, (int)pixel&0xFF);
		pixman_image_composite32(PIXMAN_OP_SRC, bitmap.get(), (pixman_image_t*) NULL, bmp.bitmap.get(),
								 16, 32,  0, 0,  0, 0,  1, 1);
		sh_color = Color((int)(pixel>>24)&0xFF, (int)(pixel>>16)&0xFF, (int)(pixel>>8)&0xFF, (int)pixel&0xFF);
	}

	if (flags & Flag_Chipset) {
		const int h = height() / TILE_SIZE;
		const int w = width() / TILE_SIZE;
		tile_opacity = TileOpacity(w, h);

		for (int ty = 0; ty < h; ++ty) {
			for (int tx = 0; tx < w; ++tx) {
				Rect rect(tx * TILE_SIZE, ty * TILE_SIZE, TILE_SIZE, TILE_SIZE);
				auto op = ComputeImageOpacity(rect);
				tile_opacity.Set(tx, ty, op);
			}
		}
	}

	if (flags & Flag_ReadOnly) {
		read_only = true;

		image_opacity = ComputeImageOpacity();
	}
}

Color Bitmap::GetColorAt(int x, int y) const {
	if (x < 0 || x >= width() || y < 0 || y >= height()) {
		return {};
	}

	Color color;

	const uint8_t* pos = &reinterpret_cast<const uint8_t*>(pixels())[y * pitch() + x * bpp()];
	uint32_t pixel = *reinterpret_cast<const uint32_t*>(pos);
	format.uint32_to_rgba(pixel, color.red, color.green, color.blue, color.alpha);

	return color;
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
	bmp.Blit(0, 0, src, src_rect, Opacity::Opaque());

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

	Blit(dst_rect.x, dst_rect.y, bmp, bmp.GetRect(), Opacity::Opaque());
}

Point Bitmap::TextDraw(Rect const& rect, int color, StringView text, Text::Alignment align) {
	FontRef font = Font::Default();

	switch (align) {
	case Text::AlignLeft:
		return TextDraw(rect.x, rect.y, color, text);
		break;
	case Text::AlignCenter: {
		Rect text_rect = Text::GetSize(*font, text);
		int dx = rect.x + (rect.width - text_rect.width) / 2;
		return TextDraw(dx, rect.y, color, text);
		break;
	}
	case Text::AlignRight: {
		Rect text_rect = Text::GetSize(*font, text);
		int dx = rect.x + rect.width - text_rect.width;
		return TextDraw(dx, rect.y, color, text);
		break;
	}
	default: assert(false);
	}

	return Point();
}

Point Bitmap::TextDraw(int x, int y, int color, StringView text, Text::Alignment align) {
	auto font = Font::Default();
	auto system = Cache::SystemOrBlack();
	return Text::Draw(*this, x, y, *font, *system, color, text, align);
}

Point Bitmap::TextDraw(Rect const& rect, Color color, StringView text, Text::Alignment align) {
	FontRef font = Font::Default();

	switch (align) {
	case Text::AlignLeft:
		return TextDraw(rect.x, rect.y, color, text);
		break;
	case Text::AlignCenter: {
		Rect text_rect = Text::GetSize(*font, text);
		int dx = rect.x + (rect.width - text_rect.width) / 2;
		return TextDraw(dx, rect.y, color, text);
		break;
	}
	case Text::AlignRight: {
		Rect text_rect = Text::GetSize(*font, text);
		int dx = rect.x + rect.width - text_rect.width;
		return TextDraw(dx, rect.y, color, text);
		break;
	}
	default: assert(false);
	}

	return Point();
}

Point Bitmap::TextDraw(int x, int y, Color color, StringView text) {
	auto font = Font::Default();
	return Text::Draw(*this, x, y, *font, color, text);
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

static constexpr std::array<std::pair<int,pixman_format_code_t>, 27> formats_map = {{
		{ DynamicFormat(32,8,24,8,16,8,8,8,0,PF::Alpha).code_alpha(), PIXMAN_r8g8b8a8 },
		{ DynamicFormat(32,8,24,8,16,8,8,8,0,PF::NoAlpha).code_alpha(), PIXMAN_r8g8b8x8 },

		{ DynamicFormat(32,8,16,8,8,8,0,8,24,PF::Alpha).code_alpha(), PIXMAN_a8r8g8b8 },
		{ DynamicFormat(32,8,16,8,8,8,0,0,0,PF::NoAlpha).code_alpha(), PIXMAN_x8r8g8b8 },
		{ DynamicFormat(32,8,0,8,8,8,16,8,24,PF::Alpha).code_alpha(), PIXMAN_a8b8g8r8 },
		{ DynamicFormat(32,8,0,8,8,8,16,0,0,PF::NoAlpha).code_alpha(), PIXMAN_x8b8g8r8 },
		{ DynamicFormat(32,8,8,8,16,8,24,8,0,PF::Alpha).code_alpha(), PIXMAN_b8g8r8a8 },
		{ DynamicFormat(32,8,8,8,16,8,24,0,0,PF::NoAlpha).code_alpha(), PIXMAN_b8g8r8x8 },

		{ DynamicFormat(32,6,12,6,6,6,0,0,0,PF::NoAlpha).code_alpha(), PIXMAN_x14r6g6b6 },
		{ DynamicFormat(32,10,20,10,10,10,0,0,0,PF::NoAlpha).code_alpha(), PIXMAN_x2r10g10b10 },
		{ DynamicFormat(32,10,20,10,10,10,0,2,30,PF::Alpha).code_alpha(), PIXMAN_a2r10g10b10 },
		{ DynamicFormat(32,10,0,10,10,10,20,0,0,PF::NoAlpha).code_alpha(), PIXMAN_x2b10g10r10 },
		{ DynamicFormat(32,10,0,10,10,10,20,2,30,PF::Alpha).code_alpha(), PIXMAN_a2b10g10r10 },

		{ DynamicFormat(24,8,16,8,8,8,0,0,0,PF::NoAlpha).code_alpha(), PIXMAN_r8g8b8 },
		{ DynamicFormat(24,8,0,8,8,8,16,0,0,PF::NoAlpha).code_alpha(), PIXMAN_b8g8r8 },

		{ DynamicFormat(16,5,11,6,5,5,0,0,0,PF::NoAlpha).code_alpha(), PIXMAN_r5g6b5 },
		{ DynamicFormat(16,5,0,6,5,5,11,0,0,PF::NoAlpha).code_alpha(), PIXMAN_b5g6r5 },
		{ DynamicFormat(16,5,10,5,5,5,0,1,15,PF::Alpha).code_alpha(), PIXMAN_a1r5g5b5 },
		{ DynamicFormat(16,5,10,5,5,5,0,0,0,PF::NoAlpha).code_alpha(), PIXMAN_x1r5g5b5 },
		{ DynamicFormat(16,5,0,5,5,5,10,1,15,PF::Alpha).code_alpha(), PIXMAN_a1b5g5r5 },
		{ DynamicFormat(16,5,0,5,5,5,10,0,0,PF::NoAlpha).code_alpha(), PIXMAN_x1b5g5r5 },
		{ DynamicFormat(16,4,8,4,4,4,0,4,12,PF::Alpha).code_alpha(), PIXMAN_a4r4g4b4 },
		{ DynamicFormat(16,4,8,4,4,4,0,0,0,PF::NoAlpha).code_alpha(), PIXMAN_x4r4g4b4 },
		{ DynamicFormat(16,4,0,4,4,4,8,4,12,PF::Alpha).code_alpha(), PIXMAN_a4b4g4r4 },
		{ DynamicFormat(16,4,0,4,4,4,8,0,0,PF::NoAlpha).code_alpha(), PIXMAN_x4b4g4r4 },
		{ DynamicFormat(8,8,0,8,0,8,0,8,0,PF::Alpha).code_alpha(), PIXMAN_g8 },
		{ DynamicFormat(8,8,0,8,0,8,0,0,0,PF::NoAlpha).code_alpha(), PIXMAN_g8 }
}};

pixman_format_code_t Bitmap::find_format(const DynamicFormat& format) {
	auto dcode = format.code_alpha();
	auto iter = std::find_if(formats_map.begin(), formats_map.end(), [dcode](const auto& p) { return p.first == dcode; });
	if (iter == formats_map.end()) {
		// To fix add a pair to initialize_formats that maps the outputted
		// DynamicFormat to a pixman format
		Output::Error("{}\nDynamicFormat({}, {}, {}, {}, {}, {}, {}, {}, {}, {})",
		"Couldn't find Pixman format for",
		format.bits,
		format.r.bits, format.r.shift,
		format.g.bits, format.g.shift,
		format.b.bits, format.b.shift,
		format.a.bits, format.a.shift,
		format.alpha_type == PF::Alpha ? "PF::Alpha" : "PF::NoAlpha");
	}
	return iter->second;
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

	bitmap.reset(pixman_image_create_bits(pixman_format, width, height, (uint32_t*) data, pitch));

	if (bitmap == NULL) {
		Output::Error("Couldn't create {}x{} image.", width, height);
	}

	if (format.bits == 8) {
		initialize_palette();
		pixman_image_set_indexed(bitmap.get(), &palette);
	}

	if (data != NULL && destroy)
		pixman_image_set_destroy_function(bitmap.get(), destroy_func, data);
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
	BlitFast(0, 0, src, src.GetRect(), Opacity::Opaque());
	free(pixels);
}

void* Bitmap::pixels() {
	if (!bitmap) {
		return nullptr;
	}

	return (void*) pixman_image_get_data(bitmap.get());
}
void const* Bitmap::pixels() const {
	return (void const*) pixman_image_get_data(bitmap.get());
}

int Bitmap::bpp() const {
	return (pixman_image_get_depth(bitmap.get()) + 7) / 8;
}

int Bitmap::width() const {
	return pixman_image_get_width(bitmap.get());
}

int Bitmap::height() const {
	return pixman_image_get_height(bitmap.get());
}

int Bitmap::pitch() const {
	return pixman_image_get_stride(bitmap.get());
}

namespace {
	PixmanImagePtr CreateMask(Opacity const& opacity, Rect const& src_rect, Transform const* pxform = nullptr) {
		if (opacity.IsOpaque()) {
			return nullptr;
		}

		if (!opacity.IsSplit()) {
			pixman_color_t tcolor = {0, 0, 0, static_cast<uint16_t>(opacity.Value() << 8)};
			return PixmanImagePtr{ pixman_image_create_solid_fill(&tcolor) };
		}

		auto mask = PixmanImagePtr{pixman_image_create_bits(PIXMAN_a8, 1, 2, (uint32_t*) NULL, 4)};
		uint32_t* pixels = pixman_image_get_data(mask.get());
		*reinterpret_cast<uint8_t*>(&pixels[0]) = (opacity.top & 0xFF);
		*reinterpret_cast<uint8_t*>(&pixels[1]) = (opacity.bottom & 0xFF);

		Transform xform = Transform::Scale(1.0 / src_rect.width, 1.0 / src_rect.height);
		xform *= Transform::Translation(0, opacity.split);

		if (pxform)
			xform *= *pxform;

		pixman_image_set_transform(mask.get(), &xform.matrix);

		return mask;
	}
} // anonymous namespace

void Bitmap::Blit(int x, int y, Bitmap const& src, Rect const& src_rect, Opacity const& opacity, Bitmap::BlendMode blend_mode) {
	if (opacity.IsTransparent()) {
		return;
	}

	auto mask = CreateMask(opacity, src_rect);

	pixman_image_composite32(src.GetOperator(mask.get(), blend_mode),
							 src.bitmap.get(),
							 mask.get(), bitmap.get(),
							 src_rect.x, src_rect.y,
							 0, 0,
							 x, y,
							 src_rect.width, src_rect.height);
}

void Bitmap::BlitFast(int x, int y, Bitmap const & src, Rect const & src_rect, Opacity const & opacity) {
	if (opacity.IsTransparent()) {
		return;
	}

	pixman_image_composite32(PIXMAN_OP_SRC,
		src.bitmap.get(),
		nullptr, bitmap.get(),
		src_rect.x, src_rect.y,
		0, 0,
		x, y,
		src_rect.width, src_rect.height);
}

PixmanImagePtr Bitmap::GetSubimage(Bitmap const& src, const Rect& src_rect) {
	uint8_t* pixels = (uint8_t*) src.pixels() + src_rect.x * src.bpp() + src_rect.y * src.pitch();
	return PixmanImagePtr{ pixman_image_create_bits(src.pixman_format, src_rect.width, src_rect.height,
									(uint32_t*) pixels, src.pitch()) };
}

void Bitmap::TiledBlit(Rect const& src_rect, Bitmap const& src, Rect const& dst_rect, Opacity const& opacity, Bitmap::BlendMode blend_mode) {
	TiledBlit(0, 0, src_rect, src, dst_rect, opacity, blend_mode);
}

void Bitmap::TiledBlit(int ox, int oy, Rect const& src_rect, Bitmap const& src, Rect const& dst_rect, Opacity const& opacity, Bitmap::BlendMode blend_mode) {
	if (opacity.IsTransparent()) {
		return;
	}

	if (ox >= src_rect.width)	ox %= src_rect.width;
	if (oy >= src_rect.height)	oy %= src_rect.height;
	if (ox < 0) ox += src_rect.width  * ((-ox + src_rect.width  - 1) / src_rect.width);
	if (oy < 0) oy += src_rect.height * ((-oy + src_rect.height - 1) / src_rect.height);

	auto src_bm = GetSubimage(src, src_rect);

	pixman_image_set_repeat(src_bm.get(), PIXMAN_REPEAT_NORMAL);

	auto mask = CreateMask(opacity, src_rect);

	pixman_image_composite32(src.GetOperator(mask.get(), blend_mode),
							 src_bm.get(), mask.get(), bitmap.get(),
							 ox, oy,
							 0, 0,
							 dst_rect.x, dst_rect.y,
							 dst_rect.width, dst_rect.height);
}

void Bitmap::StretchBlit(Bitmap const&  src, Rect const& src_rect, Opacity const& opacity, Bitmap::BlendMode blend_mode) {
	StretchBlit(GetRect(), src, src_rect, opacity, blend_mode);
}

void Bitmap::StretchBlit(Rect const& dst_rect, Bitmap const& src, Rect const& src_rect, Opacity const& opacity, Bitmap::BlendMode blend_mode) {
	if (opacity.IsTransparent()) {
		return;
	}

	double zoom_x = (double)src_rect.width  / dst_rect.width;
	double zoom_y = (double)src_rect.height / dst_rect.height;

	Transform xform = Transform::Scale(zoom_x, zoom_y);

	pixman_image_set_transform(src.bitmap.get(), &xform.matrix);

	auto mask = CreateMask(opacity, src_rect, &xform);

	pixman_image_composite32(src.GetOperator(mask.get(), blend_mode),
							 src.bitmap.get(), mask.get(), bitmap.get(),
							 src_rect.x / zoom_x, src_rect.y / zoom_y,
							 0, 0,
							 dst_rect.x, dst_rect.y,
							 dst_rect.width, dst_rect.height);

	pixman_image_set_transform(src.bitmap.get(), nullptr);
}

void Bitmap::WaverBlit(int x, int y, double zoom_x, double zoom_y, Bitmap const& src, Rect const& src_rect, int depth, double phase, Opacity const& opacity, Bitmap::BlendMode blend_mode) {
	if (opacity.IsTransparent()) {
		return;
	}

	Transform xform = Transform::Scale(1.0 / zoom_x, 1.0 / zoom_y);

	pixman_image_set_transform(src.bitmap.get(), &xform.matrix);

	auto mask = CreateMask(opacity, src_rect, &xform);

	int height = static_cast<int>(std::floor(src_rect.height * zoom_y));
	int width  = static_cast<int>(std::floor(src_rect.width * zoom_x));
	const auto xoff = src_rect.x * zoom_x;
	const auto yoff = src_rect.y * zoom_y;
	const auto yclip = y < 0 ? -y : 0;
	const auto yend = std::min(height, this->height() - y);
	for (int i = yclip; i < yend; i++) {
		int dy = y + i;
		// RPG_RT starts the effect from the top of the screen even if the image is clipped. The result
		// is that moving images which cross the top of the screen can appear to go too fast or too slow
		// in RPT_RT. The (i - yclip) is RPG_RT compatible behavior. Just (i) would be more correct.
		const double sy = (i - yclip) * (2 * M_PI) / (32.0 * zoom_y);
		const int offset = 2 * zoom_x * depth * std::sin(phase + sy);

		pixman_image_composite32(src.GetOperator(mask.get(), blend_mode),
								 src.bitmap.get(), mask.get(), bitmap.get(),
								 xoff, yoff + i,
								 0, i,
								 x + offset, dy,
								 width, 1);
	}

	pixman_image_set_transform(src.bitmap.get(), nullptr);
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

	pixman_box32_t box = { 0, 0, width(), height() };

	pixman_image_fill_boxes(PIXMAN_OP_SRC, bitmap.get(), &pcolor, 1, &box);
}

void Bitmap::FillRect(Rect const& dst_rect, const Color &color) {
	pixman_color_t pcolor = PixmanColor(color);

	auto timage = PixmanImagePtr{pixman_image_create_solid_fill(&pcolor)};

	pixman_image_composite32(PIXMAN_OP_OVER,
			timage.get(), nullptr, bitmap.get(),
			0, 0,
			0, 0,
			dst_rect.x, dst_rect.y,
			dst_rect.width, dst_rect.height);
}

void Bitmap::Clear() {
	if (!pixels()) {
		// Happens when height or width of bitmap are 0
		return;
	}

	memset(pixels(), '\0', height() * pitch());
}

void Bitmap::ClearRect(Rect const& dst_rect) {
	pixman_color_t pcolor = {};
	pixman_box32_t box = {
		dst_rect.x,
		dst_rect.y,
		dst_rect.x + dst_rect.width,
		dst_rect.y + dst_rect.height
	};

	box.x2 = Utils::Clamp<int32_t>(box.x2, 0, width());
	box.y2 = Utils::Clamp<int32_t>(box.y2, 0, height());

	pixman_image_fill_boxes(PIXMAN_OP_CLEAR, bitmap.get(), &pcolor, 1, &box);
}

// Hard light lookup table mapping source color to destination color
// FIXME: Replace this with std::array<std::array<uint8_t,256>,256> when we have C++17
struct HardLightTable {
	uint8_t table[256][256] = {};
};

static constexpr HardLightTable make_hard_light_lookup() {
	HardLightTable hl;
	for (int i = 0; i < 256; ++i) {
		for (int j = 0; j < 256; ++j) {
			int res = 0;
			if (i <= 128)
				res = (2 * i * j) / 255;
			else
				res = 255 - 2 * (255 - i) * (255 - j) / 255;
			hl.table[i][j] = res > 255 ? 255 : res < 0 ? 0 : res;
		}
	}
	return hl;
}

constexpr auto hard_light = make_hard_light_lookup();


// Saturation Tone Inline: Changes a pixel saturation
static inline void saturation_tone(uint32_t &src_pixel, const int saturation, const int rs, const int gs, const int bs, const int as) {
	// Algorithm from OpenPDN (MIT license)
	// Transformation in Y'CbCr color space
	uint8_t r = (src_pixel >> rs) & 0xFF;
	uint8_t g = (src_pixel >> gs) & 0xFF;
	uint8_t b = (src_pixel >> bs) & 0xFF;
	uint8_t a = (src_pixel >> as) & 0xFF;

	// Y' = 0.299 R' + 0.587 G' + 0.114 B'
	uint8_t lum = (7471 * b + 38470 * g + 19595 * r) >> 16;

	// Scale Cb/Cr by scale factor "sat"
	int red = ((lum * 1024 + (r - lum) * saturation) >> 10);
	red = red > 255 ? 255 : red < 0 ? 0 : red;
	int green = ((lum * 1024 + (g - lum) * saturation) >> 10);
	green = green > 255 ? 255 : green < 0 ? 0 : green;
	int blue = ((lum * 1024 + (b - lum) * saturation) >> 10);
	blue = blue > 255 ? 255 : blue < 0 ? 0 : blue;

	src_pixel = ((uint32_t)red << rs) | ((uint32_t)green << gs) | ((uint32_t)blue << bs) | ((uint32_t)a << as);
}

// Color Tone Inline: Changes color of a pixel by hard light table
static inline void color_tone(uint32_t &src_pixel, const Tone& tone, const int rs, const int gs, const int bs, const int as) {
	src_pixel = ((uint32_t)hard_light.table[tone.red][(src_pixel >> rs) & 0xFF] << rs)
		| ((uint32_t)hard_light.table[tone.green][(src_pixel >> gs) & 0xFF] << gs)
		| ((uint32_t)hard_light.table[tone.blue][(src_pixel >> bs) & 0xFF] << bs)
		| ((uint32_t)((src_pixel >> as) & 0xFF) << as);
}

static inline void color_tone_alpha(uint32_t &src_pixel, const Tone& tone, const int rs, const int gs, const int bs, const int as) {
	uint8_t a = (src_pixel >> as) & 0xFF;
	uint8_t r = ((uint32_t)hard_light.table[tone.red][(src_pixel >> rs) & 0xFF]) * a / 255;
	uint8_t g = ((uint32_t)hard_light.table[tone.green][(src_pixel >> gs) & 0xFF]) * a / 255;
	uint8_t b = ((uint32_t)hard_light.table[tone.blue][(src_pixel >> bs) & 0xFF]) * a / 255;
	src_pixel = ((uint32_t)r << rs) | ((uint32_t)g << gs) | ((uint32_t)b << bs) | ((uint32_t)a << as);
}

void Bitmap::ToneBlit(int x, int y, Bitmap const& src, Rect const& src_rect, const Tone &tone, Opacity const& opacity) {
	if (opacity.IsTransparent()) {
		return;
	}

	// Optimisations based on Opacity:
	// Transparent: Nothing to do
	// Opaque: Alpha check can be skipped
	// 1 Bit: Premultiplied Alpha can be skipped
	// 8 Bit: No optimisations possible

	auto src_opacity = src.GetImageOpacity();

	if (src_opacity == ImageOpacity::Transparent) {
		return;
	}

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

	if (&src != this) {
		pixman_image_composite32(src.GetOperator(),
		src.bitmap.get(), nullptr, bitmap.get(),
		src_rect.x, src_rect.y,
		0, 0,
		x, y,
		src_rect.width, src_rect.height);
	}

	const int as = pixel_format.a.shift;
	const int rs = pixel_format.r.shift;
	const int gs = pixel_format.g.shift;
	const int bs = pixel_format.b.shift;
	int next_row = pitch() / sizeof(uint32_t);
	uint32_t* pixels = (uint32_t*)this->pixels();
	pixels = pixels + (y - 1) * next_row + x;

	const uint16_t limit_height = std::min<uint16_t>(src_rect.height, height());
	const uint16_t limit_width = std::min<uint16_t>(src_rect.width, width());

	const bool apply_sat = tone.gray != 128;
	const bool apply_tone = (tone.red != 128 || tone.green != 128 || tone.blue != 128);

	// If Saturation + Color:
	if (apply_sat && apply_tone) {
		int sat = tone.gray > 128 ? 1024 + (tone.gray - 128) * 16 : tone.gray * 8;

		if (src_opacity == ImageOpacity::Opaque) {
			for (uint16_t i = 0; i < limit_height; ++i) {
				pixels += next_row;
				for (uint16_t j = 0; j < limit_width; ++j) {
					saturation_tone(pixels[j], sat, rs, gs, bs, as);
					color_tone(pixels[j], tone, rs, gs, bs, as);
				}
			}
		} else if (src_opacity == ImageOpacity::Alpha_1Bit) {
			for (uint16_t i = 0; i < limit_height; ++i) {
				pixels += next_row;
				for (uint16_t j = 0; j < limit_width; ++j) {
					uint8_t a = (uint8_t)((pixels[j] >> as) & 0xFF);
					if (a == 0)
						continue;

					saturation_tone(pixels[j], sat, rs, gs, bs, as);
					color_tone(pixels[j], tone, rs, gs, bs, as);
				}
			}
		} else { // 8 Bit Alpha
			for (uint16_t i = 0; i < limit_height; ++i) {
				pixels += next_row;
				for (uint16_t j = 0; j < limit_width; ++j) {
					uint8_t a = (uint8_t)((pixels[j] >> as) & 0xFF);
					if (a == 0) {
						continue;
					}

					saturation_tone(pixels[j], sat, rs, gs, bs, as);
					color_tone_alpha(pixels[j], tone, rs, gs, bs, as);
				}
			}
		}
	}

	// If Only Saturation:
	else if (apply_sat) {
		int sat = tone.gray > 128 ? 1024 + (tone.gray - 128) * 16 : tone.gray * 8;

		if (src_opacity == ImageOpacity::Opaque) {
			for (uint16_t i = 0; i < limit_height; ++i) {
				pixels += next_row;
				for (uint16_t j = 0; j < limit_width; ++j) {
					saturation_tone(pixels[j], sat, rs, gs, bs, as);
				}
			}
		} else { // Any kind of alpha
			for (uint16_t i = 0; i < limit_height; ++i) {
				pixels += next_row;
				for (uint16_t j = 0; j < limit_width; ++j) {
					uint8_t a = (uint8_t)((pixels[j] >> as) & 0xFF);
					if (a == 0)
						continue;

					saturation_tone(pixels[j], sat, rs, gs, bs, as);
				}
			}
		}
	}

	// If Only Color:
	else if (apply_tone) {
		if (src_opacity == ImageOpacity::Opaque) {
			for (uint16_t i = 0; i < limit_height; ++i) {
				pixels += next_row;
				for (uint16_t j = 0; j < limit_width; ++j) {
					color_tone(pixels[j], tone, rs, gs, bs, as);
				}
			}
		} else if (src_opacity == ImageOpacity::Alpha_1Bit) {
			for (uint16_t i = 0; i < limit_height; ++i) {
				pixels += next_row;
				for (uint16_t j = 0; j < limit_width; ++j) {
					uint8_t a = (uint8_t)((pixels[j] >> as) & 0xFF);
					if (a == 0)
						continue;

					color_tone(pixels[j], tone, rs, gs, bs, as);
				}
			}
		} else { // 8 Bit Alpha
			for (uint16_t i = 0; i < limit_height; ++i) {
				pixels += next_row;
				for (uint16_t j = 0; j < limit_width; ++j) {
					uint8_t a = (uint8_t)((pixels[j] >> as) & 0xFF);
					if (a == 0)
						continue;
					else if (a == 255) {
						color_tone(pixels[j], tone, rs, gs, bs, as);
					} else {
						color_tone_alpha(pixels[j], tone, rs, gs, bs, as);
					}
				}
			}
		}
	}
}

void Bitmap::BlendBlit(int x, int y, Bitmap const& src, Rect const& src_rect, const Color& color, Opacity const& opacity) {
	if (opacity.IsTransparent()) {
		return;
	}

	if (color.alpha == 0) {
		if (&src != this)
			Blit(x, y, src, src_rect, opacity);
		return;
	}

	if (&src != this)
		pixman_image_composite32(src.GetOperator(),
								 src.bitmap.get(), nullptr, bitmap.get(),
								 src_rect.x, src_rect.y,
								 0, 0,
								 x, y,
								 src_rect.width, src_rect.height);

	pixman_color_t tcolor = PixmanColor(color);
	auto timage = PixmanImagePtr{ pixman_image_create_solid_fill(&tcolor) };

	pixman_image_composite32(PIXMAN_OP_OVER,
							 timage.get(), src.bitmap.get(), bitmap.get(),
							 0, 0,
							 src_rect.x, src_rect.y,
							 x, y,
							 src_rect.width, src_rect.height);
}

void Bitmap::FlipBlit(int x, int y, Bitmap const& src, Rect const& src_rect, bool horizontal, bool vertical, Opacity const& opacity, Bitmap::BlendMode blend_mode) {
	if (opacity.IsTransparent()) {
		return;
	}

	bool has_xform = (horizontal || vertical);
	const auto img_w = src.GetWidth();
	const auto img_h = src.GetHeight();

	auto rect = src_rect;
	if (has_xform) {
		Transform xform = Transform::Scale(horizontal ? -1 : 1, vertical ? -1 : 1);
		xform *= Transform::Translation(horizontal ? -img_w : 0, vertical ? -img_h : 0);

		pixman_image_set_transform(src.bitmap.get(), &xform.matrix);
		const auto src_x = horizontal ? img_w - src_rect.x - src_rect.width : src_rect.x;
		const auto src_y = vertical ? img_h - src_rect.y - src_rect.height : src_rect.y;

		rect = Rect{ src_x, src_y, src_rect.width, src_rect.height };
	}

	Blit(x, y, src, rect, opacity, blend_mode);

	if (has_xform) {
		pixman_image_set_transform(src.bitmap.get(), nullptr);
	}
}

void Bitmap::Flip(bool horizontal, bool vertical) {
	if (!horizontal && !vertical) {
		return;
	}
	const auto w = GetWidth();
	const auto h = GetHeight();
	const auto p = pitch();

	auto temp = PixmanImagePtr{ pixman_image_create_bits(pixman_format, w, h, nullptr, p) };

	std::memcpy(pixman_image_get_data(temp.get()),
			pixman_image_get_data(bitmap.get()),
			p * h);

	Transform xform = Transform::Scale(horizontal ? -1 : 1, vertical ? -1 : 1);
	xform *= Transform::Translation(horizontal ? -w : 0, vertical ? -h : 0);

	pixman_image_set_transform(temp.get(), &xform.matrix);

	pixman_image_composite32(PIXMAN_OP_SRC,
							 temp.get(), nullptr, bitmap.get(),
							 0, 0, 0, 0, 0, 0, w, h);
}

void Bitmap::MaskedBlit(Rect const& dst_rect, Bitmap const& mask, int mx, int my, Color const& color) {
	pixman_color_t tcolor = {
		static_cast<uint16_t>(color.red << 8),
		static_cast<uint16_t>(color.green << 8),
		static_cast<uint16_t>(color.blue << 8),
		static_cast<uint16_t>(color.alpha << 8)};

	auto source = PixmanImagePtr{ pixman_image_create_solid_fill(&tcolor) };

	pixman_image_composite32(PIXMAN_OP_OVER,
							 source.get(), mask.bitmap.get(), bitmap.get(),
							 0, 0,
							 mx, my,
							 dst_rect.x, dst_rect.y,
							 dst_rect.width, dst_rect.height);
}

void Bitmap::MaskedBlit(Rect const& dst_rect, Bitmap const& mask, int mx, int my, Bitmap const& src, int sx, int sy) {
	pixman_image_composite32(PIXMAN_OP_OVER,
							 src.bitmap.get(), mask.bitmap.get(), bitmap.get(),
							 sx, sy,
							 mx, my,
							 dst_rect.x, dst_rect.y,
							 dst_rect.width, dst_rect.height);
}

void Bitmap::Blit2x(Rect const& dst_rect, Bitmap const& src, Rect const& src_rect) {
	Transform xform = Transform::Scale(0.5, 0.5);

	pixman_image_set_transform(src.bitmap.get(), &xform.matrix);

	pixman_image_composite32(PIXMAN_OP_SRC,
							 src.bitmap.get(), nullptr, bitmap.get(),
							 src_rect.x, src_rect.y,
							 0, 0,
							 dst_rect.x, dst_rect.y,
							 dst_rect.width, dst_rect.height);

	pixman_image_set_transform(src.bitmap.get(), nullptr);
}

void Bitmap::EffectsBlit(int x, int y, int ox, int oy,
						 Bitmap const& src, Rect const& src_rect,
						 Opacity const& opacity,
						 double zoom_x, double zoom_y, double angle,
						 int waver_depth, double waver_phase, Bitmap::BlendMode blend_mode) {
	if (opacity.IsTransparent()) {
		return;
	}

	bool rotate = angle != 0.0;
	bool scale = zoom_x != 1.0 || zoom_y != 1.0;
	bool waver = waver_depth != 0;

	if (waver) {
		WaverBlit(x - ox * zoom_x, y - oy * zoom_y, zoom_x, zoom_y, src, src_rect,
				  waver_depth, waver_phase, opacity, blend_mode);
	}
	else if (rotate) {
		RotateZoomOpacityBlit(x, y, ox, oy, src, src_rect, angle, zoom_x, zoom_y, opacity, blend_mode);
	}
	else if (scale) {
		ZoomOpacityBlit(x, y, ox, oy, src, src_rect, zoom_x, zoom_y, opacity, blend_mode);
	}
	else {
		Blit(x - ox, y - oy, src, src_rect, opacity, blend_mode);
	}
}

void Bitmap::RotateZoomOpacityBlit(int x, int y, int ox, int oy,
		Bitmap const& src, Rect const& src_rect,
		double angle, double zoom_x, double zoom_y, Opacity const& opacity, Bitmap::BlendMode blend_mode)
{
	if (opacity.IsTransparent()) {
		return;
	}

	auto* src_img = src.bitmap.get();

	Transform fwd = Transform::Translation(x, y);
	fwd *= Transform::Rotation(angle);
	if (zoom_x != 1.0 || zoom_y != 1.0) {
		fwd *= Transform::Scale(zoom_x, zoom_y);
	}
	fwd *= Transform::Translation(-ox, -oy);

	Rect dst_rect = TransformRectangle(fwd, Rect{0, 0, src_rect.width, src_rect.height});
	dst_rect.Adjust(GetRect());
	if (dst_rect.IsEmpty())
		return;

	auto inv = fwd.Inverse();

	PixmanImagePtr temp;
	if (src_rect != src.GetRect()) {
		temp = GetSubimage(src, src_rect);
		src_img = temp.get();
	}

	pixman_image_set_transform(src_img, &inv.matrix);

	auto mask = CreateMask(opacity, src_rect, &inv);

	// OP_SRC draws a black rectangle around the rotated image making this operator unusable here
	blend_mode = (blend_mode == BlendMode::Default ? BlendMode::Normal : blend_mode);
	pixman_image_composite32(GetOperator(mask.get(), blend_mode),
							 src_img, mask.get(), bitmap.get(),
							 dst_rect.x, dst_rect.y,
							 dst_rect.x, dst_rect.y,
							 dst_rect.x, dst_rect.y,
							 dst_rect.width, dst_rect.height);

	pixman_image_set_transform(src_img, nullptr);
}

void Bitmap::ZoomOpacityBlit(int x, int y, int ox, int oy,
							 Bitmap const& src, Rect const& src_rect,
							 double zoom_x, double zoom_y,
							 Opacity const& opacity, Bitmap::BlendMode blend_mode)
{
	if (opacity.IsTransparent()) {
		return;
	}

	Rect dst_rect(
		x - static_cast<int>(std::floor(ox * zoom_x)),
		y - static_cast<int>(std::floor(oy * zoom_y)),
		static_cast<int>(std::floor(src_rect.width * zoom_x)),
		static_cast<int>(std::floor(src_rect.height * zoom_y)));
	StretchBlit(dst_rect, src, src_rect, opacity, blend_mode);
}

pixman_op_t Bitmap::GetOperator(pixman_image_t* mask, Bitmap::BlendMode blend_mode) const {
	if (blend_mode != BlendMode::Default) {
		switch (blend_mode) {
			case BlendMode::Normal:
				return PIXMAN_OP_OVER;
			case BlendMode::NormalWithoutAlpha:
				return PIXMAN_OP_SRC;
			case BlendMode::XOR:
				return PIXMAN_OP_XOR;
			case BlendMode::Additive:
				return PIXMAN_OP_ADD;
			case BlendMode::Multiply:
				return PIXMAN_OP_MULTIPLY;
			case BlendMode::Overlay:
				return PIXMAN_OP_OVERLAY;
			case BlendMode::Saturate:
				return PIXMAN_OP_SATURATE;
			case BlendMode::Darken:
				return PIXMAN_OP_DARKEN;
			case BlendMode::Lighten:
				return PIXMAN_OP_LIGHTEN;
			case BlendMode::ColorDodge:
				return PIXMAN_OP_COLOR_DODGE;
			case BlendMode::ColorBurn:
				return PIXMAN_OP_COLOR_BURN;
			case BlendMode::Difference:
				return PIXMAN_OP_DIFFERENCE;
			case BlendMode::Exclusion:
				return PIXMAN_OP_EXCLUSION;
			case BlendMode::SoftLight:
				return PIXMAN_OP_SOFT_LIGHT;
			case BlendMode::HardLight:
				return PIXMAN_OP_HARD_LIGHT;
			default:
				return PIXMAN_OP_CLEAR;
		}
	}

	if (!mask && (!GetTransparent() || GetImageOpacity() == ImageOpacity::Opaque)) {
		return PIXMAN_OP_SRC;
	}

	return PIXMAN_OP_OVER;
}

void Bitmap::EdgeMirrorBlit(int x, int y, Bitmap const& src, Rect const& src_rect, bool mirror_x, bool mirror_y, Opacity const& opacity) {
	if (opacity.IsTransparent())
		return;

	auto mask = CreateMask(opacity, src_rect);

	const auto dst_rect = GetRect();

	auto draw = [&](int x, int y) {
		pixman_image_composite32(src.GetOperator(mask.get()),
				src.bitmap.get(),
				mask.get(), bitmap.get(),
				src_rect.x, src_rect.y,
				0, 0,
				x, y,
				src_rect.width, src_rect.height);
	};

	draw(x, y);

	const bool clone_x = (mirror_x && x + src_rect.width > dst_rect.width);
	const bool clone_y = (mirror_y && y + src_rect.height > dst_rect.height);

	if (clone_x) {
		draw(x - dst_rect.width, y);
	}

	if (clone_y) {
		draw(x, y - dst_rect.height);
	}

	if (clone_x && clone_y) {
		draw(x - dst_rect.width, y - dst_rect.height);
	}
}

