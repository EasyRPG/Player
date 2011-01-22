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
#ifdef USE_PIXMAN_BITMAP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <png.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H
#include "cache.h"
#include "filefinder.h"
#include "options.h"
#include "data.h"
#include "output.h"
#include "utils.h"
#include "pixman_bitmap.h"

////////////////////////////////////////////////////////////
void PixmanBitmap::Init(int width, int height) {
	bitmap = pixman_image_create_bits(PIXMAN_a8r8g8b8, width, height, (uint32_t*) NULL, width*4);

	if (bitmap == NULL) {
		Output::Error("Couldn't create %dx%d image.\n", width, height);
	}
}

////////////////////////////////////////////////////////////
static void read_data(png_structp png_ptr, png_bytep data, png_size_t length) {
    png_bytep* bufp = (png_bytep*) png_get_io_ptr(png_ptr);
	memcpy(data, *bufp, length);
	*bufp += length;
}

////////////////////////////////////////////////////////////
void PixmanBitmap::ReadPNG(FILE *stream, const void *buffer) {
	bitmap = NULL;

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
		png_set_read_fn(png_ptr, (voidp) &buffer, read_data);

	png_read_info(png_ptr, info_ptr);

	png_uint_32 width, height;
	int bit_depth, color_type;
	png_get_IHDR(png_ptr, info_ptr, &width, &height,
				 &bit_depth, &color_type, NULL, NULL, NULL);

	png_color black = {0,0,0};
	png_colorp palette;
	int num_palette = 0;

	switch (color_type) {
		case PNG_COLOR_TYPE_PALETTE:
			if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
				png_set_tRNS_to_alpha(png_ptr);
			else if (transparent && png_get_valid(png_ptr, info_ptr, PNG_INFO_PLTE))
				png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);
			png_set_palette_to_rgb(png_ptr);
			png_set_swap_alpha(png_ptr);
			png_set_filler(png_ptr, 0xFF, PNG_FILLER_BEFORE);
			break;
		case PNG_COLOR_TYPE_GRAY:
			png_set_gray_to_rgb(png_ptr);
			if (bit_depth < 8)
				png_set_expand_gray_1_2_4_to_8(png_ptr);
			png_set_filler(png_ptr, 0xFF, PNG_FILLER_BEFORE);
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
			png_set_filler(png_ptr, 0xFF, PNG_FILLER_BEFORE);
			break;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			png_set_swap_alpha(png_ptr);
			break;
	}

	if (bit_depth < 8)
		png_set_packing(png_ptr);

	if (bit_depth == 16)
		png_set_strip_16(png_ptr);

	png_read_update_info(png_ptr, info_ptr);

	Init(width, height);

	for (png_uint_32 y = 0; y < height; y++) {
		png_bytep dst = (png_bytep) pixels() + y * pitch();
		png_read_row(png_ptr, dst, NULL);
		for (png_uint_32 x = 0; x < width; x++) {
			uint32 a = dst[x * 4 + 0];
			uint32 r = dst[x * 4 + 1] * a / 0xFF;
			uint32 g = dst[x * 4 + 2] * a / 0xFF;
			uint32 b = dst[x * 4 + 3] * a / 0xFF;
			((uint32*)dst)[x] = (a<<24) | (r<<16) | (g<<8) | (b<<0);
		}
	}

	// TODO: stride
	if (transparent && num_palette > 0) {
		png_color& ck = palette[0];
		//uint8 ck1[4] = {255, ck.red, ck.green, ck.blue};
		//uint32 srckey = *(uint32*)ck1;
		uint32 srckey = GetUint32Color(ck.red, ck.green, ck.blue, 255);
		uint32 dstkey = 0;
		uint32* p = (uint32*) pixels();
		int pad = pitch() / bpp() - width;
		for (png_uint_32 y = 0; y < height; y++) {
			for (png_uint_32 x = 0; x < width; x++) {
				if (*p == srckey)
					*p = dstkey;
				p++;
			}
			p += pad;
		}
	}

	png_read_end(png_ptr, NULL);

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
}

