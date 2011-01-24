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
#include <cmath>
#include <cstring>
#include <algorithm>
#include "utils.h"
#include "bitmap.h"
#include "bitmap_screen.h"
#include "text.h"

#if defined(USE_SDL_BITMAP)
	#include "sdl_bitmap.h"
#endif
#if defined(USE_SOFT_BITMAP)
	#include "soft_bitmap.h"
#endif
#if defined(USE_PIXMAN_BITMAP)
	#include "pixman_bitmap.h"
#endif
#if defined(USE_OPENGL)
	#include "gl_bitmap.h"
#endif

#include "util_macro.h"

////////////////////////////////////////////////////////////
static int GetMaskByte(uint32 mask) {
	const uint8* bytes = (const uint8*) &mask;
	if (bytes[3] == 0xFF)
		return 3;
	if (bytes[2] == 0xFF)
		return 2;
	if (bytes[1] == 0xFF)
		return 1;
	if (bytes[0] == 0xFF)
		return 0;
	return 0;
}

////////////////////////////////////////////////////////////
Bitmap* Bitmap::CreateBitmap(int width, int height, bool transparent) {
	#if defined(USE_SDL_BITMAP)
		return (Bitmap*)new SdlBitmap(width, height, transparent);
	#elif defined(USE_SOFT_BITMAP)
		return (Bitmap*)new SoftBitmap(width, height, transparent);
	#elif defined(USE_PIXMAN_BITMAP)
		return (Bitmap*)new PixmanBitmap(width, height, transparent);
	#elif defined(USE_OPENGL_BITMAP)
		return (Bitmap*)new GlBitmap(width, height, transparent);
	#else
		#error "No bitmap implementation selected"
	#endif
}

Bitmap* Bitmap::CreateBitmap(const std::string& filename, bool transparent, bool read_only) {
	#if defined(USE_SDL_BITMAP)
		return (Bitmap*)new SdlBitmap(filename, transparent);
	#elif defined(USE_SOFT_BITMAP)
		return (Bitmap*)new SoftBitmap(filename, transparent);
	#elif defined(USE_PIXMAN_BITMAP)
		return (Bitmap*)new PixmanBitmap(filename, transparent);
	#elif defined(USE_OPENGL_BITMAP)
		return (Bitmap*)new GlBitmap(filename, transparent);
	#else
		#error "No bitmap implementation selected"
	#endif
}

Bitmap* Bitmap::CreateBitmap(const uint8* data, uint bytes, bool transparent) {
	#if defined(USE_SDL_BITMAP)
		return (Bitmap*)new SdlBitmap(data, bytes, transparent);
	#elif defined(USE_SOFT_BITMAP)
		return (Bitmap*)new SoftBitmap(data, bytes, transparent);
	#elif defined(USE_PIXMAN_BITMAP)
		return (Bitmap*)new PixmanBitmap(data, bytes, transparent);
	#elif defined(USE_OPENGL_BITMAP)
		return (Bitmap*)new GlBitmap(data, bytes, transparent);
	#else
		#error "No bitmap implementation selected"
	#endif
}

Bitmap* Bitmap::CreateBitmap(Bitmap* source, Rect src_rect, bool transparent) {
	#if defined(USE_SDL_BITMAP)
		return (Bitmap*)new SdlBitmap(source, src_rect, transparent);
	#elif defined(USE_SOFT_BITMAP)
		return (Bitmap*)new SoftBitmap(source, src_rect, transparent);
	#elif defined(USE_PIXMAN_BITMAP)
		return (Bitmap*)new PixmanBitmap(source, src_rect, transparent);
	#elif defined(USE_OPENGL_BITMAP)
		return (Bitmap*)new GlBitmap(source, src_rect, transparent);
	#else
		#error "No bitmap implementation selected"
	#endif
}

////////////////////////////////////////////////////////////
Bitmap::Bitmap() :
	editing(false) {

	font = Font::CreateFont();
}

////////////////////////////////////////////////////////////
Bitmap::~Bitmap() {
	delete font;
}

