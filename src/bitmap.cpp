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
#include "bitmap.h"
#include "cache.h"
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

////////////////////////////////////////////////////////////
/// Defines
////////////////////////////////////////////////////////////

// Format ARGB (which is the one used by SDL internally)
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    const unsigned int rmask = 0x0000FF00;
    const unsigned int gmask = 0x00FF0000;
    const unsigned int bmask = 0xFF000000;
    const unsigned int amask = 0x000000FF;
#else
    const unsigned int rmask = 0x00FF0000;
    const unsigned int gmask = 0x0000FF00;
    const unsigned int bmask = 0x000000FF;
    const unsigned int amask = 0xFF000000;
#endif


////////////////////////////////////////////////////////////
/// Constructors
////////////////////////////////////////////////////////////
Bitmap::Bitmap(int width, int height) {
	SDL_Surface* temp;
#ifdef USE_ALPHA
	temp = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, rmask, gmask, bmask, amask);
#else
	// Create 32 bit (for now) surface with SRC_COLORKEY
	temp = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCCOLORKEY, width, height, 32, rmask, gmask, bmask, amask);
#endif
	if (temp == NULL) {
		Output::Error("Couldn't create %dx%d image.\n%s\n", width, height, SDL_GetError());
	}
#ifdef USE_ALPHA
	bitmap = SDL_DisplayFormatAlpha(temp);
#else
	// Don't need alpha here
	bitmap = SDL_DisplayFormat(temp);
#endif
	if (bitmap == NULL) {
		Output::Error("Couldn't optimize %dx%d image.\n%s\n", width, height, SDL_GetError());
	}
	SDL_FreeSurface(temp);
}
Bitmap::Bitmap(std::string filename, bool transparent) {
	std::string path = FileFinder::FindImage(filename);
	if (path == "") {
		Output::Error("No such file or directory - %s", filename.c_str());
	}
	SDL_Surface* temp;
	temp = IMG_Load(path.c_str());
	if (temp == NULL) {
		Output::Error("Couldn't load %s image.\n%s\n", filename.c_str(), IMG_GetError());
	}
	Color col(0, 0, 0, 0);
	if ((temp->format->BitsPerPixel == 8) && (transparent)) {
		// The first color of the palette will be the key color
		// if needed.
		SDL_Color colorkey = temp->format->palette->colors[0];
		col.red = colorkey.r;
		col.green = colorkey.g;
		col.blue = colorkey.b;
#ifndef USE_ALPHA
		// Remove color duplicates from palette
		// This hack is needed to emulate RPG_RT behaviour
		RemoveColorDuplicates(temp, &colorkey);
		// Set color key
		SDL_SetColorKey(temp, SDL_SRCCOLORKEY, SDL_MapRGB(temp->format, col.red, col.green, col.blue));
#endif
	}
#ifdef USE_ALPHA
	bitmap = SDL_DisplayFormatAlpha(temp);
#else
	// Don't need alpha
	bitmap = SDL_DisplayFormat(temp);
#endif
	if (bitmap == NULL) {
		Output::Error("Couldn't optimize %s image.\n%s\n", filename.c_str(), SDL_GetError());
	}
#ifdef USE_ALPHA
	if (temp->format->BitsPerPixel == 8) {
		SetTransparent(col);	
	}
#endif
	SDL_FreeSurface(temp);
}
Bitmap::Bitmap(Bitmap* source, Rect src_rect) {
	SDL_Surface* temp;
#ifdef USE_ALPHA
	temp = SDL_CreateRGBSurface(SDL_SWSURFACE, src_rect.width, src_rect.height, 32, rmask, gmask, bmask, amask);
#else
	temp = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCCOLORKEY, src_rect.width, src_rect.height, 32, rmask, gmask, bmask, amask);
#endif
	if (temp == NULL) {
		Output::Error("Couldn't create %dx%d image.\n%s\n", src_rect.width, src_rect.height, SDL_GetError());
	}
#ifdef USE_ALPHA
	bitmap = SDL_DisplayFormatAlpha(temp);
#else
	bitmap = SDL_DisplayFormat(temp);
#endif
	if (bitmap == NULL) {
		Output::Error("Couldn't optimize %dx%d image.\n%s\n", src_rect.width, src_rect.height, SDL_GetError());
	}
	SDL_FreeSurface(temp);
	Blit(0, 0, source, src_rect, 255);
}

void Bitmap::RemoveColorDuplicates(SDL_Surface* src, SDL_Color* src_color) {
	// Iterate through the palete to remove all color duplicates except
	// for the first ocurrence.
	int n_colors = src->format->palette->ncolors;
	SDL_Color* colors = src->format->palette->colors;

	bool color_was_found = false;

	for (int i = 0; i < n_colors; i++) {
		if (color_was_found) {
			if ( (colors[i].r == src_color->r) &&
				(colors[i].g == src_color->g) &&
				(colors[i].b == src_color->b) 
				) {
					(colors[i].b != 255) ? colors[i].b++ : colors[i].b--;
			}

		} else {
			if ( (colors[i].r == src_color->r) &&
				(colors[i].g == src_color->g) &&
				(colors[i].b == src_color->b) 
				) {
					color_was_found = true;
			}
		}
	}
}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
Bitmap::~Bitmap() {
	SDL_FreeSurface(bitmap);
}

