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
Surface* Surface::CreateSurface(int width, int height, bool transparent, int bpp) {
	#if defined(USE_SDL_BITMAP)
		return (Surface*)new SdlBitmap(width, height, transparent, bpp);
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

Surface* Surface::CreateSurfaceFrom(void *pixels, int width, int height, int pitch, const DynamicFormat& format) {
	#if defined(USE_SDL_BITMAP)
		return (Surface*) new SdlBitmap(pixels, width, height, pitch, format);
	#elif defined(USE_SOFT_BITMAP)
		return (Surface*) new SoftBitmap(pixels, width, height, pitch, format);
	#elif defined(USE_PIXMAN_BITMAP)
		return (Surface*) new PixmanBitmap(pixels, width, height, pitch, format);
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
	if (x < 0 || y < 0 || x >= width() || y >= height()) return;

	BitmapUtils* bm_utils = Begin();

	uint8* dst_pixels = pointer(x, y);
	bm_utils->SetPixel(dst_pixels, color.red, color.green, color.blue, color.alpha);

	End();
}

////////////////////////////////////////////////////////////
void Surface::Blit(int x, int y, Bitmap* src, Rect src_rect, int opacity) {
	if (opacity < 0) return;

	Rect dst_rect(x, y, 0, 0);

	if (!Rect::AdjustRectangles(src_rect, dst_rect, src->GetRect()))
		return;
	if (!Rect::AdjustRectangles(dst_rect, src_rect, GetRect()))
		return;

	if (opacity >= 255)
		opacity = 255;

	BitmapUtils* bm_utils = Begin(src);

	bool opacity_blit = opacity < 255;
	bool overlay_blit = bm_utils->GetSrcFormat().alpha_type != PF::NoAlpha;

	const uint8* src_pixels = src->pointer(src_rect.x, src_rect.y);
	uint8* dst_pixels = pointer(dst_rect.x, dst_rect.y);

	for (int i = 0; i < dst_rect.height; i++) {
		if (opacity_blit)
			bm_utils->OpacityBlit(dst_pixels, src_pixels, dst_rect.width, opacity);
		else if (overlay_blit)
			bm_utils->OverlayBlit(dst_pixels, src_pixels, dst_rect.width);
		else
			bm_utils->CopyBlit(dst_pixels, src_pixels, dst_rect.width);
		src_pixels += src->pitch();
		dst_pixels += pitch();
	}

	End(src);
}

////////////////////////////////////////////////////////////
void Surface::TiledBlit(Rect src_rect, Bitmap* src, Rect dst_rect, int opacity) {
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
		tile.height = std::min(src_rect.height, dst_rect.height + dst_rect.y - j * src_rect.height);
		for (int i = 0; i < x_blits; i++) {
			tile.width = std::min(src_rect.width, dst_rect.width + dst_rect.x - i * src_rect.width);
			Blit(dst_rect.x + i * src_rect.width, dst_rect.y + j * src_rect.height, src, tile, opacity);
		}
	}
}

////////////////////////////////////////////////////////////
void Surface::TiledBlit(int ox, int oy, Rect src_rect, Bitmap* src, Rect dst_rect, int opacity) {
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

		tile.height = std::min(tile.height, dst_rect.y + dst_rect.height - dst_y);

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

			tile.width = std::min(tile.width, dst_rect.x + dst_rect.width - dst_x);

			Blit(dst_x, dst_y, src, tile, opacity);
		}
	}
}

////////////////////////////////////////////////////////////
void Surface::StretchBlit(Bitmap* src, Rect src_rect, int opacity) {
	StretchBlit(GetRect(), src, src_rect, opacity);
}

