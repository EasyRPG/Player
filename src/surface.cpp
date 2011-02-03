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
#include "bitmap_screen.h"
#include "output.h"
#include "text.h"
#include "bitmap_utils.h"
#include "surface.h"
#include "wcwidth.h"

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
Surface* Surface::CreateSurface(int width, int height, int bpp, bool transparent) {
	#if defined(USE_SDL_BITMAP)
		return (Surface*)new SdlBitmap(width, height, bpp, transparent);
	#elif defined(USE_SOFT_BITMAP)
		return (Surface*)new SoftBitmap(width, height, transparent);
	#elif defined(USE_PIXMAN_BITMAP)
		return (Surface*)new PixmanBitmap(width, height, transparent);
	#elif defined(USE_OPENGL_BITMAP)
		return (Surface*)new GlBitmap(width, height, transparent);
	#else
		#error "No bitmap implementation selected"
	#endif
}

Surface* Surface::CreateSurface(Bitmap* source, Rect src_rect, bool transparent) {
	#if defined(USE_SDL_BITMAP)
		return (Surface*)new SdlBitmap(source, src_rect, transparent);
	#elif defined(USE_SOFT_BITMAP)
		return (Surface*)new SoftBitmap(source, src_rect, transparent);
	#elif defined(USE_PIXMAN_BITMAP)
		return (Surface*)new PixmanBitmap(source, src_rect, transparent);
	#elif defined(USE_OPENGL_BITMAP)
		return (Surface*)new GlBitmap(source, src_rect, transparent);
	#else
		#error "No bitmap implementation selected"
	#endif
}

Surface* Surface::CreateSurfaceFrom(void *pixels, int width, int height, int depth, int pitch, uint32 Rmask, uint32 Gmask, uint32 Bmask, uint32 Amask) {
	#if defined(USE_SDL_BITMAP)
		return (Surface*) new SdlBitmap(pixels, width, height, depth, pitch, Rmask, Gmask, Bmask, Amask);
	#elif defined(USE_SOFT_BITMAP)
		return (Surface*) new SoftBitmap(pixels, width, height, pitch);
	#elif defined(USE_PIXMAN_BITMAP)
		return (Surface*) new PixmanBitmap(pixels, width, height, pitch);
	#else
		#error "No bitmap implementation selected"
	#endif
}

////////////////////////////////////////////////////////////
Surface::Surface() :
	editing(false) {
	font = Font::CreateFont();
}

////////////////////////////////////////////////////////////
Surface::~Surface() {
}

////////////////////////////////////////////////////////////
void Surface::SetPixel(int x, int y, const Color &color) {
	bm_utils->SetPixel(this, x, y, color);
}

////////////////////////////////////////////////////////////
void Surface::Blit(int x, int y, Bitmap* src, Rect src_rect, int opacity) {
	bm_utils->Blit(this, x, y, src, src_rect, opacity);
}

////////////////////////////////////////////////////////////
void Surface::TiledBlit(Rect src_rect, Bitmap* src, Rect dst_rect, int opacity) {
	bm_utils->TiledBlit(this, src_rect, src, dst_rect, opacity);
}

////////////////////////////////////////////////////////////
void Surface::TiledBlit(int ox, int oy, Rect src_rect, Bitmap* src, Rect dst_rect, int opacity) {
	bm_utils->TiledBlit(this, ox, oy, src_rect, src, dst_rect, opacity);
}

////////////////////////////////////////////////////////////
void Surface::StretchBlit(Bitmap* src, Rect src_rect, int opacity) {
	if (src_rect.width == width() && src_rect.height == height())
		Blit(0, 0, src, src_rect, opacity);
	else
		StretchBlit(GetRect(), src, src_rect, opacity);
}

////////////////////////////////////////////////////////////
void Surface::StretchBlit(Rect dst_rect, Bitmap* src, Rect src_rect, int opacity) {
	bm_utils->StretchBlit(this, dst_rect, src, src_rect, opacity);
}

////////////////////////////////////////////////////////////
void Surface::FlipBlit(int x, int y, Bitmap* src, Rect src_rect, bool horizontal, bool vertical) {
	bm_utils->FlipBlit(this, x, y, src, src_rect, horizontal, vertical);
}

////////////////////////////////////////////////////////////
void Surface::ScaleBlit(const Rect& dst_rect, Bitmap* src, const Rect& src_rect) {
	bm_utils->ScaleBlit(this, dst_rect, src, src_rect);
}

////////////////////////////////////////////////////////////
void Surface::TransformBlit(Rect dst_rect, Bitmap* src, Rect src_rect, const Matrix& inv) {
	bm_utils->TransformBlit(this, dst_rect, src, src_rect, inv);
}