////////////////////////////////////////////////////////////
void Bitmap::Blit(int x, int y, Bitmap* src, Rect src_rect, int opacity) {
	if (opacity < 0) return;

	Rect dst_rect(x, y, 0, 0);

	if (!Rect::AdjustRectangles(src_rect, dst_rect, src->GetRect()))
		return;
	if (!Rect::AdjustRectangles(dst_rect, src_rect, GetRect()))
		return;

	if (opacity > 255) opacity = 255;

	Lock();
	src->Lock();

	if (bpp() == 2) {
		const uint16* src_pixels = (uint16*)src->pixels() + src_rect.x + src_rect.y * src->pitch() / bpp();
		uint16* dst_pixels = (uint16*)pixels() + x + y * pitch() / bpp();

		#ifdef USE_ALPHA
			int src_stride = src->pitch() / bpp() - dst_rect.width;
			int dst_stride = pitch() / bpp() - dst_rect.width;

			uint8 src_r, src_g, src_b, src_a;
			uint8 dst_r, dst_g, dst_b, dst_a;

			for (int i = 0; i < dst_rect.height; i++) {
				for (int j = 0; j < dst_rect.width; j++) {
					GetColorComponents(src_pixels[0], src_r, src_g, src_b, src_a);
					GetColorComponents(dst_pixels[0], dst_r, dst_g, dst_b, dst_a);

					uint8 alpha = src_a * opacity / 255;

					dst_r = (dst_r * (255 - alpha) + src_r * alpha) / 255;
					dst_g = (dst_g * (255 - alpha) + src_g * alpha) / 255;
					dst_b = (dst_b * (255 - alpha) + src_b * alpha) / 255;
					dst_a = dst_a * (255 - alpha) / 255 + alpha;

					dst_pixels[0] = (uint16)GetUint32Color(dst_r, dst_g, dst_b, dst_a);

					src_pixels += 1;
					dst_pixels += 1;
				}
				src_pixels += src_stride;
				dst_pixels += dst_stride;
			}
		#else
			if (opacity < 255) {
				int src_stride = src->pitch() / bpp() - dst_rect.width;
				int dst_stride = pitch() / bpp() - dst_rect.width;

				uint8 src_r, src_g, src_b, src_a;
				uint8 dst_r, dst_g, dst_b, dst_a;

				for (int i = 0; i < dst_rect.height; i++) {
					for (int j = 0; j < dst_rect.width; j++) {
						if (transparent && src_pixels[0] == (uint16)colorkey()) continue;

						GetColorComponents(src_pixels[0], src_r, src_g, src_b, src_a);
						GetColorComponents(dst_pixels[0], dst_r, dst_g, dst_b, dst_a);

						dst_r = (uint8)((dst_r * (255 - opacity) + src_r * opacity) / 255);
						dst_g = (uint8)((dst_g * (255 - opacity) + src_g * opacity) / 255);
						dst_b = (uint8)((dst_b * (255 - opacity) + src_b * opacity) / 255);
						dst_a = (uint8)(dst_a * (255 - opacity) / 255 + opacity);

						dst_pixels[0] = (uint16)GetUint32Color(dst_r, dst_g, dst_b, dst_a);

						src_pixels += 1;
						dst_pixels += 1;
					}
					src_pixels += src_stride;
					dst_pixels += dst_stride;
				}
			} else if (transparent) {
				int src_stride = src->pitch() / bpp() - dst_rect.width;
				int dst_stride = pitch() / bpp() - dst_rect.width;

				for (int i = 0; i < dst_rect.height; i++) {
					for (int j = 0; j < dst_rect.width; j++) {
						if (transparent && src_pixels[0] == (uint16)colorkey()) continue;

						dst_pixels[0] = src_pixels[0];

						src_pixels += 1;
						dst_pixels += 1;
					}
					src_pixels += src_stride;
					dst_pixels += dst_stride;
				}
			} else {
				int stride = dst_rect.width * bpp();

				for (int i = 0; i < dst_rect.height; i++) {
					memcpy(dst_pixels, src_pixels, stride);

					src_pixels += src->pitch() / bpp();
					dst_pixels += pitch() / bpp();
				}
			}
		#endif
	} else if (bpp() == 4) {
		const int rbyte = GetMaskByte(rmask());
		const int gbyte = GetMaskByte(gmask());
		const int bbyte = GetMaskByte(bmask());

		#ifdef USE_ALPHA
			const int abyte = GetMaskByte(amask());

			const uint8* src_pixels = (uint8*)src->pixels() + src_rect.x * bpp() + src_rect.y * src->pitch();
			uint8* dst_pixels = (uint8*)pixels() + x * bpp() + y * pitch();

			int src_stride = src->pitch() - dst_rect.width * bpp();
			int dst_stride = pitch() - dst_rect.width * bpp();

			for (int i = 0; i < dst_rect.height; i++) {
				for (int j = 0; j < dst_rect.width; j++) {
					uint8 srca = src_pixels[abyte] * opacity / 255;

					dst_pixels[rbyte] = (dst_pixels[rbyte] * (255 - srca) + src_pixels[rbyte] * srca) / 255;
					dst_pixels[gbyte] = (dst_pixels[gbyte] * (255 - srca) + src_pixels[gbyte] * srca) / 255;
					dst_pixels[bbyte] = (dst_pixels[bbyte] * (255 - srca) + src_pixels[bbyte] * srca) / 255;
					dst_pixels[abyte] = dst_pixels[abyte] * (255 - srca) / 255 + srca;

					src_pixels += bpp();
					dst_pixels += bpp();
				}
				src_pixels += src_stride;
				dst_pixels += dst_stride;
			}
		#else
			if (opacity < 255) {
				const uint8* src_pixels = (uint8*)src->pixels() + src_rect.x * bpp() + src_rect.y * src->pitch();
				uint8* dst_pixels = (uint8*)pixels() + x * bpp() + y * pitch();

				int src_stride = src->pitch() - dst_rect.width * bpp();
				int dst_stride = pitch() - dst_rect.width * bpp();

				for (int i = 0; i < dst_rect.height; i++) {
					for (int j = 0; j < dst_rect.width; j++) {
						if (transparent && ((uint32*)dst_pixels)[0] == colorkey()) continue;

						dst_pixels[rbyte] = (uint8)((dst_pixels[rbyte] * (255 - opacity) + src_pixels[rbyte] * opacity) / 255);
						dst_pixels[gbyte] = (uint8)((dst_pixels[gbyte] * (255 - opacity) + src_pixels[gbyte] * opacity) / 255);
						dst_pixels[bbyte] = (uint8)((dst_pixels[bbyte] * (255 - opacity) + src_pixels[bbyte] * opacity) / 255);

						src_pixels += bpp();
						dst_pixels += bpp();
					}
					src_pixels += src_stride;
					dst_pixels += dst_stride;
				}
			} else if (transparent) {
				const uint32* src_pixels = (uint32*)src->pixels() + src_rect.x + src_rect.y * src->pitch() / bpp();
				uint32* dst_pixels = (uint32*)pixels() + x + y * pitch() / bpp();

				int src_stride = src->pitch() / bpp() - dst_rect.width;
				int dst_stride = pitch() / bpp() - dst_rect.width;
				
				for (int i = 0; i < dst_rect.height; i++) {
					for (int j = 0; j < dst_rect.width; j++) {
						if (transparent && src_pixels[0] == colorkey()) continue;

						dst_pixels[0] = src_pixels[0];

						src_pixels += 1;
						dst_pixels += 1;
					}
					src_pixels += src_stride;
					dst_pixels += dst_stride;
				}
			} else {
				const uint8* src_pixels = (uint8*)src->pixels() + src_rect.x * bpp() + src_rect.y * src->pitch();
				uint8* dst_pixels = (uint8*)pixels() + x * bpp() + y * pitch();

				int stride = dst_rect.width * bpp();

				for (int i = 0; i < dst_rect.height; i++) {
					memcpy(dst_pixels, src_pixels, stride);

					src_pixels += src->pitch();
					dst_pixels += pitch();
				}
			}
		#endif
	}

	Unlock();
	src->Unlock();

	RefreshCallback();
}