////////////////////////////////////////////////////////////
/// Blit screen
////////////////////////////////////////////////////////////
void Bitmap::BlitScreen(int x, int y) {
	SDL_Rect offset = {x, y, 0, 0};
	SDL_BlitSurface(bitmap, NULL, Player::main_window, &offset);
}
void Bitmap::BlitScreen(int x, int y, int opacity) {
	Rect src_rect(0, 0, GetWidth(), GetHeight());
	BlitScreen(x, y, src_rect, opacity);
}
void Bitmap::BlitScreen(int x, int y, Rect src_rect, int opacity) {
	if (GetWidth() == 0 || GetHeight() == 0) return;
	if (x >= Player::GetWidth() || y >= Player::GetHeight()) return;
#ifndef USE_ALPHA
	SDL_Rect src_r = { src_rect.x, src_rect.y, src_rect.width, src_rect.height };
	SDL_Rect dst_r = {x, y, 0, 0};

	// Regular blit here...
	SDL_BlitSurface(bitmap, &src_r, Player::main_window, &dst_r);
#else	
	if (opacity >= 255) {
		SDL_Rect offset = {x, y, 0, 0};
		SDL_Rect src_rect_sdl = {src_rect.x, src_rect.y, src_rect.width, src_rect.height};
		if (SDL_BlitSurface(bitmap, &src_rect_sdl, Player::main_window, &offset) < 0) {
			Output::Error("Bitmap.cpp BlitScreen(): Could not blit surface:\n%s", SDL_GetError());
		}
	} else if (opacity > 0) {
		src_rect.Adjust(GetWidth(), GetHeight());
		if (src_rect.IsOutOfBounds(GetWidth(), GetHeight())) return;

		SDL_LockSurface(Player::main_window);
		SDL_LockSurface(bitmap);
		
		int src_stride = GetWidth() * 4;
		int dst_stride = Player::main_window->w * 4;
		const Uint8* src_pixels = ((Uint8*)bitmap->pixels) + (src_rect.x + src_rect.y * GetWidth()) * 4;
		Uint8* dst_pixels = ((Uint8*)Player::main_window->pixels) + (x + y * Player::main_window->w) * 4;

		for (int i = 0; i < src_rect.height; ++i) {
			for (int j = 0; j < src_rect.width; ++j) {
				const Uint8* src = src_pixels + j * 4;
				Uint8* dst = dst_pixels + j * 4;

				Uint8 srca = src[3] * opacity / 255;
				dst[0] = (dst[0] * (255 - srca) + src[0] * srca) / 255;
				dst[1] = (dst[1] * (255 - srca) + src[1] * srca) / 255;
				dst[2] = (dst[2] * (255 - srca) + src[2] * srca) / 255;
				dst[3] = dst[3] * (255 - srca) / 255 + srca;
			}
			src_pixels += src_stride;
			dst_pixels += dst_stride;
		}
		
		SDL_UnlockSurface(Player::main_window);
		SDL_UnlockSurface(bitmap);
	}
#endif
}

////////////////////////////////////////////////////////////
/// Get width
////////////////////////////////////////////////////////////
int Bitmap::GetWidth() const {
	return bitmap->w;
}

////////////////////////////////////////////////////////////
/// Get height
////////////////////////////////////////////////////////////
int Bitmap::GetHeight() const {
	return bitmap->h;
}

////////////////////////////////////////////////////////////
/// Blit
////////////////////////////////////////////////////////////
void Bitmap::Blit(int x, int y, Bitmap* src_bitmap, Rect src_rect, int opacity) {
	SDL_Surface* source = src_bitmap->bitmap;

	if (source->w == 0 || source->h == 0 || GetWidth() == 0 || GetHeight() == 0) return;
	if (x >= GetWidth() || y >= GetHeight()) return;
	
	src_rect.Adjust(src_bitmap->GetWidth(), src_bitmap->GetHeight());
	if (src_rect.IsOutOfBounds(src_bitmap->GetWidth(), src_bitmap->GetHeight())) return;
#ifndef USE_ALPHA
	SDL_Rect src_r = {src_rect.x, src_rect.y, src_rect.width, src_rect.height };
	SDL_Rect dst_r = {x, y, 0, 0};

	// Again regular blit...
	SDL_BlitSurface(source, &src_r, bitmap, &dst_r);
#else
	int width = src_rect.width;
	int height = src_rect.height;
	if (x < 0) {
		width += x;
		x = 0;
	}
	if (y < 0) {
		height += y;
		y = 0;
	}
	if (x + width > GetWidth()) width = GetWidth() - x;
	if (y + height > GetHeight()) height = GetHeight() - y;	
	if (width <= 0 || height <= 0) return;

	SDL_LockSurface(source);
	SDL_LockSurface(bitmap);
	
	int src_stride = source->w * 4;
	int dst_stride = GetWidth() * 4;
	const Uint8* src_pixels = ((Uint8*)source->pixels) + (src_rect.x + src_rect.y * source->w) * 4;
	Uint8* dst_pixels = ((Uint8*)bitmap->pixels) + (x + y * GetWidth()) * 4;

	const int rbyte = MaskGetByte(bitmap->format->Rmask);
	const int gbyte = MaskGetByte(bitmap->format->Gmask);
	const int bbyte = MaskGetByte(bitmap->format->Bmask);
	const int abyte = MaskGetByte(bitmap->format->Amask);

	if (opacity > 255) opacity = 255;
	if (opacity > 0) {
		for (int i = 0; i < height; ++i) {
			for (int j = 0; j < width; ++j) {
				const Uint8* src = src_pixels + j * 4;
				Uint8* dst = dst_pixels + j * 4;

				Uint8 srca = src[abyte] * opacity / 255;
				dst[rbyte] = (dst[rbyte] * (255 - srca) + src[rbyte] * srca) / 255;
				dst[gbyte] = (dst[gbyte] * (255 - srca) + src[gbyte] * srca) / 255;
				dst[bbyte] = (dst[bbyte] * (255 - srca) + src[bbyte] * srca) / 255;
				dst[abyte] = dst[abyte] * (255 - srca) / 255 + srca;
			}
			src_pixels += src_stride;
			dst_pixels += dst_stride;
		}
	}
	
	SDL_UnlockSurface(source);
	SDL_UnlockSurface(bitmap);
#endif
}

