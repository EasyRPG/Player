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
#ifdef USE_SDL_BITMAP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <cassert>
#include <cmath>
#include "filefinder.h"
#include "options.h"
#include "output.h"
#include "SDL_image.h"
#include "image_xyz.h"
//#include "SDL_rotozoom.h"
#include "utils.h"
#include "baseui.h"
#include "pixel_format.h"
#include "bitmap_utils.h"
#include "sdl_bitmap.h"

////////////////////////////////////////////////////////////

#ifdef USE_ALPHA
	#define TRANSPARENT_FLAGS SDL_SRCALPHA
	#define DisplayFormat(surface) SDL_DisplayFormatAlpha(surface)
#else
	#define TRANSPARENT_FLAGS SDL_SRCCOLORKEY
	#define DisplayFormat(surface) SDL_DisplayFormat(surface)
#endif

#ifdef USE_RLE
	#define COLORKEY_FLAGS SDL_SRCCOLORKEY | SDL_RLEACCEL
	#define SETALPHA_FLAGS SDL_SRCALPHA | SDL_RLEACCEL
#else
	#define COLORKEY_FLAGS SDL_SRCCOLORKEY
	#define SETALPHA_FLAGS SDL_SRCALPHA
#endif

////////////////////////////////////////////////////////////

SDL_Surface* SdlBitmap::ReadXYZ(const std::string& filename, bool transparent, const uint8 *data, uint len) {
	int w, h;
	void* pixels;
	if (!filename.empty()) {
		FILE* stream = FileFinder::fopenUTF8(filename, "rb");
		if (!stream)
			Output::Error("Couldn't open image file %s", filename.c_str());
		ImageXYZ::ReadXYZ(stream, transparent, w, h, pixels);
		fclose(stream);
	} else
		ImageXYZ::ReadXYZ(data, len, transparent, w, h, pixels);

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	SDL_Surface* src = SDL_CreateRGBSurfaceFrom(pixels, w, h, 32, w * 4, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);
#else
	SDL_Surface* src = SDL_CreateRGBSurfaceFrom(pixels, w, h, 32, w * 4, 0xFF000000, 0x00FF0000, 0x0000FF00, 0);
#endif
	SDL_Surface* dst = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, DisplayUi->GetDisplaySurface()->bytes() * 8, 0, 0, 0, 0);

	SDL_Rect rect = {0, 0, (uint16) w, (uint16) h};
	SDL_BlitSurface(src, &rect, dst, &rect);

	SDL_FreeSurface(src);
	free(pixels);

	return dst;
}

////////////////////////////////////////////////////////////
void SdlBitmap::SetupFormat(SDL_PixelFormat* fmt, bool transparent) {
#ifdef USE_ALPHA
	uint32 amask = (~0U >> (32 - fmt->BitsPerPixel)) ^ (fmt->Rmask | fmt->Gmask | fmt->Bmask);
	PF::AlphaType alpha = transparent ? (amask == 0 ? PF::ColorKey : PF::Alpha) : PF::NoAlpha;
#else
	uint32 amask = 0;
	PF::AlphaType alpha = transparent ? PF::ColorKey : PF::NoAlpha;
#endif
	format = DynamicFormat((int) fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask, fmt->Bmask, amask, alpha);
}

SdlBitmap::SdlBitmap(void *pixels, int width, int height, int pitch, const DynamicFormat& format) {
	bitmap = SDL_CreateRGBSurfaceFrom(pixels, width, height, format.bits, pitch,
									  format.r.mask, format.g.mask, format.b.mask, format.a.mask);

	if (bitmap == NULL) {
		Output::Error("Couldn't create %dx%d bitmap.\n%s\n", width, height, SDL_GetError());
	}

	SetupFormat(bitmap->format, false);
}