////////////////////////////////////////////////////////////
void Bitmap::TiledBlit(Rect src_rect, Bitmap* src, Rect dst_rect, int opacity) {
	int y_blits = 1;
	if (src_rect.height < dst_rect.height && src_rect.height != 0) {
		y_blits = (int)ceil(dst_rect.height / (float)src_rect.height);
	}

	int x_blits = 1;
	if (src_rect.width < dst_rect.width && src_rect.width != 0) {
		x_blits = (int)ceil(dst_rect.width / (float)src_rect.width);
	}

	Rect tile = src_rect;

	for (int j = 0; j < y_blits; j++) {
		tile.height = min(src_rect.height, dst_rect.height + dst_rect.y - j * src_rect.height);
		for (int i = 0; i < x_blits; i++) {
			tile.width = min(src_rect.width, dst_rect.width + dst_rect.x - i * src_rect.width);
			Blit(dst_rect.x + i * src_rect.width, dst_rect.y + j * src_rect.height, src, tile, opacity);
		}
	}
}


////////////////////////////////////////////////////////////
void Bitmap::TiledBlit(int ox, int oy, Rect src_rect, Bitmap* src, Rect dst_rect, int opacity) {
	while (ox >= src_rect.width) ox -= src_rect.width;
	while (oy >= src_rect.height) ox -= src_rect.height;
	while (ox < 0) ox += src_rect.width;
	while (oy < 0) ox += src_rect.height;

	int y_blits = 1;
	if (src_rect.height - oy < dst_rect.height && src_rect.height != 0) {
		y_blits = (int)ceil((dst_rect.height + oy) / (float)src_rect.height);
	}

	int x_blits = 1;
	if (src_rect.width - ox < dst_rect.width && src_rect.width != 0) {
		x_blits = (int)ceil((dst_rect.width + ox) / (float)src_rect.width);
	}

	Rect tile;
	int dst_x, dst_y;

	for (int j = 0; j < y_blits; j++) {
		dst_y = dst_rect.y + j * src_rect.height;

		if (j == 0) {
			tile.y = src_rect.y + oy;
			tile.height = src_rect.height - oy;
		} else {
			dst_y -= oy;
			tile.y = src_rect.y;
			tile.height = src_rect.height;
		}

		tile.height = min(tile.height, dst_rect.y + dst_rect.height - dst_y);

		for (int i = 0; i < x_blits; i++) {
			dst_x = dst_rect.x + i * src_rect.width;

			if (i == 0) {
				tile.x = src_rect.x + ox;
				tile.width = src_rect.width - ox;
			} else {
				dst_x -= ox;
				tile.x = src_rect.x;
				tile.width = src_rect.width;
			}

			tile.width = min(tile.width, dst_rect.x + dst_rect.width - dst_x);

			Blit(dst_x, dst_y, src, tile, opacity);
		}
	}
}

////////////////////////////////////////////////////////////
void Bitmap::StretchBlit(Bitmap* src, Rect src_rect, int opacity) {
	if (src_rect.width == width() && src_rect.height == height()) {
		Blit(0, 0, src, src_rect, opacity);
	} else {
		StretchBlit(GetRect(), src, src_rect, opacity);
	}
}

////////////////////////////////////////////////////////////
void Bitmap::StretchBlit(Rect dst_rect, Bitmap* src, Rect src_rect, int opacity) {
	if (src_rect.width == dst_rect.width && src_rect.height == dst_rect.height) {
		Blit(dst_rect.x, dst_rect.y, src, src_rect, opacity);
	} else {
		src_rect.Adjust(src->width(), src->height());
		if (src_rect.IsOutOfBounds(src->width(), src->height())) return;

		if (dst_rect.IsOutOfBounds(width(), height())) return;

		Bitmap* resampled = src->Resample(dst_rect.width, dst_rect.height, src_rect);

		Blit(dst_rect.x, dst_rect.y, resampled, resampled->GetRect(), opacity);

		delete resampled;
	}
}