// This method stretches `src` (according to `src_rect`)
// to the dimensions of `bitmap` and then blits the result
// to it. This is pretty much used for things like 
// message backgrounds. This is also the method that 
// old player used to do this.
void Bitmap::StretchBlit(Bitmap* src, Rect& src_rect) {
	SDL_Surface* tmp = NULL;
	Bitmap* tmp2 = NULL;

	double zoom_x_factor = bitmap->w / (double) src_rect.width;
	double zoom_y_factor = bitmap->h / (double) src_rect.height;

	tmp2 = new Bitmap(src_rect.width, src_rect.height);
	
	tmp2->Blit(0, 0, src, src_rect, 255);

	tmp = zoomSurface(tmp2->bitmap, zoom_x_factor, zoom_y_factor, 0);
	if (tmp == NULL) {
		Output::Error("Error when resizing\n");
	}

	SDL_BlitSurface(tmp, NULL, bitmap, NULL);

	SDL_FreeSurface(tmp);
	delete tmp2;
}

////////////////////////////////////////////////////////////
/// Stretch blit
////////////////////////////////////////////////////////////
void Bitmap::StretchBlit(Rect dst_rect, Bitmap* src_bitmap, Rect src_rect, int opacity) {
	if (src_rect.width == dst_rect.width && src_rect.height == dst_rect.height) {
		Blit(dst_rect.x, dst_rect.y, src_bitmap, src_rect, opacity);
	} else {
		src_rect.Adjust(src_bitmap->GetWidth(), src_bitmap->GetHeight());
		if (src_rect.IsOutOfBounds(src_bitmap->GetWidth(), src_bitmap->GetHeight())) return;
		Bitmap* resampled = src_bitmap->Resample(dst_rect.width, dst_rect.height, src_rect);
		Rect rect(0, 0, dst_rect.width, dst_rect.height);
		Blit(dst_rect.x, dst_rect.y, resampled, rect, opacity);
		delete resampled;
	}
}

// Tile src onto `bitmap` horizontally. This method calculates
// how many blits are neccessary in order to fill up `bitmap`
// horizontally.
void Bitmap::TileBlitX(const Rect& src_rect, Bitmap* src, const Rect& dst_rect) {
	SDL_Rect sdl_src_rect = { src_rect.x, src_rect.y, src_rect.width, src_rect.height };
	SDL_Rect sdl_dst_rect = { dst_rect.x, dst_rect.y, dst_rect.width, dst_rect.height };

	int n_blits;
	if ( (src_rect.width >= bitmap->w ) || (src_rect.width == 0) ) {
		n_blits = 1; 
	} else {
		n_blits = (int) ceil((float)bitmap->w / (float)src_rect.width);
	}

	for (int i = 0; i < n_blits; i++) {
		sdl_dst_rect.x = i*sdl_src_rect.w;
		SDL_BlitSurface(src->bitmap, &sdl_src_rect, bitmap, &sdl_dst_rect);
	}
}

// Same as TileBlitX() but tiles vertically.
void Bitmap::TileBlitY(const Rect& src_rect, Bitmap* src, const Rect& dst_rect) {
	SDL_Rect sdl_src_rect = { src_rect.x, src_rect.y, src_rect.width, src_rect.height };
	SDL_Rect sdl_dst_rect = { dst_rect.x, dst_rect.y, dst_rect.width, dst_rect.height };

	int n_blits;
	if ( (src_rect.height >= bitmap->h ) || (src_rect.height == 0) ) {
		n_blits = 1; 
	} else {
		n_blits = (int) ceil((float)bitmap->h / (float)src_rect.height);
	}

	for (int i = 0; i < n_blits; i++) {
		sdl_dst_rect.y = i*sdl_src_rect.h;
		SDL_BlitSurface(src->bitmap, &sdl_src_rect, bitmap, &sdl_dst_rect);
	}
}