////////////////////////////////////////////////////////////
void PixmanBitmap::ReadXYZ(const uint8 *data, uint len) {
	bitmap = NULL;

    if (len < 8 || strncmp((char *) data, "XYZ1", 4) != 0) {
		Output::Error("Not a valid XYZ file.");
		return;
    }

    unsigned short w = data[4] + (data[5] << 8);
    unsigned short h = data[6] + (data[7] << 8);

	uLongf src_size = len - 8;
    Bytef* src_buffer = (Bytef*)&data[8];
    uLongf dst_size = 768 + (w * h);
    Bytef* dst_buffer = new Bytef[dst_size];

    int status = uncompress(dst_buffer, &dst_size, src_buffer, src_size);
	if (status != Z_OK) {
		Output::Error("Error decompressing XYZ file.");
		return;
	}
    const uint8 (*palette)[3] = (const uint8(*)[3]) dst_buffer;

	Init(w, h);

    uint32* dst = (uint32*) pixels();
    const uint8* src = (const uint8*) &dst_buffer[768];
    for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			uint8 pix = *src++;
			const uint8* color = palette[pix];
			*dst++ = (transparent && pix == 0)
				? 0
				: ((0xFF<<24) | (color[0]<<16) | (color[1]<<8) | (color[2]<<0));
		}
    }

    delete[] dst_buffer;
}

void PixmanBitmap::ReadXYZ(FILE *stream) {
    fseek(stream, 0, SEEK_END);
    long size = ftell(stream);
    fseek(stream, 0, SEEK_SET);
	uint8* buffer = new uint8[size];
	fread((void*) buffer, 1, size, stream);
	ReadXYZ(buffer, (uint) size);
	delete[] buffer;
}

////////////////////////////////////////////////////////////
PixmanBitmap::PixmanBitmap(int width, int height, bool itransparent) {
	transparent = itransparent;

	Init(width, height);
}

PixmanBitmap::PixmanBitmap(const std::string filename, bool itransparent) {
	transparent = itransparent;

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
		ReadPNG(stream, (void*) NULL);
	else if (ext == "xyz")
		ReadXYZ(stream);

	fclose(stream);
}

PixmanBitmap::PixmanBitmap(const uint8* data, uint bytes, bool itransparent) {
	transparent = itransparent;

	if (bytes > 4 && strncmp((char*) data, "XYZ1", 4) == 0)
		ReadXYZ(data, bytes);
	else
		ReadPNG((FILE*) NULL, (const void*) data);
}

PixmanBitmap::PixmanBitmap(Bitmap* source, Rect src_rect, bool itransparent) {
	transparent = itransparent;

	Init(src_rect.width, src_rect.height);

	Blit(0, 0, source, src_rect, 255);
}

////////////////////////////////////////////////////////////
PixmanBitmap::~PixmanBitmap() {
	pixman_image_unref(bitmap);
}

////////////////////////////////////////////////////////////
void* PixmanBitmap::pixels() {
	return (void*) pixman_image_get_data(bitmap);
}

uint8 PixmanBitmap::bpp() const {
	return 4;
}

int PixmanBitmap::width() const {
	return pixman_image_get_width(bitmap);
}

int PixmanBitmap::height() const {
	return pixman_image_get_height(bitmap);
}

uint16 PixmanBitmap::pitch() const {
	return pixman_image_get_stride(bitmap);
}

uint32 PixmanBitmap::rmask() const {
	return RMASK;
}

uint32 PixmanBitmap::gmask() const {
	return GMASK;
}

uint32 PixmanBitmap::bmask() const {
	return BMASK;
}

uint32 PixmanBitmap::amask() const {
	return AMASK;
}

uint32 PixmanBitmap::colorkey() const {
	return 0;
}

////////////////////////////////////////////////////////////
void PixmanBitmap::Blit(int x, int y, Bitmap* _src, Rect src_rect, int opacity) {
	if (opacity < 0)
		return;

	PixmanBitmap* src = (PixmanBitmap*) _src;

	if (opacity > 255) opacity = 255;

	pixman_image_composite32(PIXMAN_OP_OVER,
							 src->bitmap, (pixman_image_t*) NULL, bitmap,
							 src_rect.x, src_rect.y,
							 0, 0,
							 x, y,
							 src_rect.width, src_rect.height);

	RefreshCallback();
}