////////////////////////////////////////////////////////////
void Bitmap::Mask(int x, int y, Bitmap* src, Rect src_rect) {
	Lock();
	src->Lock();

	if (bpp() == 2) {
		for (int j = 0; j < src_rect.height; j++) {
			uint16* src_pixels = (uint16*) src->pixels() + (src_rect.y + j) * src->pitch() / 2 + src_rect.x;
			uint16* dst_pixels = (uint16*) pixels() + (y + j) * pitch() / 2 + x;
			for (int i = 0; i < src_rect.width; i++) {
				uint32 src_pix = (uint32) *src_pixels;
				uint8 src_r, src_g, src_b, src_a;
				GetColorComponents(src_pix, src_r, src_g, src_b, src_a);

				uint32 dst_pix = (uint32) *dst_pixels;
				uint8 dst_r, dst_g, dst_b, dst_a;
				GetColorComponents(src_pix, dst_r, dst_g, dst_b, dst_a);

				dst_pix = GetUint32Color(dst_r, dst_g, dst_b, src_a);

				src_pixels++;
				*dst_pixels++ = dst_pix;
			}
		}
	} else if (bpp() == 4) {
		#ifdef USE_ALPHA
		const int src_abyte = GetMaskByte(src->amask());
		const int dst_abyte = GetMaskByte(amask());
		const int src_bpp = src->bpp();
		const int dst_bpp = bpp();

		for (int j = 0; j < src_rect.height; j++) {
			uint8* src_pixels = (uint8*) src->pixels() + (src_rect.y + j) * src->pitch() + src_rect.x * src_bpp;
			uint8* dst_pixels = (uint8*) pixels() + (y + j) * pitch() + x * dst_bpp;
			for (int i = 0; i < src_rect.width; i++) {
				dst_pixels[dst_abyte] = src_pixels[src_abyte];
				src_pixels += src_bpp;
				dst_pixels += dst_bpp;
			}
		}
		#else
		const uint32 src_trans = src->colorkey();
		const uint32 dst_trans = colorkey();

		for (int j = 0; j < src_rect.height; j++) {
			uint32* src_pixels = (uint32*) src->pixels() + (src_rect.y + j) * src->pitch() / src->bpp() + src_rect.x;
			uint32* dst_pixels = (uint32*) pixels() + (y + j) * pitch() / bpp() + x;
			for (int i = 0; i < src_rect.width; i++) {
				if (*src_pixels == src_trans)
					*dst_pixels = dst_trans;
				src_pixels++;
				dst_pixels++;
			}
		}
		#endif
	}

	src->Unlock();
	Unlock();

	RefreshCallback();
}

////////////////////////////////////////////////////////////
void Bitmap::Fill(const Color &color) {
	Lock();

	if (bpp() == 2) {
		uint16 pixel = (uint16)GetUint32Color(color);

		uint16* dst_pixels = (uint16*)pixels();

		if (pitch() == width()) {
			std::fill(dst_pixels, dst_pixels + height() * pitch(), pixel);
		} else {
			for (int i = 0; i < height(); i++) {
				std::fill(dst_pixels, dst_pixels + width(), pixel);
				dst_pixels += width();
			}
		}
	} else if (bpp() == 4) {
		uint32 pixel = GetUint32Color(color);

		uint32* dst_pixels = (uint32*)pixels();

		if (pitch() == width()) {
			std::fill(dst_pixels, dst_pixels + height() * pitch(), pixel);
		} else {
			for (int i = 0; i < height(); i++) {
				std::fill(dst_pixels, dst_pixels + width(), pixel);
				dst_pixels += width();
			}
		}
	}

	Unlock();

	RefreshCallback();
}

////////////////////////////////////////////////////////////
void Bitmap::FillRect(Rect dst_rect, const Color &color) {
	dst_rect.Adjust(width(), height());
	if (dst_rect.IsOutOfBounds(width(), height())) return;

	Lock();

	if (bpp() == 2) {
		uint16 pixel = (uint16)GetUint32Color(color);

		uint16* dst_pixels = (uint16*)pixels() + dst_rect.y * pitch() / bpp() + dst_rect.x;

		for (int i = 0; i < dst_rect.height; i++) {
			std::fill(dst_pixels, dst_pixels + dst_rect.width, pixel);
			dst_pixels += pitch() / bpp();
		}
	} else if (bpp() == 4) {
		uint32 pixel = GetUint32Color(color);

		uint32* dst_pixels = (uint32*)pixels() + dst_rect.y * pitch() / bpp() + dst_rect.x;

		for (int i = 0; i < dst_rect.height; i++) {
			std::fill(dst_pixels, dst_pixels + dst_rect.width, pixel);
			dst_pixels += pitch() / bpp();
		}
	}

	Unlock();

	RefreshCallback();
}

////////////////////////////////////////////////////////////
void Bitmap::Clear() {
	Fill(GetTransparentColor());
}

////////////////////////////////////////////////////////////
void Bitmap::ClearRect(Rect dst_rect) {
	FillRect(dst_rect, GetTransparentColor());
}

////////////////////////////////////////////////////////////
Color Bitmap::GetPixel(int x, int y) {
	if (x < 0 || y < 0 || x >= width() || y >= height()) return Color();

	uint32 pixel = 0;

	Lock();

	if (bpp() == 2) {
		uint16* src_pixel = (uint16*)pixels() + x + y * (pitch() / bpp());
		pixel = src_pixel[0];
	} else if (bpp() == 4) {
		uint32* src_pixel = (uint32*)pixels() + x + y * (pitch() / bpp());
		pixel = src_pixel[0];
	}

	Unlock();

	Color color = GetColor(pixel);

	#ifndef USE_ALPHA
		if (transparent && pixel == colorkey()) {
			color.alpha = 0;
		} else {
			color.alpha = 255;
		}
	#endif

	return color;
}

////////////////////////////////////////////////////////////
void Bitmap::SetPixel(int x, int y, const Color &color) {
	if (x < 0 || y < 0 || x >= width() || y >= height()) return;

	Lock();

	if (bpp() == 2) {
		uint16* dst_pixel = (uint16*)pixels() + x + y * pitch() / bpp();
		dst_pixel[0] = (uint16)GetUint32Color(color);
	} else if (bpp() == 4) {
		uint32* dst_pixel = (uint32*)pixels() + x + y * pitch() / bpp();
		dst_pixel[0] = GetUint32Color(color);
	}

	Unlock();

	RefreshCallback();
}

////////////////////////////////////////////////////////////
void Bitmap::HueChange(double hue) {
	HSLChange(hue, 1, 1, 0, GetRect());
}

////////////////////////////////////////////////////////////

