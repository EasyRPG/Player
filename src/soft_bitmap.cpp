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
#include <cstdlib>
#include <iostream>
#include <png.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H
#include "SDL.h"

#include "cache.h"
#include "filefinder.h"
#include "options.h"
#include "data.h"
#include "output.h"
#include "system.h"
#include "soft_bitmap.h"

////////////////////////////////////////////////////////////
void SoftBitmap::Init(int width, int height) {
	w = width;
	h = height;
	bitmap = calloc(h, w * 4);

	if (bitmap == NULL) {
		Output::Error("Couldn't create %dx%d image.\n", w, h);
	}
}

void SoftBitmap::copy_from_sdl(void* bitmap, SDL_Surface* src) {

	SDL_Surface* tmp = SDL_CreateRGBSurface(
		SDL_SWSURFACE|SDL_SRCALPHA, src->w, src->h,
		32, RMASK, GMASK, BMASK, AMASK);

	if (tmp == NULL) {
		Output::Error("Couldn't create surface\n%s\n", SDL_GetError());
	}

	SDL_BlitSurface(src, NULL, tmp, NULL);

	if (SDL_MUSTLOCK(tmp)) {
		if (SDL_LockSurface(tmp) < 0) {
			Output::Error("%s", SDL_GetError());
		}
	}

	uint32* q = (uint32*) bitmap;
	for (int y = 0; y < tmp->h; y++) {
		const uint32 *p = (const uint32*) ((const char*) tmp->pixels + y * tmp->pitch);
		for (int x = 0; x < tmp->w; x++)
			*q++ = *p++;
	}

	if (SDL_MUSTLOCK(tmp)) {
		SDL_UnlockSurface(tmp);
	}

	SDL_FreeSurface(tmp);
}

SDL_Surface* SoftBitmap::copy_to_sdl(Bitmap* source) {
	SoftBitmap* src = (SoftBitmap*) source;

	SDL_Surface* temp = SDL_CreateRGBSurfaceFrom(
		src->pixels(), src->width(), src->height(), 32, src->width() * 4,
		RMASK, GMASK, BMASK, AMASK);

	return temp;
}

////////////////////////////////////////////////////////////
static void read_data(png_structp png_ptr, png_bytep data, png_size_t length) {
    png_bytep* bufp = (png_bytep*) png_get_io_ptr(png_ptr);
	memcpy(data, *bufp, length);
	*bufp += length;
}

////////////////////////////////////////////////////////////
void SoftBitmap::ReadPNG(FILE *stream, const void *buffer) {
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

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
		Output::Error("Error reading PNG file");
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
	}

	if (transparent && num_palette > 0) {
		png_color& ck = palette[0];
		uint8 ck1[4] = {255, ck.red, ck.green, ck.blue};
		uint8 ck2[4] = {  0, ck.red, ck.green, ck.blue};
		uint32 srckey = *(uint32*)ck1;
		uint32 dstkey = *(uint32*)ck2;
		uint32* p = (uint32*) bitmap;
		for (int i = 0; i < w * h; i++, p++)
			if (*p == srckey)
				*p = dstkey;
	}

	png_read_end(png_ptr, NULL);

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
}

////////////////////////////////////////////////////////////
SoftBitmap::SoftBitmap(int width, int height, bool itransparent) {
	transparent = itransparent;

	Init(width, height);
}

SoftBitmap::SoftBitmap(const std::string filename, bool itransparent) {
	transparent = itransparent;

	FILE* stream = fopen(filename.c_str(), "rb");
	if (!stream) {
		Output::Error("Couldn't open input file %s", filename.c_str());
		return;
	}
	ReadPNG(stream, (void*) NULL);
	fclose(stream);
}

SoftBitmap::SoftBitmap(const uint8* data, uint bytes, bool itransparent) {
	transparent = itransparent;

	ReadPNG((FILE*) NULL, (const void*) data);
}

