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
//#include "SDL_rotozoom.h"
#include "SDL_ttf.h"
#include "sdl_ui.h"
#include "system.h"
#include "util_macro.h"

////////////////////////////////////////////////////////////
#ifdef USE_ALPHA
	#define TRANSPARENT_FLAGS SDL_SRCALPHA
	#define COLORKEY_FLAGS SDL_SRCCOLORKEY
	#define DisplayFormat(surface) SDL_DisplayFormatAlpha(surface)
#else
	#define TRANSPARENT_FLAGS SDL_SRCCOLORKEY
	#ifdef USE_RLE
		#define COLORKEY_FLAGS SDL_SRCCOLORKEY | SDL_RLEACCEL
		#define SETALPHA_FLAGS SDL_SRCALPHA | SDL_RLEACCEL
	#else
		#define COLORKEY_FLAGS SDL_SRCCOLORKEY
		#define SETALPHA_FLAGS SDL_SRCALPHA
	#endif
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

	SDL_Surface* temp = IMG_Load(filename.c_str());

	if (temp == NULL) {
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
		RemovePaletteColorkeyDuplicates(temp, &colorkey);
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
	SDL_Rect src_r = {(int16)src_rect.x, (int16)src_rect.y, (uint16)src_rect.width, (uint16)src_rect.height};
	SDL_Rect dst_r = {(int16)x, (int16)y, 0, 0};

	#ifdef USE_ALPHA
	if (opacity < 255)
		Bitmap::Blit(x, y, src, src_rect, opacity);
	else {
		bool has_alpha = (((SdlBitmap*)src)->bitmap->flags & SDL_SRCALPHA) != 0;
		SDL_SetAlpha(((SdlBitmap*)src)->bitmap, 0, 255);
		SDL_BlitSurface(((SdlBitmap*)src)->bitmap, &src_r, bitmap, &dst_r);
		SDL_SetAlpha(((SdlBitmap*)src)->bitmap, has_alpha ? SDL_SRCALPHA : 0, 255);
	}
	#else
		if (opacity < 255) SDL_SetAlpha(((SdlBitmap*)src)->bitmap, SETALPHA_FLAGS, (uint8)opacity);

		SDL_BlitSurface(((SdlBitmap*)src)->bitmap, &src_r, bitmap, &dst_r);
		
		if (opacity < 255) SDL_SetAlpha(((SdlBitmap*)src)->bitmap, SETALPHA_FLAGS, 255);
	#endif

	RefreshCallback();
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
void SdlBitmap::TextDraw(int x, int y, std::string text, TextAlignment align) {
	if (text.length() == 0) return;
	Rect dst_rect = GetTextSize(text);
	dst_rect.x = x; dst_rect.y = y;
	dst_rect.width += 1; dst_rect.height += 1; // Need place for shadow
	if (dst_rect.IsOutOfBounds(GetWidth(), GetHeight())) return;

	TTF_Font* ttf_font = font->GetTTF();
	int style = 0;
	if (font->bold) style |= TTF_STYLE_BOLD;
	if (font->italic) style |= TTF_STYLE_ITALIC;
	TTF_SetFontStyle(ttf_font, style);

	Bitmap* text_surface; // Complete text will be on this surface
	Bitmap* text_surface_aux;
	Bitmap* mask;

	Bitmap* char_surface; // Single char
	Bitmap* char_shadow; // Drop shadow of char

	text_surface = CreateBitmap(dst_rect.width, TTF_FontHeight(ttf_font));
	text_surface_aux = CreateBitmap(dst_rect.width, TTF_FontHeight(ttf_font));

	Color white_color(255, 255, 255, 0);
	Color black_color(0, 0, 0, 0);

	text_surface_aux->SetTransparentColor(black_color);
	text_surface->SetTransparentColor(black_color);

	char text2[2]; text2[1] = '\0';

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

	#ifdef USE_ALPHA
	SDL_SetColorKey(((SdlBitmap*) exfont)->bitmap, COLORKEY_FLAGS, SDL_MapRGB(((SdlBitmap*) exfont)->bitmap->format, 0, 0, 0));
	#endif

	// This loops always renders a single char, color blends it and then puts
	// it onto the text_surface (including the drop shadow)
	for (unsigned c = 0; c < text.size(); ++c) {
		text2[0] = text[c];

		Rect next_glyph_rect(next_glyph_pos, 0, 0, 0);

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

		#ifdef USE_ALPHA
			char_surface = CreateBitmap(12, 12);
			char_shadow = CreateBitmap(12, 12);
			mask = CreateBitmap(12, 12);

			// Get exfont from graphic
			Rect rect_exfont((exfont_value % 13) * 12, (exfont_value / 13) * 12, 12, 12);

			text_surface->Blit(next_glyph_rect.x, next_glyph_rect.y, exfont, rect_exfont, 255);

			// Get color region from system graphic
			Rect clip_system(8+16*(font->color%10), 4+48+16*(font->color/10), 6, 12);

			// Blit color background (twice because its a full glyph)
			char_surface->Blit(0, 0, system, clip_system, 255);
			char_surface->Blit(6, 0, system, clip_system, 255);

			// Blit black mask onto color background
			SDL_Rect src_r = {(int16)clip_system.x, (int16)clip_system.y, (uint16)clip_system.width, (uint16)clip_system.height};
			SDL_Rect dst_r = {(int16)next_glyph_rect.x, (int16)next_glyph_rect.y, 0, 0};
			SDL_BlitSurface(((SdlBitmap*)system)->bitmap, &src_r, ((SdlBitmap*)text_surface)->bitmap, &dst_r);
			dst_r.x += 6;
			SDL_BlitSurface(((SdlBitmap*)system)->bitmap, &src_r, ((SdlBitmap*)text_surface)->bitmap, &dst_r);
		#else
			char_surface = CreateBitmap(12, 12);
			char_shadow = CreateBitmap(12, 12);
			mask = CreateBitmap(12, 12);

			// Get exfont from graphic
			Rect rect_exfont((exfont_value % 13) * 12, (exfont_value / 13) * 12, 12, 12);

			// Create a black mask
			mask->Blit(0, 0, exfont, rect_exfont, 255);
			mask->SetTransparentColor(white_color);

			// Get color region from system graphic
			Rect clip_system(8+16*(font->color%10), 4+48+16*(font->color/10), 6, 12);

			// Blit color background (twice because its a full glyph)
			char_surface->Blit(0, 0, system, clip_system, 255);
			char_surface->Blit(6, 0, system, clip_system, 255);

			// Blit black mask onto color background
			char_surface->Blit(0, 0, mask, mask->GetRect(), 255);
			char_surface->SetTransparentColor(black_color);

			// Paint char shadow surface of shadow color
			char_shadow->Fill(shadow_color);
			// Reset the mask
			mask->Fill(black_color);

			// Paste white exfont onto mask
			mask->Blit(0, 0, exfont, rect_exfont, 255);
			mask->SetTransparentColor(white_color);

			// Paste mask onto char_shadow
			char_shadow->Blit(0, 0, mask, mask->GetRect(), 255);

			// Blit first shadow and then text
			text_surface->Blit(next_glyph_rect.x + 1, next_glyph_rect.y + 1, char_shadow, char_shadow->GetRect(), 255);
			text_surface->Blit(next_glyph_rect.x, next_glyph_rect.y, char_surface, char_surface->GetRect(), 255);
		#endif
		} else {
			// No ExFont, draw normal text

		#ifdef USE_ALPHA
			// ToDo: Remove SDL-Dependency (use FreeType directly?) 
			SDL_Color white_color2 = {255, 255, 255, 255};
			SDL_Color black_color2 = {0, 0, 0, 255};
			SDL_Surface* char_surface_temp = TTF_RenderUTF8_Solid(ttf_font, text2, white_color2);
			SDL_Surface* char_shadow_temp = TTF_RenderUTF8_Solid(ttf_font, text2, black_color2);
			SDL_Surface* char_surface_surf = DisplayFormat(char_surface_temp);
			SDL_Surface* char_shadow_surf = DisplayFormat(char_shadow_temp);
			SDL_FreeSurface(char_surface_temp);
			SDL_FreeSurface(char_shadow_temp);

			char_surface = new SdlBitmap(char_surface_surf);
			char_shadow = new SdlBitmap(char_shadow_surf);

			if (!((SdlBitmap*)char_surface)->bitmap || !((SdlBitmap*)char_shadow)->bitmap) {
				Output::Warning("Couldn't render char %c (%d). Skipping...", text[c], (int)text[c]);
				delete char_surface;
				delete char_shadow;
				continue;
			}
			// Create a black mask
			mask = CreateBitmap(char_surface->GetWidth(), char_surface->GetHeight());
			mask->Blit(0, 0, char_surface, char_surface->GetRect(), 255);

			// Get color region from system graphic
			Rect clip_system(8+16*(font->color%10), 4+48+16*(font->color/10), char_surface->GetWidth(), char_surface->GetHeight());

			text_surface->Blit(next_glyph_rect.x, next_glyph_rect.y, char_surface, char_surface->GetRect(), 255);
			// Blit color background
			SDL_Rect src_r = {(int16)clip_system.x, (int16)clip_system.y, (uint16)clip_system.width, (uint16)clip_system.height};
			SDL_Rect dst_r = {(int16)next_glyph_rect.x, (int16)next_glyph_rect.y, 0, 0};
			SDL_BlitSurface(((SdlBitmap*)system)->bitmap, &src_r, ((SdlBitmap*)text_surface)->bitmap, &dst_r);
			// text_surface->Blit(next_glyph_rect.x, next_glyph_rect.y, system, clip_system, 255);
		#else
			// ToDo: Remove SDL-Dependency (use FreeType directly?)
			SDL_Color white_color2 = {white_color.red, white_color.green, white_color.blue, 0};
			SDL_Color c_tmp2 = {shadow_color.red, shadow_color.green, shadow_color.blue, 0};
			char_surface = new SdlBitmap(TTF_RenderUTF8_Solid(ttf_font, text2, white_color2));
			char_shadow = new SdlBitmap(TTF_RenderUTF8_Solid(ttf_font, text2, c_tmp2));

			if (!((SdlBitmap*)char_surface)->bitmap || !((SdlBitmap*)char_shadow)->bitmap) {
				Output::Warning("Couldn't render char %c (%d). Skipping...", text[c], (int)text[c]);
				delete char_surface;
				delete char_shadow;
				continue;
			}

			// Create a black mask
			mask = CreateBitmap(char_surface->GetWidth(), char_surface->GetHeight());
			mask->Blit(0, 0, char_surface, char_surface->GetRect(), 255);
			mask->SetTransparentColor(white_color);

			// Get color region from system graphic
			Rect clip_system(8+16*(font->color%10), 4+48+16*(font->color/10), char_surface->GetWidth(), char_surface->GetHeight());

			// Blit color background
			text_surface_aux->Blit(next_glyph_rect.x, next_glyph_rect.y, system, clip_system, 255);
			// Blit mask onto background
			text_surface_aux->Blit(next_glyph_rect.x, next_glyph_rect.y, mask, mask->GetRect(), 255);

			// Blit first shadow and then text
			text_surface->Blit(next_glyph_pos+1, 1, char_shadow, char_shadow->GetRect(), 255);
			text_surface->Blit(0, 0, text_surface_aux, text_surface_aux->GetRect(), 255);
		#endif
		}

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

	Blit(ix, iy, text_bmp, src_rect, SDL_ALPHA_OPAQUE);

	delete text_bmp;
	delete text_surface;
	delete text_surface_aux;
}

////////////////////////////////////////////////////////////
Rect SdlBitmap::GetTextSize(std::string text) const {
	return Rect(0, 0, text.size() * 6, 12);
	//return Rect(0, 0, text.size() * 6, min(TTF_FontHeight(font->GetTTF()), 12));
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