static inline void RGB_to_HSL(const uint8& r, const uint8& g, const uint8& b,
							  int &h, int &s, int &l)
{
	enum RGBOrder {
		O_RGB,
		O_RBG,
		O_GRB,
		O_GBR,
		O_BRG,
		O_BGR
	} order = (r > g)
		  ? ((r > b) ? ((g < b) ? O_RBG : O_RGB) : O_BRG)
		  : ((r < b) ? ((g > b) ? O_GBR : O_BGR) : O_GRB);

	int c = 0;
	int l2 = 0;
	switch (order) {
		case O_RGB: c = (r - b); h = (c == 0) ? 0 : 0x100*(g - b)/c + 0x000; l2 = (r + b); break;
		case O_RBG: c = (r - g); h = (c == 0) ? 0 : 0x100*(g - b)/c + 0x600; l2 = (r + g); break;
		case O_GRB: c = (g - b); h = (c == 0) ? 0 : 0x100*(b - r)/c + 0x200; l2 = (g + b); break;
		case O_GBR: c = (g - r); h = (c == 0) ? 0 : 0x100*(b - r)/c + 0x200; l2 = (g + r); break;
		case O_BRG: c = (b - g); h = (c == 0) ? 0 : 0x100*(r - g)/c + 0x400; l2 = (b + g); break;
		case O_BGR: c = (b - r); h = (c == 0) ? 0 : 0x100*(r - g)/c + 0x400; l2 = (b + r); break;
	}

	if (l2 == 0) {
		s = 0;
		l = 0;
	}
	else {
		s = 0x100 * c / ((l2 > 0xFF) ? 0x1FF - l2 : l2);
		l = l2 / 2;
	}
}

static inline void HSL_to_RGB(const int& h, const int& s, const int& l,
							  uint8 &r, uint8 &g, uint8 &b)
{

	int l2 = 2 * l;
	int c = s * ((l2 > 0xFF) ? 0x1FF - l2 : l2) / 0x100;
	int m = (l2 - c) / 2;
	int h0 = h & 0xFF;
	int h1 = 0xFF - h0;

	switch (h >> 8) {
		case 0: r = m + c; g = m + h0*c/0x100; b = m; break;
		case 1: r = m + h1*c/0x100; g = m + c; b = m; break;
		case 2: r = m; g = m + c; b = m + h0*c/0x100; break;
		case 3: r = m; g = m + h1*c/0x100; b = m + c; break;
		case 4: r = m + h0*c/0x100; g = m; b = m + c; break;
		case 5: r = m + c; g = m; b = m + h1*c/0x100; break;
	}
}

static inline void HSL_adjust(int& h, int& s, int& l,
							  int hue, int sat, int lmul, int loff) {
	h += hue;
	if (h > 0x600) h -= 0x600;
	s = s * sat / 0x100;
	if (s > 0xFF) s = 0xFF;
	l = l * lmul / 0x100 + loff;
	l = (l > 0xFF) ? 0xFF : (l < 0) ? 0 : l;
}

static inline void RGB_adjust_HSL(uint8& r, uint8& g, uint8& b,
								  int hue, int sat, int lmul, int loff) {
	int h, s, l;
	RGB_to_HSL(r, g, b, h, s, l);
	HSL_adjust(h, s, l, hue, sat, lmul, loff);
	HSL_to_RGB(h, s, l, r, g, b);
}

void Bitmap::HSLChange(double h, double s, double l, double lo, Rect dst_rect) {
	dst_rect.Adjust(width(), height());
	if (dst_rect.IsOutOfBounds(width(), height())) return;

	int hue  = (int) (h / 60.0 * 0x100);
	int sat  = (int) (s * 0x100);
	int lum  = (int) (l * 0x100);
	int loff = (int) (lo * 0x100);

	if (hue < 0)
		hue += ((-hue + 0x5FF) / 0x600) * 0x600;
	else if (hue > 0x600)
		hue -= (hue / 0x600) * 0x600;

	Lock();

	if (bpp() == 2) {
		uint16* dst_pixels = (uint16*)pixels() + dst_rect.x + dst_rect.y * pitch() / bpp();
		int pad = pitch() / bpp() - dst_rect.width;

		for (int i = 0; i < dst_rect.height; i++) {
			for (int j = 0; j < dst_rect.width; j++) {
				#ifndef USE_ALPHA
					if (transparent && dst_pixels[0] == (uint16)colorkey()) continue;
				#endif
				uint8 dst_r, dst_g, dst_b, dst_a;
				GetColorComponents(dst_pixels[0], dst_r, dst_g, dst_b, dst_a);
				RGB_adjust_HSL(dst_r, dst_g, dst_b, hue, sat, lum, loff);
				dst_pixels[0] = (uint16)GetUint32Color(dst_r, dst_g, dst_b, dst_a);

				dst_pixels += 1;
			}
			dst_pixels += pad;
		}
	} else if (bpp() == 4) {
		const int rbyte = GetMaskByte(rmask());
		const int gbyte = GetMaskByte(gmask());
		const int bbyte = GetMaskByte(bmask());
		uint8* dst_pixels = (uint8*) pixels() + dst_rect.x * bpp() + dst_rect.y * pitch();
		int pad = pitch() - dst_rect.width * bpp();

		for (int i = 0; i < dst_rect.height; i++) {
			for (int j = 0; j < dst_rect.width; j++) {
				#ifndef USE_ALPHA
					if (transparent && dst_pixels[0] == colorkey()) continue;
				#endif
				RGB_adjust_HSL(dst_pixels[rbyte], dst_pixels[gbyte], dst_pixels[bbyte], hue, sat, lum, loff);

				dst_pixels += bpp();
			}
			dst_pixels += pad;
		}
	}

	Unlock();

	RefreshCallback();
}

