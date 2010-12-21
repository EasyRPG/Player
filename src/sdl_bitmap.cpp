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
#include "sdl_bitmap.h"
#include "filefinder.h"
#include "graphics.h"
#include "hslrgb.h"
#include "main_data.h"
#include "options.h"
#include "output.h"
#include "player.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_rotozoom.h"
#include "exfont.xpm"
#include "util_macro.h"
#include "system.h"
#include <cmath>
#include "sdl_ui.h"

////////////////////////////////////////////////////////////
#ifdef USE_ALPHA
	#define TRANSPARENT_FLAGS SDL_SRCALPHA
	#define COLORKEY_FLAGS SDL_SRCCOLORKEY
	#define DisplayFormat(surface) SDL_DisplayFormatAlpha(surface)
RMASK, GMASK, BMASK, AMASK
#else
	#define TRANSPARENT_FLAGS SDL_SRCALPHA | SDL_SRCCOLORKEY
	#define COLORKEY_FLAGS SDL_SRCCOLORKEY | SDL_RLEACCEL
	#define SETALPHA_FLAGS SDL_SRCALPHA | SDL_RLEACCEL
	#define DisplayFormat(surface) SDL_DisplayFormat(surface)
#endif


#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	const unsigned int RMASK = 0x0000FF00;
	const unsigned int GMASK = 0x00FF0000;
	const unsigned int BMASK = 0xFF000000;
	#ifdef USE_ALPHA
		const unsigned int AMASK = 0x000000FF;
	#else
		const unsigned int AMASK = 0x00000000;
	#endif
#else
	const unsigned int RMASK = 0x00FF0000;
	const unsigned int GMASK = 0x0000FF00;
	const unsigned int BMASK = 0x000000FF;
	#ifdef USE_ALPHA
		const unsigned int AMASK = 0xFF000000;
	#else
		const unsigned int AMASK = 0x00000000;
	#endif
#endif

////////////////////////////////////////////////////////////
SdlBitmap::SdlBitmap(int width, int height, bool itransparent) {
	transparent = itransparent;

	uint32 flags = SDL_SWSURFACE;
	if (transparent)
		flags |= TRANSPARENT_FLAGS;

	SDL_Surface* temp = SDL_CreateRGBSurface(flags, width, height, ((SdlUi*)DisplayUi)->GetDisplaySurface()->format->BitsPerPixel, RMASK, GMASK, BMASK, AMASK);

	if (temp == NULL) {
		Output::Error("Couldn't create %dx%d image.\n%s\n", width, height, SDL_GetError());
	}

	bitmap = DisplayFormat(temp);

	if (bitmap == NULL) {
		Output::Error("Couldn't optimize %dx%d image.\n%s\n", width, height, SDL_GetError());
	}

	SDL_FreeSurface(temp);

	SetupBitmapData();
}

SdlBitmap::SdlBitmap(const std::string filename, bool itransparent) {
	transparent = itransparent;

	std::string path = FileFinder::FindImage(filename);

	if (path.empty()) {
		Output::Error("No such file or directory - %s", filename.c_str());
	}

	SDL_Surface* temp = IMG_Load(path.c_str());

	if (temp == NULL) {
		Output::Error("Couldn't load %s image.\n%s\n", filename.c_str(), IMG_GetError());
	}

	if (temp->format->BitsPerPixel == 8 && transparent) {
		SDL_Color colorkey = temp->format->palette->colors[0];
		RemovePalleteColorkeyDuplicates(temp, &colorkey);
		SDL_SetColorKey(temp, COLORKEY_FLAGS, SDL_MapRGB(temp->format, colorkey.r, colorkey.g, colorkey.b));
	}

	bitmap = DisplayFormat(temp);

	if (bitmap == NULL) {
		Output::Error("Couldn't optimize %s image.\n%s\n", filename.c_str(), SDL_GetError());
	}

	SDL_FreeSurface(temp);

	SetupBitmapData();
}

SdlBitmap::SdlBitmap(const uint8* data, uint bytes, bool itransparent) {
	transparent = itransparent;

	SDL_RWops* rw_ops = SDL_RWFromConstMem(data, bytes);
	SDL_Surface* temp = IMG_Load_RW(rw_ops, 1);

	if (temp == NULL) {
		Output::Error("Couldn't load image from memory.\n%s\n", IMG_GetError());
	}

	if (temp->format->BitsPerPixel == 8 && transparent) {
		SDL_Color colorkey = temp->format->palette->colors[0];
		RemovePalleteColorkeyDuplicates(temp, &colorkey);
		SDL_SetColorKey(temp, COLORKEY_FLAGS, SDL_MapRGB(temp->format, colorkey.r, colorkey.g, colorkey.b));
	}

	bitmap = DisplayFormat(temp);

	if (bitmap == NULL) {
		Output::Error("Couldn't optimize memory image.\n%s\n", SDL_GetError());
	}

	SDL_FreeSurface(temp);

	SetupBitmapData();
}