void PixmanBitmap::Mask(int x, int y, Bitmap* _src, Rect src_rect) {
	PixmanBitmap* src = (PixmanBitmap*) _src;

	pixman_image_composite32(PIXMAN_OP_DISJOINT_IN_REVERSE,
							 src->bitmap, (pixman_image_t*) NULL, bitmap,
							 src_rect.x, src_rect.y,
							 0, 0,
							 x, y,
							 src_rect.width, src_rect.height);

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

void PixmanBitmap::Fill(const Color &color) {
	pixman_color_t pcolor = PixmanColor(color);
	pixman_rectangle16_t rect = {0, 0, width(), height()};

	pixman_image_fill_rectangles(PIXMAN_OP_SRC, bitmap, &pcolor, 1, &rect);

	RefreshCallback();
}

void PixmanBitmap::FillRect(Rect dst_rect, const Color &color) {
	pixman_color_t pcolor = PixmanColor(color);
	pixman_rectangle16_t rect = {dst_rect.x, dst_rect.y, dst_rect.width, dst_rect.height};

	pixman_image_fill_rectangles(PIXMAN_OP_SRC, bitmap, &pcolor, 1, &rect);

	RefreshCallback();
}

void PixmanBitmap::Clear() {
	pixman_color_t pcolor = {0, 0, 0, 0};
	pixman_rectangle16_t rect = {0, 0, width(), height()};

	pixman_image_fill_rectangles(PIXMAN_OP_CLEAR, bitmap, &pcolor, 1, &rect);

	RefreshCallback();
}

void PixmanBitmap::ClearRect(Rect dst_rect) {
	pixman_color_t pcolor = {0, 0, 0, 0};
	pixman_rectangle16_t rect = {dst_rect.x, dst_rect.y, dst_rect.width, dst_rect.height};

	pixman_image_fill_rectangles(PIXMAN_OP_CLEAR, bitmap, &pcolor, 1, &rect);

	RefreshCallback();
}

Bitmap* PixmanBitmap::Resample(int scale_w, int scale_h, const Rect& src_rect) {
	double zoom_x = (double)src_rect.width  / scale_w;
	double zoom_y = (double)src_rect.height / scale_h;

	pixman_transform_t xform;
	pixman_transform_init_scale(&xform,
								pixman_double_to_fixed(zoom_x),
								pixman_double_to_fixed(zoom_y));

	PixmanBitmap *resampled = new PixmanBitmap(scale_w, scale_h, transparent);
	
	pixman_image_set_transform(bitmap, &xform);

	pixman_image_composite32(PIXMAN_OP_SRC,
							 bitmap, (pixman_image_t*) NULL, resampled->bitmap,
							 src_rect.x, src_rect.y,
							 0, 0,
							 0, 0,
							 scale_w, scale_h);

	pixman_transform_init_identity(&xform);
	pixman_image_set_transform(bitmap, &xform);

	return resampled;
}

Bitmap* PixmanBitmap::RotateScale(double angle, int scale_w, int scale_h) {
	pixman_transform_t fwd, rev;
	pixman_transform_init_identity(&fwd);
	pixman_transform_init_identity(&rev);

	double zoom_x = (double) scale_w  / width();
	double zoom_y = (double) scale_h / height();
	pixman_transform_scale(&fwd, &rev,
						   pixman_double_to_fixed(zoom_x),
						   pixman_double_to_fixed(zoom_y));

	double c = cos(angle);
	double s = sin(angle);
	pixman_transform_rotate(&fwd, &rev,
							pixman_double_to_fixed(c),
							pixman_double_to_fixed(s));

	pixman_box16_t box = {0, 0, width(), height()};
	pixman_transform_bounds(&fwd, &box);
	int dst_w = box.x2 - box.x1;
	int dst_h = box.y2 - box.y1;

	pixman_transform_translate(&fwd, &rev,
							   pixman_int_to_fixed(-box.x1),
							   pixman_int_to_fixed(-box.y1));

	PixmanBitmap *resampled = new PixmanBitmap(dst_w, dst_h, true);
	resampled->Clear();

	pixman_image_set_transform(bitmap, &rev);

	pixman_image_composite32(PIXMAN_OP_SRC,
							 bitmap, (pixman_image_t*) NULL, resampled->bitmap,
							 0, 0,
							 0, 0,
							 0, 0,
							 dst_w, dst_h);

	pixman_transform_t xform;
	pixman_transform_init_identity(&xform);
	pixman_image_set_transform(bitmap, &xform);

	return resampled;
}

void PixmanBitmap::OpacityChange(int opacity, const Rect& dst_rect) {
	if (opacity == 255)
		return;

	pixman_color_t pcolor = {0, 0, 0, opacity << 8};
	pixman_rectangle16_t rect = {dst_rect.x, dst_rect.y, dst_rect.width, dst_rect.height};

	pixman_image_fill_rectangles(PIXMAN_OP_IN_REVERSE, bitmap, &pcolor, 1, &rect);

	RefreshCallback();
}

void PixmanBitmap::ToneChange(const Tone &tone) {
	if (tone == Tone()) return;

	pixman_rectangle16_t rect = {0, 0, width(), height()};

	if (tone.gray == 0) {
		pixman_color_t tcolor = {tone.red << 8, tone.green << 8, tone.blue << 8, 0xFFFF};
		pixman_image_t *timage = pixman_image_create_solid_fill(&tcolor);

		pixman_image_composite32(PIXMAN_OP_ADD,
								 timage, bitmap, bitmap,
								 0, 0,
								 0, 0,
								 0, 0,
								 rect.width, rect.height);

		pixman_image_unref(timage);
	}
	else {
		PixmanBitmap *gray = new PixmanBitmap(this, GetRect(), transparent);
		pixman_color_t gcolor = {0, 0, 0, 0xFFFF};
		pixman_image_fill_rectangles(PIXMAN_OP_HSL_SATURATION, gray->bitmap, &gcolor, 1, &rect);

		pixman_color_t acolor = {0, 0, 0, tone.gray << 8};
		pixman_image_fill_rectangles(PIXMAN_OP_IN_REVERSE, gray->bitmap, &acolor, 1, &rect);

		pixman_image_composite32(PIXMAN_OP_ATOP,
								 gray->bitmap, (pixman_image_t*) NULL, bitmap,
								 0, 0,
								 0, 0,
								 0, 0,
								 rect.width, rect.height);

		pixman_color_t tcolor = {tone.red << 8, tone.green << 8, tone.blue << 8, 0xFFFF};
		pixman_image_t *timage = pixman_image_create_solid_fill(&tcolor);

		pixman_image_composite32(PIXMAN_OP_ADD,
								 timage, bitmap, bitmap,
								 0, 0,
								 0, 0,
								 0, 0,
								 rect.width, rect.height);

		pixman_image_unref(timage);

		delete gray;
	}

	RefreshCallback();
}

void PixmanBitmap::Flip(bool horizontal, bool vertical) {
	if (!horizontal && !vertical) return;

	int w = width();
	int h = height();

	pixman_transform_t xform;
	pixman_transform_init_scale(&xform,
								pixman_int_to_fixed(horizontal ? -1 : 1),
								pixman_int_to_fixed(vertical ? -1 : 1));

	pixman_transform_translate((pixman_transform_t*) NULL, &xform,
							   pixman_int_to_fixed(horizontal ? w : 0),
							   pixman_int_to_fixed(vertical ? h : 0));

	PixmanBitmap *resampled = new PixmanBitmap(w, h, transparent);

	pixman_image_set_transform(bitmap, &xform);

	pixman_image_composite32(PIXMAN_OP_SRC,
							 bitmap, (pixman_image_t*) NULL, resampled->bitmap,
							 0, 0,
							 0, 0,
							 0, 0,
							 w, h);

	pixman_transform_init_identity(&xform);
	pixman_image_set_transform(bitmap, &xform);

	pixman_image_composite32(PIXMAN_OP_SRC,
							 resampled->bitmap, (pixman_image_t*) NULL, bitmap,
							 0, 0,
							 0, 0,
							 0, 0,
							 w, h);

	delete resampled;

	RefreshCallback();
}

FT_Library PixmanBitmap::library;
FT_Face PixmanBitmap::face;
bool PixmanBitmap::ft_initialized = false;

void PixmanBitmap::InitFreeType() {
	if (ft_initialized)
		return;

    FT_Error ans = FT_Init_FreeType(&library);
    if (ans != FT_Err_Ok) {
		Output::Error("Couldn't initialize FreeType\n");
		return;
	}

	std::string path = FileFinder::FindFont(font->name);
    ans = FT_New_Face(library, path.c_str(), 0, &face);
    if (ans != FT_Err_Ok) {
		Output::Error("Couldn't initialize FreeType face\n");
		FT_Done_FreeType(library);
		return;
	}

    ans = FT_Set_Char_Size(face, font->size * 64, font->size * 64, 72, 72);
    if (ans != FT_Err_Ok) {
		Output::Error("Couldn't set FreeType face size\n");
		FT_Done_Face(face);
		FT_Done_FreeType(library);
		return;
    }

	ft_initialized = true;
}

void PixmanBitmap::DoneFreeType() {
	if (!ft_initialized)
		return;

    FT_Done_Face(face);

    FT_Done_FreeType(library);

	ft_initialized = false;
}

PixmanBitmap* PixmanBitmap::RenderFreeTypeChar(int c) {
	FT_Error ans = FT_Load_Char(face, c, FT_LOAD_NO_BITMAP);
    if (ans != FT_Err_Ok) {
		Output::Error("Couldn't load FreeType character %d\n", c);
		return NULL;
	}

	ans = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);
    if (ans != FT_Err_Ok) {
		Output::Error("Couldn't render FreeType character %d\n", c);
		return NULL;
	}

	FT_Bitmap ft_bitmap;
	if (face->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY)
		ft_bitmap = face->glyph->bitmap;
	else {
		FT_Bitmap_New(&ft_bitmap);
		FT_Bitmap_Convert(library, &face->glyph->bitmap, &ft_bitmap, 4);
	}

	if (ft_bitmap.pixel_mode != FT_PIXEL_MODE_GRAY) {
		Output::Error("FreeType character has wrong format\n", c);
		return NULL;
	}

	const uint8* src = (const uint8*) ft_bitmap.buffer;
	if (ft_bitmap.pitch < 0)
		src -= ft_bitmap.rows * ft_bitmap.pitch;

	PixmanBitmap* bitmap = new PixmanBitmap(ft_bitmap.width, font->size + 2, true);
	uint8* dst = (uint8*) bitmap->pixels();

	const int base_line = bitmap->height() / 4;
	int offset = bitmap->height() - face->glyph->bitmap_top - base_line;

	for (int yd = 0; yd < bitmap->height(); yd++) {
		int ys = yd - offset;
		if (ys < 0 || ys >= ft_bitmap.rows)
			continue;
		const uint8* p = src + ys * ft_bitmap.pitch;
		uint32* q = (uint32*) (dst + yd * bitmap->pitch());
		for (int x = 0; x < ft_bitmap.width; x++)
			*q++ = (*p++ != 0) ? 0xFF000000 : 0x00000000;
	}

	return bitmap;
}