////////////////////////////////////////////////////////////
Bitmap* Bitmap::Resample(int scale_w, int scale_h, const Rect& src_rect) {
	double zoom_x = (double)(scale_w) / src_rect.width;
	double zoom_y = (double)(scale_h) / src_rect.height;

	Bitmap* resampled = CreateBitmap(scale_w, scale_h, transparent);
	if (transparent)
		resampled->SetTransparentColor(GetTransparentColor());

	Lock();
	resampled->Lock();

	if (bpp() == 2) {
		uint16* src_pixels = (uint16*)pixels();
		uint16* dst_pixels = (uint16*)resampled->pixels();

		int stride = resampled->pitch() / resampled->bpp() - resampled->GetWidth();

		int nearest_y, nearest_match;

		for (int i = 0; i < scale_h; i++) {
			nearest_y = (src_rect.y + (int)(i / zoom_y)) * pitch() / bpp();

			for (int j = 0; j < scale_w; j++) {
				nearest_match = nearest_y + src_rect.x + (int)(j / zoom_x);
				dst_pixels[0] = src_pixels[nearest_match];
				dst_pixels += 1;
			}
			dst_pixels += stride;
		}
	} else if (bpp() == 4){
		uint32* nearest_y;
		uint32* nearest_match;
		uint32* dst_pixels = (uint32*)resampled->pixels();

		int stride = resampled->pitch() / resampled->bpp() - resampled->GetWidth();

		for (int i = 0; i < scale_h; i++) {
			nearest_y = (uint32*)pixels() + (src_rect.y + (int)(i / zoom_y)) * (pitch() / bpp());

			for (int j = 0; j < scale_w; j++) {
				nearest_match = nearest_y + src_rect.x + (int)(j / zoom_x);

				dst_pixels[0] = nearest_match[0];

				dst_pixels += 1;
			}
			dst_pixels += stride;
		}
	}

	Unlock();
	resampled->Unlock();

	return resampled;
}

////////////////////////////////////////////////////////////
void Bitmap::ToneChange(const Tone &tone) {
	if (tone == Tone()) return;

	Lock();

	if (bpp() == 2) {
		uint16* dst_pixels = (uint16*)pixels();

		int stride = pitch() / bpp() - width();

		uint8 dst_r, dst_g, dst_b, dst_a;

		if (tone.gray == 0) {
			for (int i = 0; i < height(); i++) {
				for (int j = 0; j < width(); j++) {
					#ifndef USE_ALPHA
						if (transparent && ((uint32*)dst_pixels)[0] == colorkey()) {
							dst_pixels++;
							continue;
						}
					#endif

					GetColorComponents(dst_pixels[0], dst_r, dst_g, dst_b, dst_a);

					dst_r = (uint8)max(min(dst_r + tone.red, 255), 0);
					dst_g = (uint8)max(min(dst_g + tone.green, 255), 0);
					dst_b = (uint8)max(min(dst_b + tone.blue, 255), 0);

					dst_pixels[0] = (uint16)GetUint32Color(dst_r, dst_g, dst_b, dst_a);

					dst_pixels++;
				}
				dst_pixels += stride;
			}
		} else {
			double factor = (255 - tone.gray) / 255.0;
			double gray;
			for (int i = 0; i < height(); i++) {
				for (int j = 0; j < width(); j++) {
					#ifndef USE_ALPHA
						if (transparent && ((uint32*)dst_pixels)[0] == colorkey()) {
							dst_pixels++;
							continue;
						}
					#endif

					GetColorComponents(dst_pixels[0], dst_r, dst_g, dst_b, dst_a);

					gray = dst_r * 0.299 + dst_g * 0.587 + dst_b * 0.114;

					dst_r = (uint8)max(min((dst_r - gray) * factor + gray + tone.red + 0.5, 255), 0);
					dst_g = (uint8)max(min((dst_g - gray) * factor + gray + tone.green + 0.5, 255), 0);
					dst_b = (uint8)max(min((dst_b - gray) * factor + gray + tone.blue + 0.5, 255), 0);

					dst_pixels++;
				}
				dst_pixels += stride;
			}
		}
	} else if (bpp() == 4) {
		uint8* dst_pixels = (uint8*)pixels();

		int stride = pitch() - width() * bpp();

		const int rbyte = GetMaskByte(rmask());
		const int gbyte = GetMaskByte(gmask());
		const int bbyte = GetMaskByte(bmask());

		if (tone.gray == 0) {
			for (int i = 0; i < height(); i++) {
				for (int j = 0; j < width(); j++) {
					#ifndef USE_ALPHA
						if (transparent && ((uint32*)dst_pixels)[0] == colorkey()) {
							dst_pixels += bpp();
							continue;
						}
					#endif

					dst_pixels[rbyte] = (uint8)max(min(dst_pixels[rbyte] + tone.red, 255), 0);
					dst_pixels[gbyte] = (uint8)max(min(dst_pixels[gbyte] + tone.green, 255), 0);
					dst_pixels[bbyte] = (uint8)max(min(dst_pixels[bbyte] + tone.blue, 255), 0);

					dst_pixels += bpp();
				}
				dst_pixels += stride;
			}
		} else {
			double factor = (255 - tone.gray) / 255.0;
			double gray;
			for (int i = 0; i < height(); i++) {
				for (int j = 0; j < width(); j++) {
					#ifndef USE_ALPHA
						if (transparent && ((uint32*)dst_pixels)[0] == colorkey()) {
							dst_pixels += bpp();
							continue;
						}
					#endif

					gray = dst_pixels[rbyte] * 0.299 + dst_pixels[gbyte] * 0.587 + dst_pixels[bbyte] * 0.114;

					dst_pixels[rbyte] = (uint8)max(min((dst_pixels[rbyte] - gray) * factor + gray + tone.red + 0.5, 255), 0);
					dst_pixels[gbyte] = (uint8)max(min((dst_pixels[gbyte] - gray) * factor + gray + tone.green + 0.5, 255), 0);
					dst_pixels[bbyte] = (uint8)max(min((dst_pixels[bbyte] - gray) * factor + gray + tone.blue + 0.5, 255), 0);

					dst_pixels += bpp();
				}
				dst_pixels += stride;
			}
		}
	}
	
	Unlock();

	RefreshCallback();
}

