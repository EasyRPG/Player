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

////////////////////////////////////////////////////////////
/// Defines
////////////////////////////////////////////////////////////
#define min(a, b)	(((a) < (b)) ? (a) : (b))
#define max(a, b)	(((a) > (b)) ? (a) : (b))

#define rmask 0xff000000
#define gmask 0x00ff0000
#define bmask 0x0000ff00
#define amask 0x000000ff

////////////////////////////////////////////////////////////
/// Constructors
////////////////////////////////////////////////////////////
Bitmap::Bitmap(int width, int height) {
	SDL_Surface* temp;
	temp = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, width, height, 32, rmask, gmask, bmask, amask);
	if (temp == NULL) {
		Output::Error("Couldn't create %dx%d image.\n%s\n", width, height, SDL_GetError());
	}
	bitmap = SDL_DisplayFormatAlpha(temp);
	if (bitmap == NULL) {
		Output::Error("Couldn't optimize %dx%d image.\n%s\n", width, height, SDL_GetError());
	}
	SDL_FreeSurface(temp);
}
Bitmap::Bitmap(std::string filename) {
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
	if (temp->format->BitsPerPixel == 8) {
		SDL_Color colorkey = temp->format->palette->colors[0];
		col.red = colorkey.r;
		col.green = colorkey.g;
		col.blue = colorkey.b;
	}
	bitmap = SDL_DisplayFormatAlpha(temp);
	if (bitmap == NULL) {
		Output::Error("Couldn't optimize %s image.\n%s\n", filename.c_str(), SDL_GetError());
	}
	if (temp->format->BitsPerPixel == 8) {
		SetTransparent(col);
	}
	SDL_FreeSurface(temp);
}
Bitmap::Bitmap(Bitmap* source, Rect src_rect) {
	SDL_Surface* temp;
	temp = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, src_rect.width, src_rect.height, 32, rmask, gmask, bmask, amask);
	if (temp == NULL) {
		Output::Error("Couldn't create %dx%d image.\n%s\n", src_rect.width, src_rect.height, SDL_GetError());
	}
	bitmap = SDL_DisplayFormatAlpha(temp);
	if (bitmap == NULL) {
		Output::Error("Couldn't optimize %dx%d image.\n%s\n", src_rect.width, src_rect.height, SDL_GetError());
	}
	SDL_FreeSurface(temp);
	Blit(0, 0, source, src_rect, 255);
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
	
	if (opacity >= 255) {
		SDL_Rect offset = {x, y, 0, 0};
		SDL_Rect src_rect_sdl = src_rect.Get();
		SDL_BlitSurface(bitmap, &src_rect_sdl, Player::main_window, &offset);
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

////////////////////////////////////////////////////////////
/// Fill rect
////////////////////////////////////////////////////////////
void Bitmap::FillRect(Rect rect, Color color) {
	rect.Adjust(GetWidth(), GetHeight());
	if (rect.IsOutOfBounds(GetWidth(), GetHeight())) return;
	
	SDL_Rect rect_sdl = rect.Get();
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
	Uint32* pixels = (Uint32*)bitmap->pixels;
	return Color(pixels[(y * GetWidth()) + x], bitmap->format);
}

////////////////////////////////////////////////////////////
/// Set pixel
////////////////////////////////////////////////////////////
void Bitmap::SetPixel(int x, int y, Color color) {
	if (x < 0 || y < 0) return;
	if (x >= GetWidth() || y > GetHeight()) return;
	Uint32* pixels = (Uint32*)bitmap->pixels;
	pixels[(y * GetWidth()) + x] = color.GetUint32(bitmap->format);
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

	const int rbyte = MaskGetByte(bitmap->format->Rmask);
	const int gbyte = MaskGetByte(bitmap->format->Gmask);
	const int bbyte = MaskGetByte(bitmap->format->Bmask);
	const int abyte = MaskGetByte(bitmap->format->Amask);
	
	TTF_Font* ttf_font = font.GetTTF();
	int style = 0;
	if (font.bold) style |= TTF_STYLE_BOLD;
	if (font.italic) style |= TTF_STYLE_ITALIC;
	TTF_SetFontStyle(ttf_font, style);

	SDL_Surface* text_surface;

#if FONT_HINTING == 0
	text_surface = TTF_RenderUTF8_Solid(ttf_font, text.c_str(), font.color.Get());
#else
	text_surface = TTF_RenderUTF8_Blended(ttf_font, text.c_str(), font.color.Get());
#endif

	if (!text_surface) {
		Output::Error("Couldn't draw text %s with Font %n size %d.\n%s\n", text.c_str(), font.name.c_str(), font.size, TTF_GetError());
	}

#if FONT_HINTING == 0
	// Convert to 32bit surface and get the colorkey in an endianess-neutral way
	SDL_Surface* text_surface32 = SDL_ConvertSurface(text_surface, bitmap->format, text_surface->flags);
	Uint32 colorkey = *(Uint32*)text_surface->format->palette->colors | (0xFF << abyte*8);
	SDL_FreeSurface(text_surface);
	text_surface = text_surface32;
#endif

	Bitmap* text_bmp = new Bitmap(1, 1);
	SDL_FreeSurface(text_bmp->bitmap);
	text_bmp->bitmap = text_surface;

	if (text_bmp->GetWidth() > rect.width) {
		int stretch = (int)(text_bmp->GetWidth() * 0.4);
		if (rect.width > stretch) stretch = rect.width;
		Rect resample_rect(0, 0, text_bmp->GetWidth(), text_bmp->GetHeight());
		Bitmap* resampled = text_bmp->Resample(stretch, text_bmp->GetHeight(), resample_rect);
		delete text_bmp;
		text_bmp = resampled;
	}
	Rect src_rect(0, 0, rect.width, rect.height);
	int y = rect.y;
	if (rect.height > text_bmp->GetHeight()) y += ((rect.height - text_bmp->GetHeight()) / 2);
	int x = rect.x;
	if (rect.width > text_bmp->GetWidth()) {
		if (align == 1) {
			x += (rect.width - text_bmp->GetWidth()) / 2;
		} else if (align == 2) {
			x += rect.width - text_bmp->GetWidth();
		}
	}

	// Surface for drop Shadow (HARDCODED to black)
	Bitmap* text_bmp_shadow = new Bitmap(1, 1);
	SDL_FreeSurface(text_bmp_shadow->bitmap);
	SDL_Surface* text_shadow = SDL_ConvertSurface(text_bmp->bitmap, text_bmp->bitmap->format, text_bmp->bitmap->flags);
	text_bmp_shadow->bitmap = text_shadow;

	// Experimental Color Blending
	// Looks strange with font smoothing
	SDL_LockSurface(text_bmp->bitmap);
	SDL_LockSurface(text_bmp_shadow->bitmap);
	Uint8* pixels = (Uint8*)text_bmp->bitmap->pixels;
	Uint8* shadow_pixels = (Uint8*)text_shadow->pixels;

	// HARDCODED: Should be changed to the correct color value later!
	//            Currently always the first one is used!
	Bitmap* system = Cache::System(Main_Data::data_system.system_name);

	for (int i = 0; i < text_bmp->bitmap->h; ++i) {
		for (int j = 0; j < text_bmp->bitmap->w; ++j, pixels+=4, shadow_pixels+=4) {
			Color drawColor = system->GetPixel((j%7) + 1, 52+i+1);
			
			// HACK: Only works for half size glypths
			// Skip every 7th row, this one is always empty?
			if (j % 7 == 0) {
				pixels[abyte] = 0;
				shadow_pixels[abyte] = 0;
				continue;
			}

#if FONT_HINTING == 0
			// Make everything matching the colorkey transparent
			if (*(Uint32*)pixels == colorkey) {
				pixels[abyte] = 0;
				shadow_pixels[abyte] = 0;
				continue;
			}
#endif

			pixels[rbyte] = drawColor.red;
			pixels[gbyte] = drawColor.green;
			pixels[bbyte] = drawColor.blue;

			shadow_pixels[rbyte] = 0;
			shadow_pixels[bbyte] = 0;
			shadow_pixels[gbyte] = 0;
		}
	}
	SDL_UnlockSurface(text_bmp_shadow->bitmap);
	SDL_UnlockSurface(text_bmp->bitmap);
	// End of Color blending

	Blit(x+1, y+1, text_bmp_shadow, src_rect, font.color.alpha);
	Blit(x, y, text_bmp, src_rect, font.color.alpha);

	delete text_bmp;
	delete text_bmp_shadow;
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
