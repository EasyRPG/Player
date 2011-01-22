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
#include "image.h"
#include "ftfont.h"
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

////////////////////////////////////////////////////////////
SoftBitmap::SoftBitmap(int width, int height, bool itransparent) {
	transparent = itransparent;

	Init(width, height);
}

SoftBitmap::SoftBitmap(const std::string filename, bool itransparent) {
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
		Image::ReadPNG(stream, (const void*) NULL, transparent, w, h, bitmap);
	else if (ext == "xyz")
		Image::ReadXYZ(stream, transparent, w, h, bitmap);

	fclose(stream);
}

SoftBitmap::SoftBitmap(const uint8* data, uint bytes, bool itransparent) {
	transparent = itransparent;

	if (bytes > 4 && strncmp((char*) data, "XYZ1", 4) == 0)
		Image::ReadXYZ(data, bytes, transparent, w, h, bitmap);
	else
		Image::ReadPNG((FILE*) NULL, (const void*) data, transparent, w, h, bitmap);
}

SoftBitmap::SoftBitmap(Bitmap* source, Rect src_rect, bool itransparent) {
	transparent = itransparent;

	Init(src_rect.width, src_rect.height);

	Blit(0, 0, source, src_rect, 255);
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
	Rect dst_rect(x, y, 0, 0);

	if (!Rect::AdjustRectangles(src_rect, dst_rect, src->GetRect()))
		return;
	if (!Rect::AdjustRectangles(dst_rect, src_rect, GetRect()))
		return;

	if (opacity > 255) opacity = 255;

	Lock();
	src->Lock();

	const uint8* src_pixels = (const uint8*)src->pixels() + src_rect.x * bpp() + src_rect.y * src->pitch();
	uint8* dst_pixels = (uint8*)pixels() + dst_rect.x * bpp() + dst_rect.y * pitch();

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

	Rect dst_rect(x, y, 0, 0);

	if (!Rect::AdjustRectangles(src_rect, dst_rect, src->GetRect()))
		return;
	if (!Rect::AdjustRectangles(dst_rect, src_rect, GetRect()))
		return;

	Lock();
	src->Lock();

	const uint8* src_pixels = (uint8*)src->pixels() + src_rect.x * bpp() + src_rect.y * src->pitch();
	uint8* dst_pixels = (uint8*)pixels() + dst_rect.x * bpp() + dst_rect.y * pitch();

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

	FreeType::Init(font);

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

			mask = (SoftBitmap*) FreeType::RenderChar(font, text[c]);
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

	FreeType::Done();

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

#endif