////////////////////////////////////////////////////////////
void Bitmap::Flip(bool horizontal, bool vertical) {
	if (!horizontal && !vertical) return;

	Lock();

	if (horizontal && vertical) {
		int stride = pitch() - width() * bpp();

		uint8* dst_pixels_first = (uint8*)pixels();
		uint8* dst_pixels_last = (uint8*)pixels() + (width() - 1) * bpp() + (height() - 1) * pitch();

		uint8* tmp_buffer = new uint8[bpp()];

		for (int i = 0; i < height() / 2; i++) {
			for (int j = 0; j < width(); j++) {
				if (dst_pixels_first == dst_pixels_last) break;

				memcpy(tmp_buffer, dst_pixels_first, bpp());
				memcpy(dst_pixels_first, dst_pixels_last, bpp());
				memcpy(dst_pixels_last, tmp_buffer, bpp());

				dst_pixels_first += bpp();
				dst_pixels_last -= bpp();
			}
			dst_pixels_first += stride;
			dst_pixels_last += stride;
		}

		delete tmp_buffer;
	} else if (horizontal) {
		int stride_left = (width() - width() / 2) * bpp();
		int stride_right = (width() + width() / 2) * bpp();

		uint8* dst_pixels_left = (uint8*)pixels();
		uint8* dst_pixels_right = (uint8*)pixels() + (width() - 1) * bpp();

		uint8* tmp_buffer = new uint8[bpp()];

		for (int i = 0; i < height(); i++) {
			for (int j = 0; j < width() / 2; j++) {
				if (dst_pixels_left == dst_pixels_right) continue;

				memcpy(tmp_buffer, dst_pixels_left, bpp());
				memcpy(dst_pixels_left, dst_pixels_right, bpp());
				memcpy(dst_pixels_right, tmp_buffer, bpp());

				dst_pixels_left += bpp();
				dst_pixels_right -= bpp();
			}
			dst_pixels_left += stride_left;
			dst_pixels_right += stride_right;
		}

		delete tmp_buffer;
	} else {
		uint8* dst_pixels_up = (uint8*)pixels();
		uint8* dst_pixels_down = (uint8*)pixels() + (height() - 1) * pitch();

		int stride = width() * bpp();

		uint8* tmp_buffer = new uint8[stride];

		for (int i = 0; i < height() / 2; i++) {
			if (dst_pixels_up == dst_pixels_down) break;

			memcpy(tmp_buffer, dst_pixels_down, stride);
			memcpy(dst_pixels_down, dst_pixels_up, stride);
			memcpy(dst_pixels_up, tmp_buffer, stride);

			dst_pixels_up += pitch();
			dst_pixels_down -= pitch();
		}

		delete tmp_buffer;
	}

	Unlock();

	RefreshCallback();
}

////////////////////////////////////////////////////////////
Bitmap* Bitmap::RotateScale(double angle, int scale_w, int scale_h) {
	double c = cos(-angle);
	double s = sin(-angle);
	int w = width();
	int h = height();
	double sx = (double) scale_w / w;
	double sy = (double) scale_h / h;

	double fxx =  c * sx;
	double fxy =  s * sy;
	double fyx = -s * sx;
	double fyy =  c * sy;

	double x0 = 0;
	double y0 = 0;
	double x1 = fxx * w;
	double y1 = fyx * w;
	double x2 = fxx * w + fxy * h;
	double y2 = fyx * w + fyy * h;
	double x3 = fxy * h;
	double y3 = fyy * h;

	double xmin = std::min(std::min(x0, x1), std::min(x2, x3));
	double ymin = std::min(std::min(y0, y1), std::min(y2, y3));
	double xmax = std::max(std::max(x0, x1), std::max(x2, x3));
	double ymax = std::max(std::max(y0, y1), std::max(y2, y3));
	double fx0 = -xmin;
	double fy0 = -ymin;

	int dst_w = (int)(ceil(xmax) - floor(xmin));
	int dst_h = (int)(ceil(ymax) - floor(ymin));

	double ixx =  c / sx;
	double ixy = -s / sx;
	double iyx =  s / sy;
	double iyy =  c / sy;
	double ix0 = -(c * fx0 - s * fy0) / sx;
	double iy0 = -(s * fx0 + c * fy0) / sy;

	Bitmap* result = CreateBitmap(dst_w, dst_h, true);
	const Color& trans = transparent ? GetTransparentColor() : Color(255,0,255,0);
	result->SetTransparentColor(trans);
	result->Fill(trans);

	Lock();
	result->Lock();

	if (bpp() == 2) {
		uint16* src_pixels = (uint16*)pixels();
		uint16* dst_pixels = (uint16*)result->pixels();
		int src_pitch = pitch() / bpp();
		int pad = result->pitch() / bpp() - result->GetWidth();

		for (int i = 0; i < dst_h; i++) {
			for (int j = 0; j < dst_w; j++) {
				double x = ix0 + ixy * (i + 0.5) + ixx * (j + 0.5);
				double y = iy0 + iyy * (i + 0.5) + iyx * (j + 0.5);
				int xi = (int) floor(x);
				int yi = (int) floor(y);
				if (xi < 0 || xi >= w || yi < 0 || yi >= h)
					dst_pixels++;
				else
					*dst_pixels++ = src_pixels[yi * src_pitch + xi];
			}
			dst_pixels += pad;
		}
	} else if (bpp() == 4){
		uint32* src_pixels = (uint32*)pixels();
		uint32* dst_pixels = (uint32*)result->pixels();
		int src_pitch = pitch() / bpp();
		int pad = result->pitch() / bpp() - result->GetWidth();

		for (int i = 0; i < dst_h; i++) {
			for (int j = 0; j < dst_w; j++) {
				double x = ix0 + ixy * (i + 0.5) + ixx * (j + 0.5);
				double y = iy0 + iyy * (i + 0.5) + iyx * (j + 0.5);
				int xi = (int) floor(x);
				int yi = (int) floor(y);
				if (xi < 0 || xi >= w || yi < 0 || yi >= h)
					dst_pixels++;
				else
					*dst_pixels++ = src_pixels[yi * src_pitch + xi];
			}
			dst_pixels += pad;
		}
	}

	Unlock();
	result->Unlock();

	return result;
}