// Sets the clipping rect for `bitmap` surface
void Bitmap::SetClipRect(const Rect& clip_rect) {
	SDL_Rect rect = {clip_rect.x, clip_rect.y, clip_rect.width, clip_rect.height };
	SDL_SetClipRect(bitmap, &rect);
}

// Clears the clipping rect for `bitmap` surface
void Bitmap::ClearClipRect() {
	SDL_SetClipRect(bitmap, NULL);
}

// Get the current colorkey for `bitmap`
Uint32 Bitmap::GetColorKey() const {
	return bitmap->format->colorkey;
}

// Set `color` as color key for `bitmap`
void Bitmap::SetColorKey(Uint32 color) {
	SDL_SetColorKey(bitmap, SDL_SRCCOLORKEY, color);
}

// Fills `bitmap` with the given Uint32 `color`
void Bitmap::FillofColor(Rect& rect, Uint32 color) {
	rect.Adjust(GetWidth(), GetHeight());
	if (rect.IsOutOfBounds(GetWidth(), GetHeight())) return;
	
	SDL_Rect rect_sdl = {rect.x, rect.y, rect.width, rect.height};
	SDL_FillRect(bitmap, &rect_sdl, color);
}

////////////////////////////////////////////////////////////
/// Fill rect
////////////////////////////////////////////////////////////
void Bitmap::FillRect(Rect rect, Color color) {
	rect.Adjust(GetWidth(), GetHeight());
	if (rect.IsOutOfBounds(GetWidth(), GetHeight())) return;
	
	SDL_Rect rect_sdl = {rect.x, rect.y, rect.width, rect.height};
	SDL_FillRect(bitmap, &rect_sdl, color.GetUint32(bitmap->format));
}

////////////////////////////////////////////////////////////
/// Clear
////////////////////////////////////////////////////////////
void Bitmap::Clear() {
	SDL_FillRect(bitmap, NULL, 0);
}
void Bitmap::Clear(Color color) {
	SDL_FillRect(bitmap, NULL, color.GetUint32(bitmap->format));
}

////////////////////////////////////////////////////////////
/// Get pixel
////////////////////////////////////////////////////////////
Color Bitmap::GetPixel(int x, int y) {
	if (x < 0 || y < 0) return Color(0, 0, 0, 0);
	if (x >= GetWidth() || y > GetHeight()) return Color(0, 0, 0, 0);
	if SDL_MUSTLOCK(bitmap)
		SDL_LockSurface(bitmap);
	Uint32* pixels = (Uint32*)bitmap->pixels;
	if SDL_MUSTLOCK(bitmap)
		SDL_UnlockSurface(bitmap);
	return Color(pixels[(y * GetWidth()) + x], bitmap->format);
}

////////////////////////////////////////////////////////////
/// Set pixel
////////////////////////////////////////////////////////////
void Bitmap::SetPixel(int x, int y, Color color) {
	if (x < 0 || y < 0) return;
	if (x >= GetWidth() || y > GetHeight()) return;
	if (SDL_MUSTLOCK(bitmap))
		SDL_LockSurface(bitmap);
	Uint32* pixels = (Uint32*)bitmap->pixels;
	pixels[(y * GetWidth()) + x] = color.GetUint32(bitmap->format);
	if (SDL_MUSTLOCK(bitmap))
		SDL_UnlockSurface(bitmap);
}

////////////////////////////////////////////////////////////
/// Hue change
////////////////////////////////////////////////////////////
void Bitmap::HueChange(double hue) {
	Uint32* pixels = (Uint32*)bitmap->pixels;
	
	for (int y = 0; y < GetHeight(); y++) {
		for (int x = 0; x < GetWidth(); x++) {
			Color color(pixels[(y * GetWidth()) + x], bitmap->format);
			pixels[(y * GetWidth()) + x] = RGBAdjustHSL(color, hue, 0, 1).GetUint32(bitmap->format);
		}
	}
}

////////////////////////////////////////////////////////////
/// Saturation change
////////////////////////////////////////////////////////////
void Bitmap::SatChange(double saturation) {
	Uint32* pixels = (Uint32*)bitmap->pixels;
	for (int y = 0; y < GetHeight(); y++) {
		for (int x = 0; x < GetWidth(); x++) {
			Color color(pixels[(y * GetWidth()) + x], bitmap->format);
			pixels[(y * GetWidth()) + x] = RGBAdjustHSL(color, 0, saturation, 1).GetUint32(bitmap->format);
		}
	}
}

////////////////////////////////////////////////////////////
/// Luminance change
////////////////////////////////////////////////////////////
void Bitmap::LumChange(double luminance) {
	Uint32* pixels = (Uint32*)bitmap->pixels;
	for (int y = 0; y < GetHeight(); y++) {
		for (int x = 0; x < GetWidth(); x++) {
			Color color(pixels[(y * GetWidth()) + x], bitmap->format);
			pixels[(y * GetWidth()) + x] = RGBAdjustHSL(color, 0, 0, luminance).GetUint32(bitmap->format);
		}
	}
}