SdlBitmap::SdlBitmap(Bitmap* source, Rect src_rect, bool itransparent) {
	transparent = itransparent;

	uint32 flags = SDL_SWSURFACE;
	if (transparent)
		flags |= TRANSPARENT_FLAGS;

	SDL_Surface* temp = SDL_CreateRGBSurface(flags, src_rect.width, src_rect.height, ((SdlUi*)DisplayUi)->GetDisplaySurface()->format->BitsPerPixel, RMASK, GMASK, BMASK, AMASK);

	if (temp == NULL) {
		Output::Error("Couldn't create %dx%d image.\n%s\n", src_rect.width, src_rect.height, SDL_GetError());
	}

	bitmap = DisplayFormat(temp);

	if (bitmap == NULL) {
		Output::Error("Couldn't optimize %dx%d image.\n%s\n", src_rect.width, src_rect.height, SDL_GetError());
	}

	SDL_FreeSurface(temp);

	if ((((SdlBitmap*)source)->bitmap->flags & SDL_SRCCOLORKEY) == SDL_SRCCOLORKEY && transparent) {
		SDL_FillRect(bitmap, NULL, ((SdlBitmap*)source)->bitmap->format->colorkey);
		SDL_SetColorKey(bitmap, COLORKEY_FLAGS, ((SdlBitmap*)source)->bitmap->format->colorkey);
	}

	Blit(0, 0, source, src_rect, 255);

	SetupBitmapData();
}

SdlBitmap::SdlBitmap(SDL_Surface* bitmap, bool itransparent) :
	bitmap(bitmap) {

	transparent = itransparent;

	SetupBitmapData();
}

////////////////////////////////////////////////////////////
SdlBitmap::~SdlBitmap() {
	SDL_FreeSurface(bitmap);
}

////////////////////////////////////////////////////////////
void SdlBitmap::SetupBitmapData() {
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
}

////////////////////////////////////////////////////////////
void SdlBitmap::RemovePalleteColorkeyDuplicates(SDL_Surface* src, SDL_Color* color) {
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
	#ifdef USE_ALPHA
		Bitmap::Blit(x, y, src, src_rect, opacity);
	#else
		SDL_Rect src_r = {(int16)src_rect.x, (int16)src_rect.y, (uint16)src_rect.width, (uint16)src_rect.height};
		SDL_Rect dst_r = {(int16)x, (int16)y, 0, 0};

		SDL_SetAlpha(((SdlBitmap*)src)->bitmap, SETALPHA_FLAGS, (uint8)opacity);

		SDL_BlitSurface(((SdlBitmap*)src)->bitmap, &src_r, bitmap, &dst_r);

		SDL_SetAlpha(((SdlBitmap*)src)->bitmap, SETALPHA_FLAGS, 255);

		RefreshCallback();
	#endif
}

////////////////////////////////////////////////////////////
void SdlBitmap::StretchBlit(Rect dst_rect, Bitmap* src, Rect src_rect, int opacity) {
	if (src_rect.width == dst_rect.width && src_rect.height == dst_rect.height) {
		Blit(dst_rect.x, dst_rect.y, src, src_rect, opacity);
	} else {
		Bitmap::StretchBlit(dst_rect, src, src_rect, opacity);
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
void SdlBitmap::TextDraw(Rect dst_rect, std::string text, TextAlignment align) {

}

////////////////////////////////////////////////////////////
Rect SdlBitmap::GetTextSize(std::string text) const {
	return Rect();
}

////////////////////////////////////////////////////////////
void SdlBitmap::SetTransparentColor(Color color) {
	#ifndef USE_ALPHA
		SDL_SetColorKey(bitmap, COLORKEY_FLAGS, GetUint32Color(color));

		colorkey = GetUint32Color(color);
	#endif
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
	if (SDL_MUSTLOCK(bitmap)) SDL_LockSurface(bitmap);
}

void SdlBitmap::Unlock() {
	if (SDL_MUSTLOCK(bitmap)) SDL_UnlockSurface(bitmap);
}