SoftBitmap::SoftBitmap(Bitmap* source, Rect src_rect, bool itransparent) {
	transparent = itransparent;

	Init(src_rect.width, src_rect.height);

	Blit(0, 0, source, src_rect, 255);
}

SoftBitmap::SoftBitmap(SDL_Surface* source, bool itransparent) {
	transparent = itransparent;

	Init(source->w, source->h);

	copy_from_sdl(bitmap, source);
}

////////////////////////////////////////////////////////////
SoftBitmap::~SoftBitmap() {
	free(bitmap);
}

////////////////////////////////////////////////////////////
void* SoftBitmap::pixels() {
	return bitmap;
}

uint8 SoftBitmap::bpp() const {
	return 4;
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
	return RMASK;
}

uint32 SoftBitmap::gmask() const {
	return GMASK;
}

uint32 SoftBitmap::bmask() const {
	return BMASK;
}

uint32 SoftBitmap::amask() const {
	return AMASK;
}

uint32 SoftBitmap::colorkey() const {
	return 0;
}

////////////////////////////////////////////////////////////
void SoftBitmap::Blit(int x, int y, Bitmap* _src, Rect src_rect, int opacity) {
	if (opacity < 0)
		return;

	SoftBitmap* src = (SoftBitmap*) _src;

	src_rect.Adjust(src->width(), src->height());
	if (src_rect.IsOutOfBounds(src->width(), src->height())) return;

	Rect dst_rect(x, y, src_rect.width, src_rect.height);
	dst_rect.Adjust(width(), height());
	if (dst_rect.IsOutOfBounds(width(), height())) return;

	if (opacity > 255) opacity = 255;

	Lock();
	src->Lock();

	const uint8* src_pixels = (const uint8*)src->pixels() + src_rect.x * bpp() + src_rect.y * src->pitch();
	uint8* dst_pixels = (uint8*)pixels() + x * bpp() + y * pitch();

	int src_pad = src->pitch() - dst_rect.width * bpp();
	int dst_pad = pitch() - dst_rect.width * bpp();

	for (int i = 0; i < dst_rect.height; i++) {
		for (int j = 0; j < dst_rect.width; j++) {
			uint8 srca = src_pixels[0] * opacity / 255;
			dst_pixels[1] = (dst_pixels[1] * (255 - srca) + src_pixels[1] * srca) / 255;
			dst_pixels[2] = (dst_pixels[2] * (255 - srca) + src_pixels[2] * srca) / 255;
			dst_pixels[3] = (dst_pixels[3] * (255 - srca) + src_pixels[3] * srca) / 255;
			dst_pixels[0] = std::max(dst_pixels[0], srca);

			src_pixels += 4;
			dst_pixels += 4;
		}
		src_pixels += src_pad;
		dst_pixels += dst_pad;
	}

	Unlock();
	src->Unlock();

	RefreshCallback();
}

void SoftBitmap::Mask(int x, int y, Bitmap* _src, Rect src_rect) {
	SoftBitmap* src = (SoftBitmap*) _src;

	src_rect.Adjust(src->width(), src->height());
	if (src_rect.IsOutOfBounds(src->width(), src->height())) return;

	Rect dst_rect(x, y, src_rect.width, src_rect.height);
	dst_rect.Adjust(width(), height());
	if (dst_rect.IsOutOfBounds(width(), height())) return;

	Lock();
	src->Lock();

	const uint8* src_pixels = (uint8*)src->pixels() + src_rect.x * bpp() + src_rect.y * src->pitch();
	uint8* dst_pixels = (uint8*)pixels() + x * bpp() + y * pitch();

	int src_pad = src->pitch() - dst_rect.width * bpp();
	int dst_pad = pitch() - dst_rect.width * bpp();

	for (int i = 0; i < dst_rect.height; i++) {
		for (int j = 0; j < dst_rect.width; j++) {
			dst_pixels[0] = std::min(dst_pixels[0], src_pixels[0]);

			src_pixels += 4;
			dst_pixels += 4;
		}
		src_pixels += src_pad;
		dst_pixels += dst_pad;
	}

	Unlock();
	src->Unlock();

	RefreshCallback();
}

