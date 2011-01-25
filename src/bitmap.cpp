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
#include "cache.h"
#include "bitmap.h"
#include "surface.h"
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

Bitmap* Bitmap::CreateBitmap(int width, int height, const Color& color) {
	Surface *surface = Surface::CreateSurface(width, height, false);
	surface->Fill(color);
	return surface;
}

Bitmap* Bitmap::CreateBitmap(const std::string& filename, bool transparent, uint32 flags) {
	#if defined(USE_SDL_BITMAP)
		return (Bitmap*)new SdlBitmap(filename, transparent, flags);
	#elif defined(USE_SOFT_BITMAP)
		return (Bitmap*)new SoftBitmap(filename, transparent, flags);
	#elif defined(USE_PIXMAN_BITMAP)
		return (Bitmap*)new PixmanBitmap(filename, transparent, flags);
	#elif defined(USE_OPENGL_BITMAP)
		return (Bitmap*)new GlBitmap(filename, transparent, flags);
	#else
		#error "No bitmap implementation selected"
	#endif
}

Bitmap* Bitmap::CreateBitmap(const uint8* data, uint bytes, bool transparent, uint32 flags) {
	#if defined(USE_SDL_BITMAP)
		return (Bitmap*)new SdlBitmap(data, bytes, transparent, flags);
	#elif defined(USE_SOFT_BITMAP)
		return (Bitmap*)new SoftBitmap(data, bytes, transparent, flags);
	#elif defined(USE_PIXMAN_BITMAP)
		return (Bitmap*)new PixmanBitmap(data, bytes, transparent, flags);
	#elif defined(USE_OPENGL_BITMAP)
		return (Bitmap*)new GlBitmap(data, bytes, transparent, flags);
	#else
		#error "No bitmap implementation selected"
	#endif
}

Bitmap* Bitmap::CreateBitmap(Bitmap* source, Rect src_rect, bool transparent) {
	return Surface::CreateSurface(source, src_rect, transparent);
}

////////////////////////////////////////////////////////////
Bitmap::Bitmap() {
}

////////////////////////////////////////////////////////////
Bitmap::~Bitmap() {
}

////////////////////////////////////////////////////////////
Color Bitmap::GetPixel(int x, int y) {
	if (x < 0 || y < 0 || x >= width() || y >= height()) return Color();

	uint32 pixel = 0;

	Lock();

	if (bpp() == 2) {
		const uint16* src_pixel = (const uint16*)pixels() + x + y * (pitch() / bpp());
		pixel = src_pixel[0];
	} else if (bpp() == 4) {
		const uint32* src_pixel = (const uint32*)pixels() + x + y * (pitch() / bpp());
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
Bitmap* Bitmap::Resample(int scale_w, int scale_h, const Rect& src_rect) {
	double zoom_x = (double)(scale_w) / src_rect.width;
	double zoom_y = (double)(scale_h) / src_rect.height;

	Surface* resampled = Surface::CreateSurface(scale_w, scale_h, transparent);
	if (transparent)
		resampled->SetTransparentColor(GetTransparentColor());

	Lock();
	resampled->Lock();

	if (bpp() == 2) {
		const uint16* src_pixels = (const uint16*)pixels();
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
		const uint32* nearest_y;
		const uint32* nearest_match;
		uint32* dst_pixels = (uint32*)resampled->pixels();

		int stride = resampled->pitch() / resampled->bpp() - resampled->GetWidth();

		for (int i = 0; i < scale_h; i++) {
			nearest_y = (const uint32*)pixels() + (src_rect.y + (int)(i / zoom_y)) * (pitch() / bpp());

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

	Surface* result = Surface::CreateSurface(dst_w, dst_h, true);
	const Color& trans = transparent ? GetTransparentColor() : Color(255,0,255,0);
	result->SetTransparentColor(trans);
	result->Fill(trans);

	Lock();
	result->Lock();

	if (bpp() == 2) {
		const uint16* src_pixels = (const uint16*)pixels();
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
		const uint32* src_pixels = (const uint32*)pixels();
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
Bitmap* Bitmap::Waver(int depth, double phase) {
	Surface* resampled = Surface::CreateSurface(width() + 2 * depth, height(), true);
	if (transparent)
		resampled->SetTransparentColor(GetTransparentColor());
	resampled->Clear();

	Lock();
	resampled->Lock();

	const uint8* src_pixels = (const uint8*)pixels();
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
void Bitmap::AttachBitmapScreen(BitmapScreen* bitmap) {
	attached_screen_bitmaps.push_back(bitmap);
}

void Bitmap::DetachBitmapScreen(BitmapScreen* bitmap) {
	attached_screen_bitmaps.remove(bitmap);
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

////////////////////////////////////////////////////////////
void Bitmap::CheckPixels(uint32 flags) {
	if (flags & System) {
		Cache::system_info.bg_color = GetPixel(0, 32);
		Cache::system_info.sh_color = GetPixel(16, 32);
	}

	if (flags & Chipset) {
		have_invisible_tile = true;
		for (int x = 288; x < 288 + 16; x++) {
			for (int y = 128; y < 128 + 16; y++) {
				if (GetPixel(x, y).alpha > 0) {
					have_invisible_tile = false;
					x = 288 + 16;
					break;
				}
			}
		}
	}
}

bool Bitmap::HaveInvisibleTile() {
	return have_invisible_tile;
}