////////////////////////////////////////////////////////////
void PixmanBitmap::TextDraw(int x, int y, std::string text, TextAlignment align) {
	if (text.length() == 0) return;
	Rect dst_rect = GetTextSize(text);
	dst_rect.x = x; dst_rect.y = y;
	dst_rect.width += 1; dst_rect.height += 1; // Need place for shadow
	if (dst_rect.IsOutOfBounds(GetWidth(), GetHeight())) return;

	PixmanBitmap* text_surface; // Complete text will be on this surface

	text_surface = new PixmanBitmap(dst_rect.width, dst_rect.height, true);

	// Load the system file for the shadow and text color
	Bitmap* system = Cache::System(Data::system.system_name);
	// Load the exfont-file
	Bitmap* exfont = Cache::ExFont();

	// Get the Shadow color
	Color shadow_color(system->GetPixel(16, 32));
	// If shadow is pure black, add 1 to blue channel
	// so it doesn't become transparent
	if ((shadow_color.red == 0) &&
		(shadow_color.green == 0) &&
		(shadow_color.blue == 0) ) {
			// FIXME: what if running in 16 bpp?
		shadow_color.blue++;
	}

	// Where to draw the next glyph (x pos)
	int next_glyph_pos = 0;

	// The current char is a full size glyph
	bool is_full_glyph = false;
	// The current char is an exfont (is_full_glyph must be true too)
	bool is_exfont = false;

	InitFreeType();

	// This loops always renders a single char, color blends it and then puts
	// it onto the text_surface (including the drop shadow)
	for (unsigned c = 0; c < text.size(); ++c) {
		Rect next_glyph_rect(next_glyph_pos, 0, 0, 0);

		PixmanBitmap* mask;

		// ExFont-Detection: Check for A-Z or a-z behind the $
		if (text[c] == '$' && c != text.size() - 1 &&
			((text[c+1] >= 'a' && text[c+1] <= 'z') ||
			(text[c+1] >= 'A' && text[c+1] <= 'Z'))) {
			int exfont_value;
			// Calculate which exfont shall be rendered
			if ((text[c+1] >= 'a' && text[c+1] <= 'z')) {
				exfont_value = 26 + text[c+1] - 'a';
			} else {
				exfont_value = text[c+1] - 'A';
			}
			is_full_glyph = true;
			is_exfont = true;

			mask = new PixmanBitmap(12, 12, true);

			// Get exfont from graphic
			Rect rect_exfont((exfont_value % 13) * 12, (exfont_value / 13) * 12, 12, 12);

			// Create a mask
			mask->Clear();
			mask->Blit(0, 0, exfont, rect_exfont, 255);
		} else {
			// No ExFont, draw normal text

			mask = RenderFreeTypeChar(text[c]);
			if (mask == NULL) {
				Output::Warning("Couldn't render char %c (%d). Skipping...", text[c], (int)text[c]);
				continue;
			}
		}

		// Get color region from system graphic
		Rect clip_system(8+16*(font->color%10), 4+48+16*(font->color/10), 6, 12);

		PixmanBitmap* char_surface = new PixmanBitmap(mask->width(), mask->height(), true);

		// Blit gradient color background (twice in case of a full glyph)
		char_surface->Blit(0, 0, system, clip_system, 255);
		char_surface->Blit(6, 0, system, clip_system, 255);
		// Blit mask onto background
		char_surface->Mask(0, 0, mask, mask->GetRect());

		PixmanBitmap* char_shadow = new PixmanBitmap(mask->width(), mask->height(), true);

		// Blit solid color background
		char_shadow->Fill(shadow_color);
		// Blit mask onto background
		char_shadow->Mask(0, 0, mask, mask->GetRect());

		// Blit first shadow and then text
		text_surface->Blit(next_glyph_rect.x + 1, next_glyph_rect.y + 1, char_shadow, char_shadow->GetRect(), 255);
		text_surface->Blit(next_glyph_rect.x, next_glyph_rect.y, char_surface, char_surface->GetRect(), 255);

		delete mask;
		delete char_surface;
		delete char_shadow;

		// If it's a full size glyph, add the size of a half-size glypth twice
		if (is_full_glyph) {
			next_glyph_pos += 6;
			is_full_glyph = false;
			if (is_exfont) {
				is_exfont = false;
				// Skip the next character
				++c;
			}
		}
		next_glyph_pos += 6;	
	}

	DoneFreeType();

	Bitmap* text_bmp = CreateBitmap(text_surface, text_surface->GetRect());

	Rect src_rect(0, 0, dst_rect.width, dst_rect.height);
	int iy = dst_rect.y;
	if (dst_rect.height > text_bmp->GetHeight()) {
		iy += ((dst_rect.height - text_bmp->GetHeight()) / 2);
	}
	int ix = dst_rect.x;
	
	// Alignment code
	if (dst_rect.width > text_bmp->GetWidth()) {
		if (align == Bitmap::TextAlignCenter) {
			ix += (dst_rect.width - text_bmp->GetWidth()) / 2;
		} else if (align == Bitmap::TextAlignRight) {
			ix += dst_rect.width - text_bmp->GetWidth();
		}
	}

	Blit(ix, iy, text_bmp, src_rect, 255);

	delete text_bmp;
	delete text_surface;

	RefreshCallback();
}