////////////////////////////////////////////////////////////
void Surface::StretchBlit(Rect dst_rect, Bitmap* src, Rect src_rect, int opacity) {
	if (opacity <= 0)
		return;

	if (src_rect.width == dst_rect.width && src_rect.height == dst_rect.height) {
		Blit(dst_rect.x, dst_rect.y, src, src_rect, opacity);
		return;
	}

	double zoom_x = (double)src_rect.width / dst_rect.width;
	double zoom_y = (double)src_rect.height / dst_rect.height;

	double sx0 = src_rect.x;
	double sy0 = src_rect.y;
	double sx1 = src_rect.x + src_rect.width;
	double sy1 = src_rect.y + src_rect.height;
	int dx0 = dst_rect.x;
	int dy0 = dst_rect.y;
	int dx1 = dst_rect.x + dst_rect.width;
	int dy1 = dst_rect.y + dst_rect.height;

	if (dx0 < 0) {
		sx0 -= zoom_x * dx0;
		dx0 = 0;
	}

	int dw = GetWidth();
	if (dx1 - dw > 0) {
		sx1 -= zoom_x * (dx1 - dw);
		dx1 = dw;
	}

	if (dy0 < 0) {
		sy0 -= zoom_y * dy0;
		dy0 = 0;
	}

	int dh = GetHeight();
	if (dy1 - dh > 0) {
		sy1 -= zoom_y * (dy1 - dh);
		dy1 = dh;
	}

	if (dx0 >= dx1 || dy0 >= dy1)
		return;

	BitmapUtils* bm_utils = Begin(src);

	uint8* dst_pixels = pointer(dx0, dy0);
	int step = (int)((sx1 - sx0) * (1 << BitmapUtils::FRAC_BITS)) / (dx1 - dx0);

	for (int i = 0; i < dy1 - dy0; i++) {
		const uint8* nearest_y = src->pointer(0, (int)(sy0 + i * zoom_y));
		int x = (int)(sx0 * (1 << BitmapUtils::FRAC_BITS)) + step / 2;

		bm_utils->ScaleBlit(dst_pixels, nearest_y, dst_rect.width, x, step, opacity);
		dst_pixels += pitch();
	}

	End(src);
}

////////////////////////////////////////////////////////////
void Surface::FlipBlit(int x, int y, Bitmap* src, Rect src_rect, bool horizontal, bool vertical) {
	if (!horizontal && !vertical) {
		Blit(x, y, src, src_rect, 255);
		return;
	}

	int ox = horizontal
		? src_rect.x + src_rect.width - 1 + x
		: src_rect.x - x;
	int oy = vertical
		? src_rect.y + src_rect.height - 1 + y
		: src_rect.y - y;

	Rect dst_rect(x, y, 0, 0);

	if (!Rect::AdjustRectangles(src_rect, dst_rect, src->GetRect()))
		return;
	if (!Rect::AdjustRectangles(dst_rect, src_rect, GetRect()))
		return;

	BitmapUtils* bm_utils = Begin(src);

	int sx0 = horizontal ? ox - dst_rect.x : ox + dst_rect.x;
	int sy0 = vertical   ? oy - dst_rect.y : oy + dst_rect.y;

	uint8* dst_pixels = pointer(dst_rect.x, dst_rect.y);
	const uint8* src_pixels = src->pointer(sx0, sy0);

	if (horizontal && vertical) {
		for (int y = 0; y < dst_rect.height; y++) {
			bm_utils->FlipHBlit(dst_pixels, src_pixels, dst_rect.width);
			dst_pixels += pitch();
			src_pixels -= src->pitch();
		}
	} else if (horizontal) {
		for (int y = 0; y < dst_rect.height; y++) {
			bm_utils->FlipHBlit(dst_pixels, src_pixels, dst_rect.width);
			dst_pixels += pitch();
			src_pixels += src->pitch();
		}
	} else {
		for (int y = 0; y < dst_rect.height; y++) {
			bm_utils->CopyBlit(dst_pixels, src_pixels, dst_rect.width);
			dst_pixels += pitch();
			src_pixels -= src->pitch();
		}
	}

	End(src);
}

////////////////////////////////////////////////////////////
void Surface::TransformBlit(Rect dst_rect, Bitmap* src, Rect src_rect, const Matrix& inv, int opacity) {
	if (opacity <= 0)
		return;

	dst_rect.Adjust(GetWidth(), GetHeight());

	BitmapUtils* bm_utils = Begin(src);

	const uint8* src_pixels = (const uint8*)src->pointer(0, 0);
	uint8* dst_pixels = pointer(dst_rect.x, dst_rect.y);

	for (int y = dst_rect.y; y < dst_rect.y + dst_rect.height; y++) {
		bm_utils->TransformBlit(dst_pixels, src_pixels, src->pitch(),
								dst_rect.x, dst_rect.x + dst_rect.width, y,
								src_rect, inv, opacity);
		dst_pixels += pitch();
	}

	End(src);
}