FT_Library SoftBitmap::library;
FT_Face SoftBitmap::face;
bool SoftBitmap::ft_initialized = false;

void SoftBitmap::InitFreeType() {
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

void SoftBitmap::DoneFreeType() {
	if (!ft_initialized)
		return;

    FT_Done_Face(face);

    FT_Done_FreeType(library);

	ft_initialized = false;
}

SoftBitmap* SoftBitmap::RenderFreeTypeChar(int c) {
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

	SoftBitmap* bitmap = new SoftBitmap(ft_bitmap.width, font->size + 2, true);
	uint8* dst = (uint8*) bitmap->pixels();

	const int base_line = bitmap->height() / 4;
	int offset = bitmap->height() - face->glyph->bitmap_top - base_line;

	for (int yd = 0; yd < bitmap->height(); yd++) {
		int ys = yd - offset;
		if (ys < 0 || ys >= ft_bitmap.rows)
			continue;
		const uint8* p = src + ys * ft_bitmap.pitch;
		uint8* q = dst + yd * bitmap->pitch();
		for (int x = 0; x < ft_bitmap.width; x++) {
			*q++ = *p++ ? 0xFF : 0x00;
			// *dst++ = *p++ * 256 / ft_bitmap.num_grays;
			*q++ = 0;
			*q++ = 0;
			*q++ = 0;
		}
	}

	return bitmap;
}

////////////////////////////////////////////////////////////
void SoftBitmap::TextDraw(int x, int y, std::string text, TextAlignment align) {
	if (text.length() == 0) return;
	Rect dst_rect = GetTextSize(text);
	dst_rect.x = x; dst_rect.y = y;
	dst_rect.width += 1; dst_rect.height += 1; // Need place for shadow
	if (dst_rect.IsOutOfBounds(GetWidth(), GetHeight())) return;

	SoftBitmap* text_surface; // Complete text will be on this surface

	text_surface = new SoftBitmap(dst_rect.width, dst_rect.height, true);

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

		SoftBitmap* mask;

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

			mask = new SoftBitmap(12, 12, true);

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

		SoftBitmap* char_surface = new SoftBitmap(mask->width(), mask->height(), true);

		// Blit gradient color background (twice in case of a full glyph)
		char_surface->Blit(0, 0, system, clip_system, 255);
		char_surface->Blit(6, 0, system, clip_system, 255);
		// Blit mask onto background
		char_surface->Mask(0, 0, mask, mask->GetRect());

		SoftBitmap* char_shadow = new SoftBitmap(mask->width(), mask->height(), true);

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
void SoftBitmap::SetTransparentColor(Color color) {
}

////////////////////////////////////////////////////////////
Color SoftBitmap::GetColor(uint32 uint32_color) const {
	const uint8* p = (const uint8*) &uint32_color;
	return Color(p[1], p[2], p[3], p[0]);
}

uint32 SoftBitmap::GetUint32Color(const Color &color) const {
	uint8 x[4] = {color.alpha, color.red, color.green, color.blue};
	return *(const uint32*) x;
}

uint32 SoftBitmap::GetUint32Color(uint8 r, uint8  g, uint8 b, uint8 a) const {
	uint8 x[4] = {a, r, g, b};
	return *(const uint32*) x;
}

void SoftBitmap::GetColorComponents(uint32 color, uint8 &r, uint8 &g, uint8 &b, uint8 &a) const {
	const uint8* p = (const uint8*) &color;
	a = p[0];
	r = p[1];
	g = p[2];
	b = p[3];
}

////////////////////////////////////////////////////////////
void SoftBitmap::Lock() {
}

void SoftBitmap::Unlock() {
}