////////////////////////////////////////////////////////////
void PixmanBitmap::SetTransparentColor(Color color) {
}

////////////////////////////////////////////////////////////
Color PixmanBitmap::GetColor(uint32 uint32_color) const {
	uint8 r, g, b, a;
	GetColorComponents(uint32_color, r, g, b, a);
	return Color(r, g, b, a);
}

uint32 PixmanBitmap::GetUint32Color(const Color &color) const {
	return GetUint32Color(color.red, color.green, color.blue, color.alpha);
}

uint32 PixmanBitmap::GetUint32Color(uint8 _r, uint8 _g, uint8 _b, uint8 _a) const {
	uint32 a = _a;
	uint32 r = _r * a / 0xFF;
	uint32 g = _g * a / 0xFF;
	uint32 b = _b * a / 0xFF;
	return (a<<24) | (r<<16) | (g<<8) | (b<<0);
}

void PixmanBitmap::GetColorComponents(uint32 color, uint8 &r, uint8 &g, uint8 &b, uint8 &a) const {
	uint32 _a = (color >> 24) & 0xFF;
	uint32 _r = (color >> 16) & 0xFF;
	uint32 _g = (color >>  8) & 0xFF;
	uint32 _b = (color >>  0) & 0xFF;
	if (_a > 0) {
		a = (uint8) _a;
		r = (uint8) (0xFF * _r / _a);
		g = (uint8) (0xFF * _g / _a);
		b = (uint8) (0xFF * _b / _a);
	}
	else {
		a = 0;
		r = 0;
		g = 0;
		b = 0;
	}
}

////////////////////////////////////////////////////////////
void PixmanBitmap::Lock() {
}

void PixmanBitmap::Unlock() {
}

#endif

