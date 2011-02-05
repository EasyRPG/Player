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
Bitmap::Bitmap() : opacity(NULL) {
}

////////////////////////////////////////////////////////////
Bitmap::~Bitmap() {
	if (opacity != NULL)
		delete[] opacity;
}

////////////////////////////////////////////////////////////
Color Bitmap::GetPixel(int x, int y) {
	if (x < 0 || y < 0 || x >= width() || y >= height())
		return Color();

	BitmapUtils* bm_utils = Begin();

	const uint8* src_pixels = pointer(x, y);
	uint8 r, g, b, a;
	bm_utils->GetPixel(src_pixels, r, g, b, a);

	End();

	return Color(r, g, b, a);
}

////////////////////////////////////////////////////////////
Bitmap* Bitmap::Resample(int scale_w, int scale_h, const Rect& src_rect) {
	Surface* dst = Surface::CreateSurface(scale_w, scale_h, transparent);
	if (transparent)
		dst->SetTransparentColor(GetTransparentColor());
	dst->Clear();
	dst->StretchBlit(dst->GetRect(), this, src_rect, 255);
	return dst;
}

////////////////////////////////////////////////////////////
void Bitmap::AttachBitmapScreen(BitmapScreen* bitmap) {
	attached_screen_bitmaps.push_back(bitmap);
}

void Bitmap::DetachBitmapScreen(BitmapScreen* bitmap) {
	attached_screen_bitmaps.remove(bitmap);
}

////////////////////////////////////////////////////////////
BitmapUtils* Bitmap::Begin() {
	Lock();
	BitmapUtils* bm_utils = BitmapUtils::Create(format, format, false);
	bm_utils->SetDstColorKey(colorkey());
	return bm_utils;
}

void Bitmap::End() {
	Unlock();
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
void Bitmap::SetTransparentColor(Color color) {
}

////////////////////////////////////////////////////////////
Bitmap::TileOpacity Bitmap::CheckOpacity(const Rect& rect) {
	bool all = true;
	bool any = false;

	BitmapUtils* bm_utils = Begin();

	uint8* src_pixels = pointer(rect.x, rect.y);

	for (int y = 0; y < rect.height; y++) {
		bm_utils->CheckOpacity(src_pixels, rect.width, all, any);
		if (any && !all)
			break;
		src_pixels += pitch();
	}

	End();

	return
		all ? Bitmap::Opaque :
		any ? Bitmap::Partial :
		Bitmap::Transparent;
}

////////////////////////////////////////////////////////////
void Bitmap::CheckPixels(uint32 flags) {
	if (flags & System) {
		Cache::system_info.bg_color = GetPixel(0, 32);
		Cache::system_info.sh_color = GetPixel(16, 32);
	}

	if (flags & Chipset) {
		opacity = new TileOpacity[16][30];
		for (int row = 0; row < 16; row++) {
			for (int col = 0; col < 30; col++) {
				Rect rect(col * 16, row * 16, 16, 16);
				opacity[row][col] = CheckOpacity(rect);
			}
		}
	}
}

////////////////////////////////////////////////////////////
Bitmap::TileOpacity Bitmap::GetTileOpacity(int row, int col) {
	return (opacity == NULL) ? Partial : opacity[row][col];
}

////////////////////////////////////////////////////////////
uint8* Bitmap::pointer(int x, int y) {
	return (uint8*) pixels() + y * pitch() + x * bpp();
}