////////////////////////////////////////////////////////////
void Surface::TransformBlit(Rect dst_rect,
							Bitmap* src, Rect src_rect,
							double angle,
							double scale_x, double scale_y,
							int src_pos_x, int src_pos_y,
							int dst_pos_x, int dst_pos_y,
							int opacity) {
	Matrix fwd = Matrix::Setup(angle, scale_x, scale_y,
							   src_pos_x, src_pos_y,
							   dst_pos_x, dst_pos_y);
	Matrix inv = fwd.Inverse();

	Rect rect = TransformRectangle(fwd, src_rect);
	dst_rect.Adjust(rect);
	if (dst_rect.IsEmpty())
		return;

	TransformBlit(dst_rect, src, src_rect, inv, opacity);
}

////////////////////////////////////////////////////////////
void Surface::MaskBlit(int x, int y, Bitmap* src, Rect src_rect) {
	Rect dst_rect(x, y, 0, 0);

	if (!Rect::AdjustRectangles(src_rect, dst_rect, src->GetRect()))
		return;
	if (!Rect::AdjustRectangles(dst_rect, src_rect, GetRect()))
		return;

	BitmapUtils* bm_utils = Begin(src);

	const uint8* src_pixels = src->pointer(src_rect.x, src_rect.y);
	uint8* dst_pixels = pointer(dst_rect.x, dst_rect.y);

	for (int j = 0; j < dst_rect.height; j++) {
		bm_utils->MaskBlit(dst_pixels, src_pixels, dst_rect.width);
		src_pixels += src->pitch();
		dst_pixels += pitch();
	}

	End(src);
}

////////////////////////////////////////////////////////////
void Surface::WaverBlit(int x, int y, Bitmap* src, Rect src_rect, int depth, double phase, int opacity) {
	if (y < 0) {
		src_rect.y -= y;
		src_rect.height += y;
		y = 0;
	}

	if (y + src_rect.height > GetHeight()) {
		int dy = y + src_rect.height - GetHeight();
		src_rect.height -= dy;
	}

	BitmapUtils* bm_utils = Begin(src);

	const uint8* src_pixels = src->pointer(src_rect.x, src_rect.y);
	uint8* dst_pixels = pointer(x, y);
	int dst_width = GetWidth();
	int src_bytes = src->bytes();
	int dst_bytes = bytes();

	for (int y = 0; y < src_rect.height; y++) {
		int offset = (int) (depth * (1 + sin((phase + y * 20) * 3.14159 / 180)));
		int sx0 = 0;
		int dx0 = offset;
		int count = src_rect.width;
		if (x + offset + count > dst_width)
			count -= x + count + offset - dst_width;
		if (x + offset < 0) {
			sx0 -= x + offset;
			dx0 -= x + offset;
			count += x + offset;
		}

		bm_utils->Blit(&dst_pixels[dx0 * dst_bytes], &src_pixels[sx0 * src_bytes], count, opacity);

		src_pixels += src->pitch();
		dst_pixels += pitch();
	}

	End(src);
}

////////////////////////////////////////////////////////////
void Surface::Fill(const Color &color) {
	FillRect(GetRect(), color);
}