////////////////////////////////////////////////////////////
SdlBitmap::SdlBitmap(int width, int height, bool transparent, int bpp) {
	if ( !bpp ) {
		assert(DisplayUi);
		bpp = DisplayUi->GetDisplaySurface()->bytes() * 8;
	}

	uint32 flags = SDL_SWSURFACE;
	if (transparent)
		flags |= bpp == 32 ? TRANSPARENT_FLAGS : SDL_SRCCOLORKEY;

	SDL_Surface* temp = SDL_CreateRGBSurface(flags, width, height, bpp, 0, 0, 0, 0);

	if (temp == NULL) {
		Output::Error("Couldn't create %dx%d image.\n%s\n", width, height, SDL_GetError());
	}

	bitmap = DisplayFormat(temp);

	if (bitmap == NULL) {
		Output::Error("Couldn't optimize %dx%d image.\n%s\n", width, height, SDL_GetError());
	}

	SetupFormat(bitmap->format, transparent);

	SDL_FreeSurface(temp);
}

SdlBitmap::SdlBitmap(const std::string& filename, bool transparent, uint32 flags) {
	std::string ext = Utils::LowerCase(filename.substr(filename.size() - 3, 3));

	SDL_Surface* temp = (ext == "xyz") ? ReadXYZ(filename, transparent, NULL, 0) : IMG_Load(filename.c_str());

	if ( !temp ) {
		Output::Error("Couldn't load %s image.\n%s\n", filename.c_str(), IMG_GetError());
	}

	if (temp->format->BitsPerPixel == 8 && transparent) {
		SDL_Color colorkey = temp->format->palette->colors[0];
		RemovePaletteColorkeyDuplicates(temp, &colorkey);
		SDL_SetColorKey(temp, COLORKEY_FLAGS, SDL_MapRGB(temp->format, colorkey.r, colorkey.g, colorkey.b));
	}

	bitmap = DisplayFormat(temp);
	if (bitmap == NULL) {
		Output::Error("Couldn't optimize %s image.\n%s\n", filename.c_str(), SDL_GetError());
	}
	SDL_FreeSurface(temp);

	SetupFormat(bitmap->format, transparent);

	CheckPixels(flags);
}

SdlBitmap::SdlBitmap(const uint8* data, uint bytes, bool transparent, uint32 flags) {
	SDL_Surface* temp;
	if (bytes > 4 && strncmp((char*) data, "XYZ1", 4) == 0)
		temp = ReadXYZ(NULL, transparent, data, bytes);
	else {
		SDL_RWops* rw_ops = SDL_RWFromConstMem(data, bytes);
		temp = IMG_Load_RW(rw_ops, 1);
	}

	if (temp == NULL) {
		Output::Error("Couldn't load image from memory.\n%s\n", IMG_GetError());
	}

	if (temp->format->BitsPerPixel == 8 && transparent) {
		SDL_Color colorkey = temp->format->palette->colors[0];
		RemovePaletteColorkeyDuplicates(temp, &colorkey);
		SDL_SetColorKey(temp, COLORKEY_FLAGS, SDL_MapRGB(temp->format, colorkey.r, colorkey.g, colorkey.b));
	}

	bitmap = DisplayFormat(temp);

	if (bitmap == NULL) {
		Output::Error("Couldn't optimize memory image.\n%s\n", SDL_GetError());
	}

	SDL_FreeSurface(temp);

	SetupFormat(bitmap->format, transparent);

	CheckPixels(flags);
}