////////////////////////////////////////////////////////////
void Surface::TransformBlit(Rect dst_rect,
							Bitmap* src, Rect src_rect,
							double angle,
							double scale_x, double scale_y,
							int src_pos_x, int src_pos_y,
							int dst_pos_x, int dst_pos_y) {
	bm_utils->TransformBlit(
		this, dst_rect, src, src_rect,
		angle, scale_x, scale_y,
		src_pos_x, src_pos_y, dst_pos_x, dst_pos_y);
}

////////////////////////////////////////////////////////////
void Surface::Mask(int x, int y, Bitmap* src, Rect src_rect) {
	bm_utils->Mask(this, x, y, src, src_rect);
}

////////////////////////////////////////////////////////////
void Surface::WaverBlit(int x, int y, Bitmap* src, Rect src_rect, int depth, double phase) {
	bm_utils->WaverBlit(this, x, y, src, src_rect, depth, phase);
}

////////////////////////////////////////////////////////////
void Surface::Fill(const Color &color) {
	FillRect(GetRect(), color);
}

////////////////////////////////////////////////////////////
void Surface::FillRect(Rect dst_rect, const Color &color) {
	bm_utils->FillRect(this, dst_rect, color);
}

////////////////////////////////////////////////////////////
void Surface::Clear() {
	Fill(GetTransparentColor());
}

////////////////////////////////////////////////////////////
void Surface::ClearRect(Rect dst_rect) {
	FillRect(dst_rect, GetTransparentColor());
}

////////////////////////////////////////////////////////////
void Surface::HueChange(double hue) {
	HSLChange(hue, 1, 1, 0, GetRect());
}

////////////////////////////////////////////////////////////
void Surface::HSLChange(double h, double s, double l, double lo, Rect dst_rect) {
	bm_utils->HSLChange(this, h, s, l, lo, dst_rect);
}

////////////////////////////////////////////////////////////
void Surface::ToneChange(const Rect &dst_rect, const Tone &tone) {
	bm_utils->ToneChange(this, dst_rect, tone);
}

////////////////////////////////////////////////////////////
void Surface::Flip(const Rect& dst_rect, bool horizontal, bool vertical) {
	bm_utils->Flip(this, dst_rect, horizontal, vertical);
}

////////////////////////////////////////////////////////////
void Surface::OpacityChange(int opacity, const Rect& src_rect) {
	bm_utils->OpacityChange(this, opacity, src_rect);
}

////////////////////////////////////////////////////////////
void Surface::BeginEditing() {
	editing = true;
}

void Surface::EndEditing() {
	editing = false;

	RefreshCallback();
}

void Surface::RefreshCallback() {
	if (editing) return;

	std::list<BitmapScreen*>::iterator it;

	for (it = attached_screen_bitmaps.begin(); it != attached_screen_bitmaps.end(); it++) {
		(*it)->SetDirty();
	}
}

////////////////////////////////////////////////////////////
Rect Surface::GetTextSize(const std::string& text) {
	return GetTextSize(Utils::DecodeUTF(text));
}

Rect Surface::GetTextSize(const std::wstring& text) {
	int size = mk_wcswidth(text.c_str(), text.size());

	if (size == -1) {
		Output::Warning("Text contains invalid chars.\n"\
			"Is the encoding correct?");

		return Rect(0, 0, text.size() * 6, 12);
	} else {
		return Rect(0, 0, size * 6, 12);
	}
}
////////////////////////////////////////////////////////////

Font* Surface::GetFont() const {
	return font;
}

void Surface::SetFont(Font* new_font) {
	font = new_font;
}

void Surface::TextDraw(int x, int y, int width, int height, int color, std::wstring wtext, TextAlignment align) {
	Rect rect = Surface::GetTextSize(wtext);
	int dx = rect.width - width;

	switch (align) {
		case TextAlignLeft:
			TextDraw(x, y, color, wtext);
			break;
		case TextAlignCenter:
			TextDraw(x + dx / 2, y, color, wtext);
			break;
		case TextAlignRight:
			TextDraw(x + dx, y, color, wtext);
			break;
	}
}

void Surface::TextDraw(int x, int y, int width, int height, int color, std::string text, TextAlignment align) {
	TextDraw(x, y, width, height, color, Utils::DecodeUTF(text), align);
}

void Surface::TextDraw(Rect rect, int color, std::wstring wtext, TextAlignment align) {
	TextDraw(rect.x, rect.y, rect.width, rect.height, color, wtext, align);
}

void Surface::TextDraw(Rect rect, int color, std::string text, TextAlignment align) {
	TextDraw(rect, color, Utils::DecodeUTF(text), align);
}

void Surface::TextDraw(int x, int y, int color, std::wstring wtext, TextAlignment align) {
	Text::Draw(this, x, y, color, wtext, align);
	RefreshCallback();
}

void Surface::TextDraw(int x, int y, int color, std::string text, TextAlignment align) {
	TextDraw(x, y, color, Utils::DecodeUTF(text), align);
}