////////////////////////////////////////////////////////////
void Surface::FillRect(Rect dst_rect, const Color &color) {
	dst_rect.Adjust(width(), height());
	if (dst_rect.IsOutOfBounds(width(), height()))
		return;

	BitmapUtils* bm_utils = Begin();

	uint8 pixel[4];
	bm_utils->SetPixel(pixel, color.red, color.green, color.blue, color.alpha);

	uint8* dst_pixels = pointer(dst_rect.x, dst_rect.y);

	if (dst_rect.width * bytes() == pitch())
		bm_utils->SetPixels(dst_pixels, pixel, dst_rect.height * dst_rect.width);
	else {
		for (int i = 0; i < dst_rect.height; i++) {
			bm_utils->SetPixels(dst_pixels, pixel, dst_rect.width);
			dst_pixels += pitch();
		}
	}

	End();
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
void Surface::HueChangeBlit(int x, int y, Bitmap* src, Rect src_rect, double hue) {
	HSLBlit(x, y, src, src_rect, hue, 1, 1, 0);
}

////////////////////////////////////////////////////////////
void Surface::HSLBlit(int x, int y, Bitmap* src, Rect src_rect, double h, double s, double l, double lo) {
	Rect dst_rect(x, y, 0, 0);

	if (!Rect::AdjustRectangles(src_rect, dst_rect, src->GetRect()))
		return;
	if (!Rect::AdjustRectangles(dst_rect, src_rect, GetRect()))
		return;

	int hue  = (int) (h / 60.0 * 0x100);
	int sat  = (int) (s * 0x100);
	int lum  = (int) (l * 0x100);
	int loff = (int) (lo * 0x100);

	if (hue < 0)
		hue += ((-hue + 0x5FF) / 0x600) * 0x600;
	else if (hue > 0x600)
		hue -= (hue / 0x600) * 0x600;

	BitmapUtils* bm_utils = Begin(src);

	const uint8* src_pixels = src->pointer(src_rect.x, src_rect.y);
	uint8* dst_pixels = pointer(dst_rect.x, dst_rect.y);

	for (int i = 0; i < dst_rect.height; i++) {
		bm_utils->HSLBlit(dst_pixels, src_pixels, dst_rect.width, hue, sat, lum, loff);
		dst_pixels += pitch();
	}

	End(src);
}

////////////////////////////////////////////////////////////
void Surface::ToneBlit(int x, int y, Bitmap* src, Rect src_rect, const Tone &tone) {
	if (tone == Tone()) {
		if (src != this)
			Blit(x, y, src, src_rect, 255);
		return;
	}

	Rect dst_rect(x, y, 0, 0);

	if (!Rect::AdjustRectangles(src_rect, dst_rect, src->GetRect()))
		return;
	if (!Rect::AdjustRectangles(dst_rect, src_rect, GetRect()))
		return;

	BitmapUtils* bm_utils = Begin(src);

	const uint8* src_pixels = src->pointer(src_rect.x, src_rect.y);
	uint8* dst_pixels = pointer(dst_rect.x, dst_rect.y);

	if (tone.gray == 0) {
		for (int i = 0; i < dst_rect.height; i++) {
			bm_utils->ToneBlit(dst_pixels, src_pixels, dst_rect.width, tone);
			src_pixels += src->pitch();
			dst_pixels += pitch();
		}
	} else {
		double factor = (255 - tone.gray) / 255.0;
		for (int i = 0; i < dst_rect.height; i++) {
			bm_utils->ToneBlit(dst_pixels, src_pixels, dst_rect.width, tone, factor);
			src_pixels += src->pitch();
			dst_pixels += pitch();
		}
	}
	
	End(src);
}

////////////////////////////////////////////////////////////
void Surface::OpacityBlit(int x, int y, Bitmap* src, Rect src_rect, int opacity) {
	if (opacity == 255) {
		if (src != this)
			Blit(x, y, src, src_rect, 255);
		return;
	}

	Rect dst_rect(x, y, 0, 0);

	if (!Rect::AdjustRectangles(src_rect, dst_rect, src->GetRect()))
		return;
	if (!Rect::AdjustRectangles(dst_rect, src_rect, GetRect()))
		return;

	BitmapUtils* bm_utils = Begin(src);

	const uint8* src_pixels = src->pointer(src_rect.x, src_rect.y);
	uint8* dst_pixels = pointer(dst_rect.x, dst_rect.y);

	for (int j = 0; j < dst_rect.height; j++) {
		bm_utils->OpacityChangeBlit(dst_pixels, src_pixels, dst_rect.width, opacity);
		src_pixels += src->pitch();
		dst_pixels += pitch();
	}

	End(src);
}

////////////////////////////////////////////////////////////
void Surface::Flip(const Rect& dst_rect, bool horizontal, bool vertical) {
	if (!horizontal && !vertical)
		return;

	BitmapUtils* bm_utils = Begin();

	if (horizontal && vertical) {
		int pad = pitch() - width() * bytes();
		uint8* dst_pixels_first = pointer(dst_rect.x, dst_rect.y);
		uint8* dst_pixels_last = pointer(dst_rect.x + dst_rect.width - 1, dst_rect.y + dst_rect.height - 1);

		for (int i = 0; i < dst_rect.height / 2; i++) {
			bm_utils->FlipHV(dst_pixels_first, dst_pixels_last, dst_rect.width);
			dst_pixels_first += pad;
			dst_pixels_last -= pad;
		}
	} else if (horizontal) {
		int pad_left = (dst_rect.width - dst_rect.width / 2) * bytes();
		int pad_right = (dst_rect.width + dst_rect.width / 2) * bytes();

		uint8* dst_pixels_left = pointer(dst_rect.x, dst_rect.y);
		uint8* dst_pixels_right = pointer(dst_rect.x + dst_rect.width - 1, dst_rect.y);

		for (int i = 0; i < dst_rect.height; i++) {
			bm_utils->FlipH(dst_pixels_left, dst_pixels_right, dst_rect.width / 2);
			dst_pixels_left += pad_left;
			dst_pixels_right += pad_right;
		}
	} else {
		uint8* dst_pixels_up = pointer(dst_rect.x, dst_rect.y);
		uint8* dst_pixels_down = pointer(dst_rect.x, dst_rect.y + dst_rect.height - 1);
		uint8* tmp_buffer = new uint8[dst_rect.width * bytes()];

		for (int i = 0; i < dst_rect.height / 2; i++) {
			if (dst_pixels_up == dst_pixels_down)
				break;
			bm_utils->FlipV(dst_pixels_up, dst_pixels_down, dst_rect.width, tmp_buffer);
			dst_pixels_up += pitch();
			dst_pixels_down -= pitch();
		}

		delete tmp_buffer;
	}

	End();
}

////////////////////////////////////////////////////////////
void Surface::Blit2x(Rect dst_rect, Bitmap* src, Rect src_rect) {
	dst_rect.Halve();
	if (!Rect::AdjustRectangles(src_rect, dst_rect, src->GetRect()))
		return;
	dst_rect.Double();

	src_rect.Double();
	if (!Rect::AdjustRectangles(dst_rect, src_rect, GetRect()))
		return;
	src_rect.Halve();

	BitmapUtils* bm_utils = Begin(src);

	const uint8* src_pixels = src->pointer(src_rect.x, src_rect.y);
	uint8* dst_pixels = pointer(dst_rect.x, dst_rect.y);

	for (int i = 0; i < src_rect.height; i++) {
		const uint8* save = dst_pixels;
		bm_utils->Blit2x(dst_pixels, src_pixels, src_rect.width);
		dst_pixels += pitch();
		memcpy(dst_pixels, save, 2 * src_rect.width * bytes());
		dst_pixels += pitch();
		src_pixels += src->pitch();
	}

	End(src);
}

////////////////////////////////////////////////////////////
BitmapUtils* Surface::Begin() {
	Lock();

	BitmapUtils* bm_utils = BitmapUtils::Create(format, format, false);
	bm_utils->SetDstColorKey(colorkey());
	return bm_utils;
}

BitmapUtils* Surface::Begin(Bitmap* src) {
	Lock();
	src->Lock();

	BitmapUtils* bm_utils = BitmapUtils::Create(format, src->format, true);
	bm_utils->SetDstColorKey(colorkey());
	bm_utils->SetSrcColorKey(src->colorkey());
	return bm_utils;
}

void Surface::End() {
	Unlock();
	RefreshCallback();
}

void Surface::End(Bitmap* src) {
	src->Unlock();
	Unlock();
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

////////////////////////////////////////////////////////////
Rect Surface::TransformRectangle(const Matrix& m, const Rect& rect) {
	int sx0 = rect.x;
	int sy0 = rect.y;
	int sx1 = rect.x + rect.width;
	int sy1 = rect.y + rect.height;

	double x0, y0, x1, y1, x2, y2, x3, y3;
	m.Transform(sx0, sy0, x0, y0);
	m.Transform(sx1, sy0, x1, y1);
	m.Transform(sx1, sy1, x2, y2);
	m.Transform(sx0, sy1, x3, y3);

	double xmin = std::min(std::min(x0, x1), std::min(x2, x3));
	double ymin = std::min(std::min(y0, y1), std::min(y2, y3));
	double xmax = std::max(std::max(x0, x1), std::max(x2, x3));
	double ymax = std::max(std::max(y0, y1), std::max(y2, y3));

	int dx0 = (int) floor(xmin);
	int dy0 = (int) floor(ymin);
	int dx1 = (int) ceil(xmax);
	int dy1 = (int) ceil(ymax);

	return Rect(dx0, dy0, dx1 - dx0, dy1 - dy0);
}

////////////////////////////////////////////////////////////
Matrix Matrix::Setup(double angle,
					 double scale_x, double scale_y,
					 int src_pos_x, int src_pos_y,
					 int dst_pos_x, int dst_pos_y) {
	Matrix m = Matrix::Translation(-src_pos_x, -src_pos_y);
	m = m.PreMultiply(Matrix::Scale(scale_x, scale_y));
	m = m.PreMultiply(Matrix::Rotation(angle));
	m = m.PreMultiply(Matrix::Translation(dst_pos_x, dst_pos_y));
	return m;
}