////////////////////////////////////////////////////////////
void Bitmap::OpacityChange(int opacity, const Rect& src_rect) {
	if (opacity == 255)
		return;

	Lock();

	if (bpp() == 2) {
		uint8 dst_r, dst_g, dst_b, dst_a;

		for (int j = src_rect.y; j < src_rect.y + src_rect.height; j++) {
			uint16* dst_pixels = (uint16*) pixels() + j * pitch() / 2 + src_rect.x;
			for (int i = src_rect.x; i < src_rect.x + src_rect.width; i++) {
				GetColorComponents(dst_pixels[0], dst_r, dst_g, dst_b, dst_a);
				dst_a = dst_a * opacity / 255;

				dst_pixels++;
			}
		}
	} else if (bpp() == 4) {
		const int abyte = GetMaskByte(amask());

		for (int j = src_rect.y; j < src_rect.y + src_rect.height; j++) {
			uint8* dst_pixels = (uint8*) pixels() + j * pitch() + src_rect.x * bpp();
			for (int i = src_rect.x; i < src_rect.x + src_rect.width; i++) {
				dst_pixels[abyte] = (dst_pixels[abyte] * opacity) / 255;
				dst_pixels += bpp();
			}
		}
	}
	
	Unlock();

	RefreshCallback();
}

////////////////////////////////////////////////////////////
Bitmap* Bitmap::Waver(int depth, double phase) {
	Bitmap* resampled = CreateBitmap(width() + 2 * depth, height(), transparent);
	if (transparent)
		resampled->SetTransparentColor(GetTransparentColor());
	resampled->Clear();

	Lock();
	resampled->Lock();

	uint8* src_pixels = (uint8*)pixels();
	uint8* dst_pixels = (uint8*)resampled->pixels();

	for (int y = 0; y < height(); y++) {
		int offset = (int) (depth * (1 + sin((phase + y * 20) * 3.14159 / 180)));

		memcpy(&dst_pixels[offset * bpp()], src_pixels, width() * bpp());

		src_pixels += pitch();
		dst_pixels += resampled->pitch();
	}

	Unlock();
	resampled->Unlock();

	return resampled;
}

////////////////////////////////////////////////////////////
void Bitmap::BeginEditing() {
	editing = true;
}

void Bitmap::EndEditing() {
	editing = false;

	RefreshCallback();
}

void Bitmap::RefreshCallback() {
	if (editing) return;

	std::list<BitmapScreen*>::iterator it;


	for (it = attached_screen_bitmaps.begin(); it != attached_screen_bitmaps.end(); it++) {
		(*it)->SetDirty();
	}
}

void Bitmap::AttachBitmapScreen(BitmapScreen* bitmap) {
	attached_screen_bitmaps.push_back(bitmap);
}

void Bitmap::DetachBitmapScreen(BitmapScreen* bitmap) {
	attached_screen_bitmaps.remove(bitmap);
}

////////////////////////////////////////////////////////////
Rect Bitmap::GetTextSize(std::string text) const {
	return GetTextSize(Utils::DecodeUTF(text));
}

Rect Bitmap::GetTextSize(std::wstring text) const {
	return Rect(0, 0, text.size() * 6, 12);
}

////////////////////////////////////////////////////////////
int Bitmap::GetWidth() const {
	return width();
}

int Bitmap::GetHeight() const {
	return height();
}

Rect Bitmap::GetRect() const {
	return Rect(0, 0, width(), height());
}

Font* Bitmap::GetFont() const {
	return font;
}

void Bitmap::SetFont(Font* new_font) {
	font = new_font;
}

bool Bitmap::GetTransparent() const {
	return transparent;
}

Color Bitmap::GetTransparentColor() const {
#ifdef USE_ALPHA
	return Color(0, 0, 0, 0);
#else
	return GetColor(colorkey());
#endif
}

void Bitmap::TextDraw(int x, int y, int width, int height, std::wstring wtext, TextAlignment align) {
	Rect rect = GetTextSize(wtext);
	int dx = rect.width - width;

	switch (align) {
		case TextAlignLeft:
			TextDraw(x, y, wtext);
			break;
		case TextAlignCenter:
			TextDraw(x + dx / 2, y, wtext);
			break;
		case TextAlignRight:
			TextDraw(x + dx, y, wtext);
			break;
	}
}

void Bitmap::TextDraw(int x, int y, int width, int height, std::string text, TextAlignment align) {
	TextDraw(x, y, width, height, Utils::DecodeUTF(text), align);
}

void Bitmap::TextDraw(Rect rect, std::wstring wtext, TextAlignment align) {
	TextDraw(rect.x, rect.y, rect.width, rect.height, wtext, align);
}

void Bitmap::TextDraw(Rect rect, std::string text, TextAlignment align) {
	TextDraw(rect, Utils::DecodeUTF(text), align);
}

void Bitmap::TextDraw(int x, int y, std::wstring wtext, TextAlignment align) {
	Text::Draw(this, x, y, wtext, align);
	RefreshCallback();
}

void Bitmap::TextDraw(int x, int y, std::string text, TextAlignment align) {
	TextDraw(x, y, Utils::DecodeUTF(text), align);
}