SdlBitmap::SdlBitmap(Bitmap* source, Rect src_rect, bool transparent) {
	uint32 flags = SDL_SWSURFACE;
	if (transparent)
		flags |= DisplayUi->GetDisplaySurface()->bytes() == 4 ? TRANSPARENT_FLAGS : SDL_SRCCOLORKEY;

	SDL_Surface* temp = SDL_CreateRGBSurface(flags, src_rect.width, src_rect.height, DisplayUi->GetDisplaySurface()->bytes() * 8, 0, 0, 0, 0);

	if (temp == NULL) {
		Output::Error("Couldn't create %dx%d image.\n%s\n", src_rect.width, src_rect.height, SDL_GetError());
	}

	bitmap = DisplayFormat(temp);

	if (bitmap == NULL) {
		Output::Error("Couldn't optimize %dx%d image.\n%s\n", src_rect.width, src_rect.height, SDL_GetError());
	}

	SDL_FreeSurface(temp);

	SetupFormat(bitmap->format, transparent);

	if (format.alpha_type == PF::Alpha)
		Clear();
	else if ((((SdlBitmap*)source)->bitmap->flags & SDL_SRCCOLORKEY) == SDL_SRCCOLORKEY && transparent) {
		SDL_FillRect(bitmap, NULL, ((SdlBitmap*)source)->bitmap->format->colorkey);
		SDL_SetColorKey(bitmap, COLORKEY_FLAGS, ((SdlBitmap*)source)->bitmap->format->colorkey);
	}

	Blit(0, 0, source, src_rect, 255);
}

SdlBitmap::SdlBitmap(SDL_Surface* bitmap, bool transparent) :
	bitmap(bitmap) {
	SetupFormat(bitmap->format, transparent);
}

////////////////////////////////////////////////////////////
SdlBitmap::~SdlBitmap() {
	SDL_FreeSurface(bitmap);
}

////////////////////////////////////////////////////////////
void* SdlBitmap::pixels() {
	// WARNING!!!! 
	// If bitmap is not locked before calling this
	// this will return an invalid pointer!!
	assert(!SDL_MUSTLOCK(bitmap) || bitmap->locked);
	return bitmap->pixels;
}

uint8 SdlBitmap::bytes() const {
	return bitmap->format->BytesPerPixel;
}

int SdlBitmap::width() const {
	return bitmap->w;
}

int SdlBitmap::height() const {
	return bitmap->h;
}

uint16 SdlBitmap::pitch() const {
	return bitmap->pitch;
}

uint32 SdlBitmap::rmask() const {
	return bitmap->format->Rmask;
}

uint32 SdlBitmap::gmask() const {
	return bitmap->format->Gmask;
}

uint32 SdlBitmap::bmask() const {
	return bitmap->format->Bmask;
}

uint32 SdlBitmap::amask() const {
	return bitmap->format->Amask;
}

uint32 SdlBitmap::colorkey() const {
	return bitmap->format->colorkey;
}

/*void SdlBitmap::SetupBitmapData() {
	pixels = bitmap->pixels;
	width = bitmap->w;
	height = bitmap->h;
	bpp = bitmap->format->BytesPerPixel;
	pitch = bitmap->pitch;
	rmask = bitmap->format->Rmask;
	gmask = bitmap->format->Gmask;
	bmask = bitmap->format->Bmask;
	amask = bitmap->format->Amask;
	colorkey = bitmap->format->colorkey;
}*/

////////////////////////////////////////////////////////////
void SdlBitmap::RemovePaletteColorkeyDuplicates(SDL_Surface* src, SDL_Color* color) {
	int n_colors = src->format->palette->ncolors;
	SDL_Color* colors = src->format->palette->colors;

	bool color_was_found = false;

	for (int i = 0; i < n_colors; i++) {
		if (color_was_found) {
			if (colors[i].r == color->r && colors[i].g == color->g && colors[i].b == color->b) {
				(colors[i].b != 255) ? colors[i].b++ : colors[i].b--;
			}
		} else {
			if (colors[i].r == color->r && colors[i].g == color->g && colors[i].b == color->b) {
				color_was_found = true;
			}
		}
	}
}