////////////////////////////////////////////////////////////
/// HSL change
////////////////////////////////////////////////////////////
void Bitmap::HSLChange(double h, double s, double l) {
	Uint32* pixels = (Uint32*)bitmap->pixels;
	for (int y = 0; y < GetHeight(); y++) {
		for (int x = 0; x < GetWidth(); x++) {
			Color color(pixels[(y * GetWidth()) + x], bitmap->format);
			Color col2 = RGBAdjustHSL(color, h, s, l);
			pixels[(y * GetWidth()) + x] = RGBAdjustHSL(color, h, s, l).GetUint32(bitmap->format);
		}
	}
}
void Bitmap::HSLChange(double h, double s, double l, Rect rect) {
	Uint32* pixels = (Uint32*)bitmap->pixels;

	if (GetWidth() == 0 || GetHeight() == 0) return;

	rect.Adjust(GetWidth(), GetHeight());
	if (rect.IsOutOfBounds(GetWidth(), GetHeight())) return;

	for (int y = rect.y; y < rect.y + rect.height; y++) {
		for (int x = rect.x; x < rect.x + rect.width; x++) {
			Color color(pixels[(y * GetWidth()) + x], bitmap->format);
			pixels[(y * GetWidth()) + x] = RGBAdjustHSL(color, h, s, l).GetUint32(bitmap->format);
		}
	}
}

