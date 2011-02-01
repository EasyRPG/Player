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
#include "bitmap_utils.h"
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
	return bm_utils->GetPixel(this, x, y);
}

////////////////////////////////////////////////////////////
Bitmap* Bitmap::Resample(int scale_w, int scale_h, const Rect& src_rect) {
	return bm_utils->Resample(this, scale_w, scale_h, src_rect);
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