////////////////////////////////////////////////////////////
void SdlBitmap::Blit(int x, int y, Bitmap* src, Rect src_rect, int opacity) {
	if (src->format.alpha_type == PF::Alpha)
		Surface::Blit(x, y, src, src_rect, opacity);
	else {
		SDL_Rect src_r = {(int16)src_rect.x, (int16)src_rect.y, (uint16)src_rect.width, (uint16)src_rect.height};
		SDL_Rect dst_r = {(int16)x, (int16)y, 0, 0};

		if (opacity < 255) SDL_SetAlpha(((SdlBitmap*)src)->bitmap, SETALPHA_FLAGS, (uint8)opacity);

		SDL_BlitSurface(((SdlBitmap*)src)->bitmap, &src_r, bitmap, &dst_r);
		
		if (opacity < 255) SDL_SetAlpha(((SdlBitmap*)src)->bitmap, SETALPHA_FLAGS, 255);
	}

	RefreshCallback();
}

////////////////////////////////////////////////////////////
void SdlBitmap::StretchBlit(Rect dst_rect, Bitmap* src, Rect src_rect, int opacity) {
	if (src_rect.width == dst_rect.width && src_rect.height == dst_rect.height) {
		Blit(dst_rect.x, dst_rect.y, src, src_rect, opacity);
	} else {
		Surface::StretchBlit(dst_rect, src, src_rect, opacity);
	}
}

////////////////////////////////////////////////////////////
void SdlBitmap::Fill(const Color &color) {
	SDL_FillRect(bitmap, NULL, SDL_MapRGBA(bitmap->format, color.red, color.green, color.blue, color.alpha));

	RefreshCallback();
}

////////////////////////////////////////////////////////////
void SdlBitmap::FillRect(Rect dst_rect, const Color &color) {
	SDL_Rect sdl_dst_rect = {(int16)dst_rect.x, (int16)dst_rect.y, (uint16)dst_rect.width, (uint16)dst_rect.height};

	SDL_FillRect(bitmap, &sdl_dst_rect, SDL_MapRGBA(bitmap->format, color.red, color.green, color.blue, color.alpha));

	RefreshCallback();
}

////////////////////////////////////////////////////////////
void SdlBitmap::Mask(int x, int y, Bitmap* src, Rect src_rect) {
	if (format.alpha_type == PF::Alpha)
		Surface::MaskBlit(x, y, src, src_rect);
	else {
		src->SetTransparentColor(Color(255,255,255,0));
		Blit(x, y, src, src_rect, 255);
		SetTransparentColor(Color(0,0,0,0));
	}

	RefreshCallback();
}

////////////////////////////////////////////////////////////
void SdlBitmap::SetTransparentColor(Color color) {
	if (format.alpha_type == PF::ColorKey) {
		uint32 colorkey = GetUint32Color(color);
		SDL_SetColorKey(bitmap, COLORKEY_FLAGS, colorkey);
	}
}

////////////////////////////////////////////////////////////
Color SdlBitmap::GetColor(uint32 uint32_color) const {
	uint8 r, g, b, a;
	SDL_GetRGBA(uint32_color, bitmap->format, &r, &g, &b, &a);
	return Color(r, g, b, a);
}

uint32 SdlBitmap::GetUint32Color(const Color &color) const {
	return SDL_MapRGBA(bitmap->format, color.red, color.green, color.blue, color.alpha);
}

uint32 SdlBitmap::GetUint32Color(uint8 r, uint8  g, uint8 b, uint8 a) const {
	return SDL_MapRGBA(bitmap->format, r, g, b, a);
}

void SdlBitmap::GetColorComponents(uint32 color, uint8 &r, uint8 &g, uint8 &b, uint8 &a) const {
	SDL_GetRGBA(color, bitmap->format, &r, &g, &b, &a);
}

////////////////////////////////////////////////////////////
void SdlBitmap::Lock() {
	if (SDL_MUSTLOCK(bitmap)) {
		if (SDL_LockSurface(bitmap) < 0) {
			Output::Error("%s", SDL_GetError());
		}
	}
}

void SdlBitmap::Unlock() {
	if (SDL_MUSTLOCK(bitmap)) {
		SDL_UnlockSurface(bitmap);
	}
}

////////////////////////////////////////////////////////////
#endif