////////////////////////////////////////////////////////////
/// Draw text
////////////////////////////////////////////////////////////
void Bitmap::TextDraw(Rect rect, std::string text, int align) {
	if (text.length() == 0) return;
	if (rect.IsOutOfBounds(GetWidth(), GetHeight())) return;

#ifdef USE_ALPHA
	const int rbyte = MaskGetByte(bitmap->format->Rmask);
	const int gbyte = MaskGetByte(bitmap->format->Gmask);
	const int bbyte = MaskGetByte(bitmap->format->Bmask);
	const int abyte = MaskGetByte(bitmap->format->Amask);
#endif
	
	TTF_Font* ttf_font = font.GetTTF();
	int style = 0;
	if (font.bold) style |= TTF_STYLE_BOLD;
	if (font.italic) style |= TTF_STYLE_ITALIC;
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
	SDL_Surface* exfont = IMG_ReadXPMFromArray(const_cast<char**>(exfont_xpm));

	// Get the Shadow color
	Color shadowColor = system->GetPixel(16, 32);

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
				exfont_value = text[c+1]-'A';
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
			SDL_Rect clip_system = { 8+16*(font.color%10), 4+48+16*(font.color/10), 6, 12 };
			// Define position of glyph
			SDL_Rect rect = {next_glyph_pos, 0, 0, 0};
			// Define position of glyph shadow
			SDL_Rect shadow_r = {next_glyph_pos+1, 1, 0, 0};
			SDL_Rect char_r = { 0, 0, 0, 0 };

			// Get exfont from graphic
			SDL_Rect rect_exfont = {(exfont_value % 13) * 12, (exfont_value / 13) * 12, 12, 12};

			// Blit white exfont on black mask
			SDL_BlitSurface(exfont, &rect_exfont, mask, NULL);

			// Make white transparent
			SDL_SetColorKey(mask, SDL_SRCCOLORKEY, SDL_MapRGB(mask->format, 255, 255, 255));

			// Blit color background
			SDL_BlitSurface(system->bitmap, &clip_system, char_surface, &char_r);
			char_r.x += 6;
			// Blit color background again (6 pixels width each)
			SDL_BlitSurface(system->bitmap, &clip_system, char_surface, &char_r);

			// Blit black mask onto color background
			SDL_BlitSurface(mask, NULL, char_surface, NULL);

			// Make black color transparent for the magic!
			SDL_SetColorKey(char_surface, SDL_SRCCOLORKEY, SDL_MapRGB(char_surface->format, 0, 0, 0));

			// Get SDL_Color from shadow color
			SDL_Color c_tmp = shadowColor.Get();
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
			SDL_FreeSurface(mask);
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
			SDL_Color c_tmp = shadowColor.Get();
			if ( (c_tmp.r == 0) &&
				(c_tmp.g == 0) &&
				(c_tmp.b == 0) ) 
			{
				shadowColor.blue++;
			}
			char_shadow = TTF_RenderUTF8_Solid(ttf_font, text2, shadowColor.Get());

			if (!char_surface || !char_shadow) {
				Output::Error("Couldn't draw text %s with Font %n size %d.\n%s\n", text.c_str(), font.name.c_str(),			font.size, TTF_GetError());
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
			SDL_Rect clip_system = { 8+16*(font.color%10), 4+48+16*(font.color/10), char_surface->w, char_surface->h };
			
			// Define position of glyph
			SDL_Rect rect = {next_glyph_pos, 0, 0, 0};

			// Blit color background
			SDL_BlitSurface(system->bitmap, &clip_system, text_surface_aux, &rect);

			// Blit mask onto background
			SDL_BlitSurface(mask, NULL, text_surface_aux, &rect);

			// Make black transparent for the magic!
			SDL_SetColorKey(text_surface_aux, SDL_SRCCOLORKEY, SDL_MapRGB(text_surface_aux->format, 0,0,0));
			SDL_SetColorKey(text_surface, SDL_SRCCOLORKEY, SDL_MapRGB(text_surface->format, 0,0,0));

			// Define text shadow position
			SDL_Rect dst = { next_glyph_pos+1, 1, 0, 0 };

			// Blit first shadow and then text
			SDL_BlitSurface(char_shadow, NULL, text_surface, &dst);
			SDL_BlitSurface(text_surface_aux, NULL, text_surface, NULL);
#endif
			// Done
			SDL_FreeSurface(mask);
			SDL_FreeSurface(char_surface);
			SDL_FreeSurface(char_shadow);
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
				
				Color drawColor = system->GetPixel(x, y);

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
	
	Bitmap* text_bmp = new Bitmap(1, 1);
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
	
	Rect src_rect(0, 0, rect.width, rect.height);
	int y = rect.y;
	if (rect.height > text_bmp->GetHeight()) y += ((rect.height - text_bmp->GetHeight()) / 2);
	int x = rect.x;
	
	/*
	// Alignment code (RMXP only)
	if (rect.width > text_bmp->GetWidth()) {
		if (align == 1) {
			x += (rect.width - text_bmp->GetWidth()) / 2;
		} else if (align == 2) {
			x += rect.width - text_bmp->GetWidth();
		}
	}
	*/

	Blit(x, y, text_bmp, src_rect, SDL_ALPHA_OPAQUE/*font.color.alpha*/);

	delete text_bmp;
#ifndef USE_ALPHA
	SDL_FreeSurface(text_surface_aux);
#endif
	SDL_FreeSurface(exfont); // FIXME : Exfont should be cached
}

////////////////////////////////////////////////////////////
/// Get text size
////////////////////////////////////////////////////////////
Rect Bitmap::GetTextSize(std::string text) {
	TTF_Font* ttf_font = font.GetTTF();
	int style = 0;
	if (font.bold) style |= TTF_STYLE_BOLD;
	if (font.italic) style |= TTF_STYLE_ITALIC;
	TTF_SetFontStyle(ttf_font, style);
	
	int w, h;
	if (TTF_SizeUTF8(ttf_font, text.c_str(), &w, &h)) {
		Output::Error("Couldn't determine text size for Font %s size %d.\n%s\n", text.c_str(), font.name.c_str(), font.size, TTF_GetError());
	}
	return Rect(0, 0, w, h);
}

////////////////////////////////////////////////////////////
/// Gradient fill rect
////////////////////////////////////////////////////////////
void Bitmap::GradientFillRect(Rect rect, Color color1, Color color2, bool vertical) {
	// TODO
}

////////////////////////////////////////////////////////////
/// Clear Rect
////////////////////////////////////////////////////////////
void Bitmap::ClearRect(Rect rect) {
	// TODO
}

////////////////////////////////////////////////////////////
/// Blur
////////////////////////////////////////////////////////////
void Bitmap::Blur() {
	// TODO
}

////////////////////////////////////////////////////////////
/// Radial Blur
////////////////////////////////////////////////////////////
void Bitmap::RadialBlur(int angle, int division) {
	// TODO
}

////////////////////////////////////////////////////////////
/// Set Transparent
////////////////////////////////////////////////////////////
void Bitmap::SetTransparent(Color col) {
	SDL_LockSurface(bitmap);

	Uint8* pixels = (Uint8*)bitmap->pixels;

	const int rbyte = MaskGetByte(bitmap->format->Rmask);
	const int gbyte = MaskGetByte(bitmap->format->Gmask);
	const int bbyte = MaskGetByte(bitmap->format->Bmask);
	const int abyte = MaskGetByte(bitmap->format->Amask);
	for (int i = 0; i < GetHeight(); i++) {
		for (int j = 0; j < GetWidth(); j++) {
			Uint8* pixel = pixels;
			if (pixel[rbyte] == col.red && pixel[gbyte] == col.green && pixel[bbyte] == col.blue) {
				pixel[abyte] = 0;
			}
			pixels += 4;
		}
	}
	SDL_UnlockSurface(bitmap);
}

////////////////////////////////////////////////////////////
/// Tone change
////////////////////////////////////////////////////////////
void Bitmap::ToneChange(Tone tone) {
	if (tone.red == 0 && tone.green == 0 && tone.blue == 0 && tone.gray == 0) return;

	SDL_LockSurface(bitmap);

	Uint8* pixels = (Uint8*)bitmap->pixels;
	
	const int rbyte = MaskGetByte(bitmap->format->Rmask);
	const int gbyte = MaskGetByte(bitmap->format->Gmask);
	const int bbyte = MaskGetByte(bitmap->format->Bmask);

	if (tone.gray == 0) {
		for (int i = 0; i < GetHeight(); i++) {
			for (int j = 0; j < GetWidth(); j++) {
				Uint8* pixel = pixels;
				pixel[rbyte] = (Uint8)max(min(pixel[rbyte] + tone.red, 255), 0);
				pixel[gbyte] = (Uint8)max(min(pixel[gbyte] + tone.green, 255), 0);
				pixel[bbyte] = (Uint8)max(min(pixel[bbyte] + tone.blue, 255), 0);
				pixels += 4;
			}
		}
	} else {
		double factor = (255 - tone.gray) / 255.0;
		double gray;
		for (int i = 0; i < GetHeight(); i++) {
			for (int j = 0; j < GetWidth(); j++) {
				Uint8* pixel = pixels;
				
				gray = pixel[rbyte] * 0.299 + pixel[gbyte] * 0.587 + pixel[bbyte] * 0.114;
				pixel[rbyte] = (Uint8)max(min((pixel[rbyte] - gray) * factor + gray + tone.red + 0.5, 255), 0);
				pixel[gbyte] = (Uint8)max(min((pixel[gbyte] - gray) * factor + gray + tone.green + 0.5, 255), 0);
				pixel[bbyte] = (Uint8)max(min((pixel[bbyte] - gray) * factor + gray + tone.blue + 0.5, 255), 0);
				pixels += 4;
			}
		}
	}
	SDL_UnlockSurface(bitmap);
}

////////////////////////////////////////////////////////////
/// Opacity change
////////////////////////////////////////////////////////////
void Bitmap::OpacityChange(int opacity, int bush_depth) {
	if (opacity >= 255 && bush_depth <= 0) return;

	SDL_LockSurface(bitmap);
	
	Uint8* pixels = (Uint8*)bitmap->pixels;
	
	int start_bush = max(GetHeight() - bush_depth, 0);

	const int abyte = MaskGetByte(bitmap->format->Amask);
	
	if (opacity < 255) {
		for (int i = 0; i < start_bush; i++) {
			for (int j = 0; j < GetWidth(); j++) {
				Uint8* pixel = pixels;
				pixel[abyte] = pixel[abyte] * opacity / 255;
				pixels += 4;
			}
		}
		for (int i = start_bush; i< GetHeight(); i++) {
			for (int j = 0; j < GetWidth(); j++) {
				Uint8* pixel = pixels;
				pixel[abyte] = (pixel[abyte] / 2) * opacity / 255;
				pixels += 4;
			}
		}
	} else {
		pixels += start_bush * GetWidth() * 4;
		for (int i = start_bush; i < GetHeight(); i++) {
			for (int j = 0; j < GetWidth(); j++) {
				Uint8* pixel = pixels;
				pixel[abyte] = pixel[abyte] / 2;
				pixels += 4;
			}
		}
	}
	SDL_UnlockSurface(bitmap);
}

////////////////////////////////////////////////////////////
/// Flip
////////////////////////////////////////////////////////////
void Bitmap::Flip(bool flipx, bool flipy) {
	if (!(flipx || flipy)) return;

	SDL_LockSurface(bitmap);
	
	Bitmap* temp_bmp = new Bitmap(GetWidth(), GetHeight());
	SDL_Surface* temp = temp_bmp->bitmap;

	SDL_LockSurface(temp);
	
	Uint32* srcpixels = (Uint32*)bitmap->pixels;
	Uint32* dstpixels = (Uint32*)temp->pixels;
	
	if (flipx && flipy) {
		long srcpixel = 0;
		long dstpixel = GetWidth() + (GetHeight() - 1) * GetWidth() - 1;
		for (int i = 0; i < GetHeight(); i++) {
			for (int j = 0; j < GetWidth(); j++) {
				dstpixels[dstpixel] = srcpixels[srcpixel];
				
				srcpixel += 1;
				dstpixel -= 1;
			}
		}
	} else if (flipx) {
		long srcpixel = 0;
		long dstpixel = GetWidth() - 1;
		for (int i = 0; i < GetHeight(); i++) {
			for (int j = 0; j < GetWidth(); j++) {
				dstpixels[dstpixel] = srcpixels[srcpixel];
				
				srcpixel += 1;
				dstpixel -= 1;
			}
			dstpixel += GetWidth() * 2;
		}
	} else if (flipy) {
		dstpixels += (GetHeight() - 1) * GetWidth();
		for (int i = 0; i < GetHeight(); i++) {
			memcpy(dstpixels, srcpixels, GetWidth() * 4);
			srcpixels += GetWidth();
			dstpixels -= GetWidth();
		}
	}
	
	SDL_UnlockSurface(bitmap);
	SDL_UnlockSurface(temp);
	
	SDL_FreeSurface(bitmap);

	bitmap = temp;

	temp_bmp->bitmap = NULL;
	delete temp_bmp;
}

////////////////////////////////////////////////////////////
/// Zoom
////////////////////////////////////////////////////////////
void Bitmap::Zoom(double zoom_x, double zoom_y) {
	if (zoom_x == 1.0 && zoom_y == 1.0) return;

	SDL_LockSurface(bitmap);
	
	int scalew = (int)(GetWidth() * zoom_x);
	int scaleh = (int)(GetHeight() * zoom_y);
	
	Bitmap* nbitmap = new Bitmap(scalew, scaleh);
	SDL_Surface* surface = nbitmap->bitmap;
		
	SDL_LockSurface(surface);
	
	Uint8* srcpixels = (Uint8*)bitmap->pixels;
	Uint8* dstpixels = (Uint8*)surface->pixels;

	int row = GetWidth() * 4;

	for (int yy = 0; yy < surface->h; yy++) {
		int nearest_matchy = (int)(yy / zoom_y) * row;
		for (int xx = 0; xx < surface->w; xx++) {
			int nearest_match = nearest_matchy + (int)(xx / zoom_x) * 4;
			dstpixels[0] = srcpixels[nearest_match];
			dstpixels[1] = srcpixels[nearest_match + 1];
			dstpixels[2] = srcpixels[nearest_match + 2];
			dstpixels[3] = srcpixels[nearest_match + 3];
			dstpixels += 4;
		}
	}
	
	SDL_UnlockSurface(surface);
	SDL_FreeSurface(bitmap);
	
	bitmap = surface;

	nbitmap->bitmap = NULL;
	delete nbitmap;
}

////////////////////////////////////////////////////////////
/// Resample
////////////////////////////////////////////////////////////
Bitmap* Bitmap::Resample(int scalew, int scaleh, Rect src_rect) {
	double zoom_x = (double)(scalew) / src_rect.width;
	double zoom_y = (double)(scaleh) / src_rect.height;

	Bitmap* resampled = new Bitmap(scalew, scaleh);
	SDL_Surface* surface = resampled->bitmap;

	SDL_LockSurface(bitmap);
	SDL_LockSurface(surface);
	
	Uint8* srcpixels = (Uint8*)bitmap->pixels;
	Uint8* dstpixels = (Uint8*)surface->pixels;

	int row = GetWidth() * 4;

	for (int yy = 0; yy < scaleh; yy++) {
		int nearest_matchy = (src_rect.y + (int)(yy / zoom_y)) * row;
		for (int xx = 0; xx < scalew; xx++) {
			int nearest_match = nearest_matchy + (src_rect.x + (int)(xx / zoom_x)) * 4;
			dstpixels[0] = srcpixels[nearest_match];
			dstpixels[1] = srcpixels[nearest_match + 1];
			dstpixels[2] = srcpixels[nearest_match + 2];
			dstpixels[3] = srcpixels[nearest_match + 3];
			dstpixels += 4;
		}
	}
	
	SDL_UnlockSurface(bitmap);
	SDL_UnlockSurface(surface);

	return resampled;
}

////////////////////////////////////////////////////////////
/// Rotate
////////////////////////////////////////////////////////////
void Bitmap::Rotate(double angle) {
	/*if (angle % 360 == 0) return;

	SDL_LockSurface(bitmap);
	
	float radians = (2 * 3.14159265 * angle) / 360;

	float cosine = (float)cos(radians);
	float sine = (float)sin(radians);

	float p1x = -GetHeight() * sine;
	float p1y = GetHeight() * cosine;
	float p2x = GetWidth() * cosine - GetHeight() * sine;
	float p2y = GetHeight() * cosine + GetWidth() * sine;
	float p3x = GetWidth() * cosine;
	float p3y = GetWidth() * sine;

	float minx = min(0, min(p1x, min(p2x, p3x)));
	float miny = min(0, min(p1y, min(p2y, p3y)));
	float maxx = max(p1x, max(p2x, p3x));
	float maxy = max(p1y, max(p2y, p3y));

	int nwidth = (int)ceil(fabs(maxx)-minx);
	int nheight = (int)ceil(fabs(maxy)-miny);
	
	SDL_Surface* nbitmap = SDL_CreateRGBASurface(nwidth, nheight);
	
	SDL_LockSurface(nbitmap);
	
	Uint8* srcpixels = (Uint8*)bitmap->pixels;
	Uint8* dstpixels = (Uint8*)nbitmap->pixels;

	int row = GetWidth() * 4;

	for (int yy = 0; yy < nbitmap->h; yy++) {
		int nearest_matchy = (int)(yy / zoom_y) * row;
		for (int xx = 0; xx < nbitmap->w; xx++) {
			int nearest_match = nearest_matchy + (int)(xx / zoom_x) * 4;
			dstpixels[0] = srcpixels[nearest_match];
			dstpixels[1] = srcpixels[nearest_match + 1];
			dstpixels[2] = srcpixels[nearest_match + 2];
			dstpixels[3] = srcpixels[nearest_match + 3];
			dstpixels += 4;
		}
	}
	
	SDL_UnlockSurface(nbitmap);
	SDL_FreeSurface(bitmap);
	
	bitmap = nbitmap;*/
}

////////////////////////////////////////////////////////////
/// Flash
////////////////////////////////////////////////////////////
void Bitmap::Flash(Color color, int frame, int duration) {
	// TODO
}

////////////////////////////////////////////////////////////
/// Properties
////////////////////////////////////////////////////////////
Rect Bitmap::GetRect() const {
	Rect rect(0, 0, GetWidth(), GetHeight());
	return rect;
}
Font* Bitmap::GetFont() {
	return &font;
}
void Bitmap::SetFont(Font nfont) {
	font = nfont;
}

////////////////////////////////////////////////////////////
/// Static MaskGetByte
////////////////////////////////////////////////////////////
int Bitmap::MaskGetByte(Uint32 mask) {
	switch (mask) {
	case 0xFF000000:
		return 3;
	case 0x00FF0000:
		return 2;
	case 0x0000FF00:
		return 1;
	default:
		return 0;
	}
}
