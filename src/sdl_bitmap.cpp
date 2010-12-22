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
#include <cassert>
#include <cmath>
#include "sdl_bitmap.h"
#include "cache.h"
#include "filefinder.h"
#include "graphics.h"
#include "hslrgb.h"
#include "main_data.h"
#include "options.h"
#include "output.h"
#include "player.h"
#include "SDL_image.h"
#include "SDL_rotozoom.h"
#include "SDL_ttf.h"
#include "sdl_ui.h"
#include "system.h"
#include "util_macro.h"
#include "exfont.xpm"

////////////////////////////////////////////////////////////
#ifdef USE_ALPHA
	#define TRANSPARENT_FLAGS SDL_SRCALPHA
	#define COLORKEY_FLAGS SDL_SRCCOLORKEY
	#define DisplayFormat(surface) SDL_DisplayFormatAlpha(surface)
RMASK, GMASK, BMASK, AMASK
#else
	#define TRANSPARENT_FLAGS /*SDL_SRCALPHA | */SDL_SRCCOLORKEY
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
}

SdlBitmap::SdlBitmap(SDL_Surface* bitmap, bool itransparent) :
	bitmap(bitmap) {

	transparent = itransparent;
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

uint8 SdlBitmap::bpp() const {
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

		//SDL_SetAlpha(((SdlBitmap*)src)->bitmap, SETALPHA_FLAGS, (uint8)opacity);

		SDL_BlitSurface(((SdlBitmap*)src)->bitmap, &src_r, bitmap, &dst_r);

		//SDL_SetAlpha(((SdlBitmap*)src)->bitmap, SETALPHA_FLAGS, 255);

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
	// ToDo: This code is ugly! Refactor this code!
	if (text.length() == 0) return;
	if (dst_rect.IsOutOfBounds(GetWidth(), GetHeight())) return;

#ifdef USE_ALPHA
	const int rbyte = MaskGetByte(bitmap->format->Rmask);
	const int gbyte = MaskGetByte(bitmap->format->Gmask);
	const int bbyte = MaskGetByte(bitmap->format->Bmask);
	const int abyte = MaskGetByte(bitmap->format->Amask);
#endif
	
	TTF_Font* ttf_font = font->GetTTF();
	int style = 0;
	if (font->bold) style |= TTF_STYLE_BOLD;
	if (font->italic) style |= TTF_STYLE_ITALIC;
	TTF_SetFontStyle(ttf_font, style);

	SDL_Surface* text_surface; // Complete text
#ifndef USE_ALPHA
	SDL_Surface* text_surface_aux;
	SDL_Surface* mask;
#endif
	SDL_Surface* char_surface; // Single char
	SDL_Surface* char_shadow; // Drop shadow of char

	// Create a new RGB Surface in an endian-neutral way
#ifdef USE_ALPHA
	text_surface = SDL_CreateRGBSurface(bitmap->flags, text.size()*6, 12, 32, (0xFF << rbyte*8), (0xFF << gbyte*8), (0xFF << bbyte*8), (0xFF << abyte*8));
#else
	text_surface = SDL_CreateRGBSurface(bitmap->flags | SDL_SRCCOLORKEY, text.size()*6, TTF_FontHeight(ttf_font), 32, 0, 0, 0, 0);

	text_surface_aux = SDL_CreateRGBSurface(bitmap->flags | SDL_SRCCOLORKEY, text.size()*6, TTF_FontHeight(ttf_font), 32, 0, 0, 0, 0);
	SDL_Color color = { 255, 255, 255, 0 };
#endif

#if FONT_SMOOTHING == 0 && defined(USE_ALPHA)
	Uint32 colorkey = 0;
#endif

	char text2[2]; text2[1] = '\0';

	// Load the system file for the shadow and text color
	Bitmap* system = Cache::System(Data::system.system_name);
	// Load the exfont-file
#if !defined(DINGOO)
	SDL_Surface* exfont = IMG_ReadXPMFromArray(const_cast<char**>(exfont_xpm));
#else
	SDL_RWops* rw_ops = SDL_RWFromConstMem((char*)exfont_h, sizeof(exfont_h));
	SDL_Surface* exfont = IMG_Load_RW(rw_ops, 1);
#endif

	// Get the Shadow color
	Color shadowColor(system->GetPixel(16, 32));

	// Where to draw the next glyph (x pos)
	int next_glyph_pos = 0;

	// The current char is an exfont/full size glyph
	bool is_full_glyph = false;

	// This loops always renders a single char, color blends it and then puts
	// it onto the text_surface
	for (unsigned c = 0; c < text.size(); ++c) {
		text2[0] = text[c];

		// EXFONT-Detection: Check for A-Z or a-z behind the $
		if (text[c] == '$' && c != text.size() - 1 &&
			((text[c+1] >= 'a' && text[c+1] <= 'z') || (text[c+1] >= 'A' && text[c+1] <= 'Z'))) {
			int exfont_value;
			// The two Bottom rows
			if ((text[c+1] >= 'a' && text[c+1] <= 'z')) {
				exfont_value = 26 + text[c+1] - 'a';
			} else {
				exfont_value = text[c+1] - 'A';
			}
			is_full_glyph = true;

#if FONT_SMOOTHING == 0
	#ifdef USE_ALPHA
			char_surface = SDL_CreateRGBSurface(exfont->flags, 12, 12, 8, (0xFF << rbyte*8), (0xFF << gbyte*8), (0xFF << bbyte*8), (0xFF << abyte*8));
	#else
			char_surface = SDL_CreateRGBSurface(exfont->flags | SDL_SRCCOLORKEY, 12, 12, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0);
			char_shadow = SDL_CreateRGBSurface(exfont->flags | SDL_SRCCOLORKEY, 12, 12, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0);
			// Create a completely black surface. This will be the base color of mask.
			mask = SDL_CreateRGBSurface(SDL_SRCCOLORKEY, 12, 12, 32, 0, 0, 0, 0);
	#endif
#else
			char_surface = SDL_CreateRGBSurface(exfont->flags, 12, 12, 32, (0xFF << rbyte*8), (0xFF << gbyte*8), (0xFF << bbyte*8), (0xFF << abyte*8));
#endif
#ifndef USE_ALPHA
			// Get color region from system graphic
			SDL_Rect clip_system = { (int16)(8+16*(font->color%10)), (int16)(4+48+16*(font->color/10)), 6, 12 };
			// Define position of glyph
			SDL_Rect rect = {(int16)next_glyph_pos, 0, 0, 0};
			// Define position of glyph shadow
			SDL_Rect shadow_r = {(int16)next_glyph_pos+1, 1, 0, 0};
			SDL_Rect char_r = { 0, 0, 0, 0 };

			// Get exfont from graphic
			SDL_Rect rect_exfont = {(int16)(exfont_value % 13) * 12, (int16)(exfont_value / 13) * 12, 12, 12};

			// Blit white exfont on black mask
			SDL_BlitSurface(exfont, &rect_exfont, mask, NULL);

			// Make white transparent
			SDL_SetColorKey(mask, SDL_SRCCOLORKEY, SDL_MapRGB(mask->format, 255, 255, 255));

			// Blit color background
			SDL_BlitSurface(((SdlBitmap*)system)->bitmap, &clip_system, char_surface, &char_r);
			char_r.x += 6;
			// Blit color background again (6 pixels width each)
			SDL_BlitSurface(((SdlBitmap*)system)->bitmap, &clip_system, char_surface, &char_r);

			// Blit black mask onto color background
			SDL_BlitSurface(mask, NULL, char_surface, NULL);

			// Make black color transparent for the magic!
			SDL_SetColorKey(char_surface, SDL_SRCCOLORKEY, SDL_MapRGB(char_surface->format, 0, 0, 0));

			// Get SDL_Color from shadow color
			SDL_Color c_tmp = { shadowColor.red, shadowColor.green, shadowColor.blue, 0 };
			// If shadow is pure black, add 1 to blue channel
			// so it doesn't become transparent
			if ( (c_tmp.r == 0) &&
				(c_tmp.g == 0) &&
				(c_tmp.b == 0) ) 
			{
				c_tmp.b++;
			}
			// Paint char shadow surface of shadow color
			SDL_FillRect(char_shadow, NULL, SDL_MapRGB(char_shadow->format, c_tmp.r, c_tmp.g, c_tmp.b));

			// Clear color key for mask
			SDL_SetColorKey(mask, 0, 0);

			// Paint black color mask (aka reset it)
			SDL_FillRect(mask, NULL, 0);

			// Paste white exfont onto mask
			SDL_BlitSurface(exfont, &rect_exfont, mask, NULL);

			// Make white transparent
			SDL_SetColorKey(mask, SDL_SRCCOLORKEY, SDL_MapRGB(char_surface->format, 255, 255, 255));

			// Paste mask onto char_shadow
			SDL_BlitSurface(mask, NULL, char_shadow, NULL);

			// Make black color key for magic!
			SDL_SetColorKey(mask, SDL_SRCCOLORKEY, SDL_MapRGB(char_surface->format, 0, 0, 0));

			// Finally, blit shadow first, and then char onto the text_surface
			SDL_BlitSurface(char_shadow, NULL, text_surface, &shadow_r);
			SDL_BlitSurface(char_surface, NULL, text_surface, &rect);

#else
			char_shadow = SDL_ConvertSurface(char_surface, char_surface->format, char_surface->flags);
			SDL_Rect rect = {(exfont_value % 13) * 12, (exfont_value / 13) * 12, 12, 12};
			SDL_BlitSurface(exfont, &rect, char_surface, NULL);
			SDL_BlitSurface(exfont, &rect, char_shadow, NULL);
#endif
			// Done
#ifndef USE_ALPHA
			SDL_FreeSurface(mask);
#endif
			SDL_FreeSurface(char_surface);
			SDL_FreeSurface(char_shadow);
		} else {
			// No EXFONT, draw normal text
#if FONT_SMOOTHING == 0
			// Render a single char
	#ifdef USE_ALPHA
			char_surface = TTF_RenderUTF8_Solid(ttf_font, text2, Color(255, 255, 255, 255).Get());
			char_shadow = TTF_RenderUTF8_Solid(ttf_font, text2, shadowColor.Get());
	#else
			char_surface = TTF_RenderUTF8_Solid(ttf_font, text2, color);
			// If shadow is pure black, add 1 to blue channel
			// so it doesn't become transparent
			SDL_Color c_tmp = { shadowColor.red, shadowColor.green, shadowColor.blue, 0 };
			if ( (c_tmp.r == 0) &&
				(c_tmp.g == 0) &&
				(c_tmp.b == 0) ) 
			{
				shadowColor.blue++;
			}
			char_shadow = TTF_RenderUTF8_Solid(ttf_font, text2, c_tmp);

			if (!char_surface || !char_shadow) {
				Output::Error("Couldn't draw text %s with Font %n size %d.\n%s\n", text.c_str(), font->name.c_str(),			font->size, TTF_GetError());
			}
	#endif
#else
			char_surface = TTF_RenderUTF8_Blended(ttf_font, text2, Color(255, 255, 255, 255).Get());
			char_shadow = TTF_RenderUTF8_Blended(ttf_font, text2, shadowColor.Get());
#endif

#ifndef USE_ALPHA
			// Create a black mask
			mask = SDL_CreateRGBSurface(SDL_SRCCOLORKEY, char_surface->w, char_surface->h, 32, 0, 0, 0, 0);

			// Paste white text onto mask
			SDL_BlitSurface(char_surface, NULL, mask, NULL);

			// Make white transparente to create the actual mask
			SDL_SetColorKey(mask, SDL_SRCCOLORKEY, SDL_MapRGB(mask->format, 255, 255, 255));

			// Get color region from system graphic
			SDL_Rect clip_system = { (int16)(8+16*(font->color%10)), (int16)(4+48+16*(font->color/10)), (uint16)char_surface->w, (uint16)char_surface->h };
			
			// Define position of glyph
			SDL_Rect rect = {(int16)next_glyph_pos, 0, 0, 0};

			// Blit color background
			SDL_BlitSurface(((SdlBitmap*)system)->bitmap, &clip_system, text_surface_aux, &rect);

			// Blit mask onto background
			SDL_BlitSurface(mask, NULL, text_surface_aux, &rect);

			// Make black transparent for the magic!
			SDL_SetColorKey(text_surface_aux, SDL_SRCCOLORKEY, SDL_MapRGB(text_surface_aux->format, 0,0,0));
			SDL_SetColorKey(text_surface, SDL_SRCCOLORKEY, SDL_MapRGB(text_surface->format, 0,0,0));

			// Define text shadow position
			SDL_Rect dst = { (int16)next_glyph_pos+1, 1, 0, 0 };

			// Blit first shadow and then text
			SDL_BlitSurface(char_shadow, NULL, text_surface, &dst);
			SDL_BlitSurface(text_surface_aux, NULL, text_surface, NULL);
#endif
			// Done
#ifndef USE_ALPHA
			SDL_FreeSurface(mask);
			SDL_FreeSurface(char_surface);
			SDL_FreeSurface(char_shadow);
#endif
		}
/*
#ifdef USE_ALPHA
		SDL_Rect rect = {next_glyph_pos + 1, 1, is_full_glyph ? 12 : 6, 12};
#endif*/

#if FONT_SMOOTHING == 0 && defined(USE_ALPHA)
		
		// Retrieve the color key once
		if (c == 0) {
			colorkey = *(Uint32*)char_surface->format->palette->colors | (0xFF << abyte*8);
		}
		// Convert to RGBA
		SDL_Surface* char_surface32 = SDL_ConvertSurface(char_surface, bitmap->format, char_surface->flags);
		SDL_Surface* char_shadow32 = SDL_ConvertSurface(char_shadow, bitmap->format, char_shadow->flags);
		SDL_FreeSurface(char_surface);
		SDL_FreeSurface(char_shadow);
		char_surface = char_surface32;
		char_shadow = char_shadow32;
#endif

#ifdef USE_ALPHA
		// Set corrent alpha flags otherwise alpha is ignored during blitting
		SDL_SetAlpha(char_surface, 0, 0);
		SDL_SetAlpha(char_shadow, 0, 0);

		// Color Blending
		SDL_LockSurface(char_surface);
		SDL_LockSurface(char_shadow);
		Uint8* pixels = (Uint8*)char_surface->pixels;
		Uint8* shadow_pixels = (Uint8*)char_shadow->pixels;

		// Get the correct color from the system file
		int color_row = 0; // First or second row
		int color_index = font.color;
		if (font.color > 9 && font.color < 20) {
			color_row = 1;
			color_index = font.color / 10;
		}

		for (int i = 0; i < char_surface->h; ++i) {
			int y = 52 + i + 1 + (color_row == 1?17:0);

			for (int j = 0; j < char_surface->w; ++j, pixels+=4, shadow_pixels+=4) {
				int x = j + 1 + 16*color_index;
				
				Color drawColor(system->GetPixel(x, y));

#if FONT_SMOOTHING == 0
				// Make everything matching the colorkey transparent
				if (*(Uint32*)pixels == colorkey) {
					pixels[abyte] = SDL_ALPHA_TRANSPARENT;
					shadow_pixels[abyte] = SDL_ALPHA_TRANSPARENT;
					continue;
				}
#endif
				pixels[rbyte] = drawColor.red;
				pixels[gbyte] = drawColor.green;
				pixels[bbyte] = drawColor.blue;
			}
		}
		SDL_UnlockSurface(char_surface);
		SDL_UnlockSurface(char_shadow);
		// End of Color blending

		// Blit the char and drop shadow on the text surface
		SDL_Rect rect = {next_glyph_pos + 1, 1, is_full_glyph ? 12 : 6, 12};
		SDL_BlitSurface(char_shadow, NULL, text_surface, &rect);
		rect.x = next_glyph_pos;
		rect.y = 0;
		SDL_BlitSurface(char_surface, NULL, text_surface, &rect);

		SDL_FreeSurface(char_surface);
		SDL_FreeSurface(char_shadow);
#endif
		// If it's a full size glyph add the size of a half-size glypth twice
		if (is_full_glyph) {
			next_glyph_pos += 6;
			is_full_glyph = false;
			// Skip the next character
			// Note that this is useful for exfont only!
			// If there should be ever support for normal Full-Glyphs this will
			// cause problems
			++c;
		}
		next_glyph_pos += 6;	
	}
	
	SdlBitmap* text_bmp = new SdlBitmap(1, 1);
	SDL_FreeSurface(text_bmp->bitmap);
	text_bmp->bitmap = text_surface;

	// This function compresses the text if there is not enough place
	// Not used in RPG2k, only in RPGXp
	/*if (text_bmp->GetWidth() > rect.width) {
		int stretch = (int)(text_bmp->GetWidth() * 0.4);
		if (rect.width > stretch) stretch = rect.width;
		Rect resample_rect(0, 0, text_bmp->GetWidth(), text_bmp->GetHeight());
		Bitmap* resampled = text_bmp->Resample(stretch, text_bmp->GetHeight(), resample_rect);
		delete text_bmp;
		text_bmp = resampled;
	}*/
	
	Rect src_rect(0, 0, dst_rect.width, dst_rect.height);
	int y = dst_rect.y;
	if (dst_rect.height > text_bmp->GetHeight()) y += ((dst_rect.height - text_bmp->GetHeight()) / 2);
	int x = dst_rect.x;
	
	// Alignment code
	if (dst_rect.width > text_bmp->GetWidth()) {
		if (align == Bitmap::TextAlignCenter) {
			x += (dst_rect.width - text_bmp->GetWidth()) / 2;
		} else if (align == Bitmap::TextAlignRight) {
			x += dst_rect.width - text_bmp->GetWidth();
		}
	}

	Blit(x, y, text_bmp, src_rect, SDL_ALPHA_OPAQUE/*font.color.alpha*/);

	delete text_bmp;
#ifndef USE_ALPHA
	SDL_FreeSurface(text_surface_aux);
#endif
	SDL_FreeSurface(exfont); // FIXME : Exfont should be cached
}

////////////////////////////////////////////////////////////
Rect SdlBitmap::GetTextSize(std::string text) const {
	return Rect(0, 0, text.size() * 6, 12);
}

////////////////////////////////////////////////////////////
void SdlBitmap::SetTransparentColor(Color color) {
	#ifndef USE_ALPHA
		SDL_SetColorKey(bitmap, COLORKEY_FLAGS, GetUint32Color(color));

		//colorkey = GetUint32Color(color);
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
