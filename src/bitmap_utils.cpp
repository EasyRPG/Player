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

#include <algorithm>
#include <cmath>
#include <cstring>
#include "system.h"
#include "bitmap.h"
#include "surface.h"
#include "pixel_format.h"
#include "bitmap_utils.h"

////////////////////////////////////////////////////////////
template <class PF>
Color BitmapUtils<PF>::GetPixel(Bitmap* src, int x, int y) {
	if (x < 0 || y < 0 || x >= src->width() || y >= src->height())
		return Color();

	src->Lock();

	const uint8* pixel = (const uint8*)src->pixels() + x * PF::bytes + y * src->pitch();
	uint8 r, g, b, a;
	PF::get_rgba(src->format, pixel, r, g, b, a);

	src->Unlock();

	return Color(r, g, b, a);
}

////////////////////////////////////////////////////////////
template <class PF>
Bitmap* BitmapUtils<PF>::Resample(Bitmap* src, int scale_w, int scale_h, const Rect& src_rect) {
	double zoom_x = (double)(scale_w) / src_rect.width;
	double zoom_y = (double)(scale_h) / src_rect.height;

	Surface* dst = Surface::CreateSurface(scale_w, scale_h, src->transparent);
	if (src->transparent)
		dst->SetTransparentColor(src->GetTransparentColor());

	src->Lock();
	dst->Lock();

	uint8* dst_pixels = (uint8*)dst->pixels();

	int pad = dst->pitch() - PF::bytes * dst->width();

	for (int i = 0; i < scale_h; i++) {
		const uint8* nearest_y = (const uint8*) src->pixels() + (src_rect.y + (int)(i / zoom_y)) * src->pitch();

		for (int j = 0; j < scale_w; j++) {
			const uint8* nearest_match = nearest_y + (src_rect.x + (int)(j / zoom_x)) * PF::bytes;
			PF::copy_pixel(dst_pixels, nearest_match);

			dst_pixels += PF::bytes;
		}
		dst_pixels += pad;
	}

	src->Unlock();
	dst->Unlock();

	return dst;
}

////////////////////////////////////////////////////////////
template <class PF>
Bitmap* BitmapUtils<PF>::RotateScale(Bitmap* src, double angle, int scale_w, int scale_h) {
	double c = cos(-angle);
	double s = sin(-angle);
	int w = src->width();
	int h = src->height();
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
	const Color& trans = src->transparent ? src->GetTransparentColor() : Color(255,0,255,0);
	result->SetTransparentColor(trans);
	result->Fill(trans);

	src->Lock();
	result->Lock();

	const uint8* src_pixels = (const uint8*)src->pixels();
	uint8* dst_pixels = (uint8*)result->pixels();
	int src_pitch = src->pitch();
	int pad = result->pitch() - result->width() * PF::bytes;

	for (int i = 0; i < dst_h; i++) {
		for (int j = 0; j < dst_w; j++) {
			double x = ix0 + ixy * (i + 0.5) + ixx * (j + 0.5);
			double y = iy0 + iyy * (i + 0.5) + iyx * (j + 0.5);
			int xi = (int) floor(x);
			int yi = (int) floor(y);
			if (xi < 0 || xi >= w || yi < 0 || yi >= h)
				;
			else
				PF::copy_pixel(dst_pixels, &src_pixels[yi * src_pitch + xi * PF::bytes]);
			dst_pixels += PF::bytes;
		}
		dst_pixels += pad;
	}

	src->Unlock();
	result->Unlock();

	return result;
}

////////////////////////////////////////////////////////////
template <class PF>
Bitmap* BitmapUtils<PF>::Waver(Bitmap* src, int depth, double phase) {
	Surface* dst = Surface::CreateSurface(src->width() + 2 * depth, src->height(), true);

	if (src->transparent)
		dst->SetTransparentColor(src->GetTransparentColor());

	dst->Clear();

	src->Lock();
	dst->Lock();

	const uint8* src_pixels = (const uint8*)src->pixels();
	uint8* dst_pixels = (uint8*)dst->pixels();

	for (int y = 0; y < src->height(); y++) {
		int offset = (int) (depth * (1 + sin((phase + y * 20) * 3.14159 / 180)));

		PF::copy_pixels(&dst_pixels[offset * PF::bytes], src_pixels, src->width());

		src_pixels += src->pitch();
		dst_pixels += dst->pitch();
	}

	src->Unlock();
	dst->Unlock();

	return dst;
}

////////////////////////////////////////////////////////////
template <class PF>
void BitmapUtils<PF>::SetPixel(Surface* dst, int x, int y, const Color &color) {
	if (x < 0 || y < 0 || x >= dst->width() || y >= dst->height()) return;

	dst->Lock();

	uint8* dst_pixel = (uint8*) dst->pixels() + x * PF::bytes + y * dst->pitch();
	PF::set_rgba(dst->format, dst_pixel, color.red, color.green, color.blue, color.alpha);

	dst->Unlock();

	dst->RefreshCallback();
}

////////////////////////////////////////////////////////////
template <class PF>
void BitmapUtils<PF>::Blit(Surface* dst, int x, int y, Bitmap* src, Rect src_rect, int opacity) {
	if (opacity < 0) return;

	Rect dst_rect(x, y, 0, 0);

	if (!Rect::AdjustRectangles(src_rect, dst_rect, src->GetRect()))
		return;
	if (!Rect::AdjustRectangles(dst_rect, src_rect, dst->GetRect()))
		return;

	if (opacity > 255) opacity = 255;

	dst->Lock();
	src->Lock();

	if (PF::has_alpha || opacity < 255) {
		const uint8* src_pixels = (uint8*)src->pixels() + src_rect.x * PF::bytes + src_rect.y * src->pitch();
		uint8* dst_pixels = (uint8*)dst->pixels() + dst_rect.x * PF::bytes + dst_rect.y * dst->pitch();
		int src_pad = src->pitch() - dst_rect.width * PF::bytes;
		int dst_pad = dst->pitch() - dst_rect.width * PF::bytes;

		for (int i = 0; i < dst_rect.height; i++) {
			for (int j = 0; j < dst_rect.width; j++) {
				uint8 sr, sg, sb, sa;
				PF::get_rgba(src->format, src_pixels, sr, sg, sb, sa);
				uint8 dr, dg, db, da;
				PF::get_rgba(dst->format, dst_pixels, dr, dg, db, da);
				int srca = (int) sa * opacity / 255;

				uint8 rr = (uint8) ((dr * (255 - srca) + sr * srca) / 255);
				uint8 rg = (uint8) ((dg * (255 - srca) + sg * srca) / 255);
				uint8 rb = (uint8) ((db * (255 - srca) + sb * srca) / 255);
				uint8 ra = (uint8) ((da * (255 - srca)) / 255 + srca);

				PF::set_rgba(dst->format, dst_pixels, rr, rg, rb, ra);

				src_pixels += PF::bytes;
				dst_pixels += PF::bytes;
			}

			src_pixels += src_pad;
			dst_pixels += dst_pad;
		}
	} else if (PF::has_colorkey) {
		const uint8* src_pixels = (uint8*)src->pixels() + src_rect.x * PF::bytes + src_rect.y * src->pitch();
		uint8* dst_pixels = (uint8*)dst->pixels() + dst_rect.x * PF::bytes + dst_rect.y * dst->pitch();

		int src_pad = src->pitch() - dst_rect.width * PF::bytes;
		int dst_pad = dst->pitch() - dst_rect.width * PF::bytes;

		for (int i = 0; i < dst_rect.height; i++) {
			for (int j = 0; j < dst_rect.width; j++) {
				if (PF::get_uint32(src_pixels) != src->format.colorkey)
					PF::copy_pixel(dst_pixels, src_pixels);

				src_pixels += PF::bytes;
				dst_pixels += PF::bytes;
			}

			src_pixels += src_pad;
			dst_pixels += dst_pad;
		}
	} else {
		const uint8* src_pixels = (uint8*)src->pixels() + src_rect.x * PF::bytes + src_rect.y * src->pitch();
		uint8* dst_pixels = (uint8*)dst->pixels() + dst_rect.x * PF::bytes + dst_rect.y * dst->pitch();

		for (int i = 0; i < dst_rect.height; i++) {
			memcpy(dst_pixels, src_pixels, dst_rect.width);

			src_pixels += src->pitch();
			dst_pixels += dst->pitch();
		}
	}

	src->Unlock();
	dst->Unlock();

	dst->RefreshCallback();
}

////////////////////////////////////////////////////////////
template <class PF>
void BitmapUtils<PF>::TiledBlit(Surface* dst, Rect src_rect, Bitmap* src, Rect dst_rect, int opacity) {
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
			dst->Blit(dst_rect.x + i * src_rect.width, dst_rect.y + j * src_rect.height, src, tile, opacity);
		}
	}
}


////////////////////////////////////////////////////////////
template <class PF>
void BitmapUtils<PF>::TiledBlit(Surface* dst, int ox, int oy, Rect src_rect, Bitmap* src, Rect dst_rect, int opacity) {
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

			dst->Blit(dst_x, dst_y, src, tile, opacity);
		}
	}
}

////////////////////////////////////////////////////////////
template <class PF>
void BitmapUtils<PF>::StretchBlit(Surface* dst, Bitmap* src, Rect src_rect, int opacity) {
	if (src_rect.width == dst->width() && src_rect.height == dst->height()) {
		dst->Blit(0, 0, src, src_rect, opacity);
	} else {
		dst->StretchBlit(dst->GetRect(), src, src_rect, opacity);
	}
}

////////////////////////////////////////////////////////////
template <class PF>
void BitmapUtils<PF>::StretchBlit(Surface* dst, Rect dst_rect, Bitmap* src, Rect src_rect, int opacity) {
	if (src_rect.width == dst_rect.width && src_rect.height == dst_rect.height) {
		dst->Blit(dst_rect.x, dst_rect.y, src, src_rect, opacity);
	} else {
		src_rect.Adjust(src->width(), src->height());
		if (src_rect.IsOutOfBounds(src->width(), src->height())) return;

		if (dst_rect.IsOutOfBounds(dst->width(), dst->height())) return;

		Bitmap* resampled = src->Resample(dst_rect.width, dst_rect.height, src_rect);

		dst->Blit(dst_rect.x, dst_rect.y, resampled, resampled->GetRect(), opacity);

		delete resampled;
	}
}

////////////////////////////////////////////////////////////
template <class PF>
void BitmapUtils<PF>::Mask(Surface* dst, int x, int y, Bitmap* src, Rect src_rect) {
	Rect dst_rect(x, y, 0, 0);

	if (!Rect::AdjustRectangles(src_rect, dst_rect, src->GetRect()))
		return;
	if (!Rect::AdjustRectangles(dst_rect, src_rect, dst->GetRect()))
		return;

	dst->Lock();
	src->Lock();

	const uint8* src_pixels = (const uint8*) src->pixels() + src_rect.y * src->pitch() + src_rect.x * PF::bytes;
	uint8* dst_pixels = (uint8*) dst->pixels() + dst_rect.y * dst->pitch() + dst_rect.x * PF::bytes;
	int src_pad = src->pitch() - dst_rect.width * PF::bytes;
	int dst_pad = dst->pitch() - dst_rect.width * PF::bytes;

	for (int j = 0; j < dst_rect.height; j++) {
		for (int i = 0; i < dst_rect.width; i++) {
			uint8 sa = PF::get_alpha(src->format, src_pixels);
			uint8 da = PF::get_alpha(dst->format, dst_pixels);
			uint8 ra = std::min(sa, da);
			PF::set_alpha(dst->format, dst_pixels, ra);
			src_pixels += PF::bytes;
			dst_pixels += PF::bytes;
		}
		src_pixels += src_pad;
		dst_pixels += dst_pad;
	}

	src->Unlock();
	dst->Unlock();

	dst->RefreshCallback();
}

////////////////////////////////////////////////////////////
template <class PF>
void BitmapUtils<PF>::Fill(Surface* dst, const Color &color) {
	dst->Lock();

	uint8 pixel[4];
	PF::set_rgba(dst->format, pixel, color.red, color.green, color.blue, color.alpha);

	uint8* dst_pixels = (uint8*)dst->pixels();

	if (dst->pitch() == dst->width() * PF::bytes) {
		PF::set_pixels(dst_pixels, pixel, dst->height() * dst->width());
	} else {
		for (int i = 0; i < dst->height(); i++) {
			PF::set_pixels(dst_pixels, pixel, dst->width());
			dst_pixels += dst->pitch();
		}
	}

	dst->Unlock();

	dst->RefreshCallback();
}

////////////////////////////////////////////////////////////
template <class PF>
void BitmapUtils<PF>::FillRect(Surface* dst, Rect dst_rect, const Color &color) {
	dst_rect.Adjust(dst->width(), dst->height());
	if (dst_rect.IsOutOfBounds(dst->width(), dst->height()))
		return;

	dst->Lock();

	uint8 pixel[4];
	PF::set_rgba(dst->format, pixel, color.red, color.green, color.blue, color.alpha);

	uint8* dst_pixels = (uint8*)dst->pixels() + dst_rect.y * dst->pitch() + dst_rect.x *  PF::bytes;

	for (int i = 0; i < dst_rect.height; i++) {
		PF::set_pixels(dst_pixels, pixel, dst_rect.width);
		dst_pixels += dst->pitch();
	}

	dst->Unlock();

	dst->RefreshCallback();
}

////////////////////////////////////////////////////////////
template <class PF>
void BitmapUtils<PF>::Clear(Surface *dst) {
	dst->Fill(dst->GetTransparentColor());
}

////////////////////////////////////////////////////////////
template <class PF>
void BitmapUtils<PF>::ClearRect(Surface *dst, Rect dst_rect) {
	dst->FillRect(dst_rect, dst->GetTransparentColor());
}

////////////////////////////////////////////////////////////
template <class PF>
void BitmapUtils<PF>::HueChange(Surface *dst, double hue) {
	dst->HSLChange(hue, 1, 1, 0, dst->GetRect());
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

template <class PF>
void BitmapUtils<PF>::HSLChange(Surface* dst, double h, double s, double l, double lo, Rect dst_rect) {
	dst_rect.Adjust(dst->width(), dst->height());
	if (dst_rect.IsOutOfBounds(dst->width(), dst->height()))
		return;

	int hue  = (int) (h / 60.0 * 0x100);
	int sat  = (int) (s * 0x100);
	int lum  = (int) (l * 0x100);
	int loff = (int) (lo * 0x100);

	if (hue < 0)
		hue += ((-hue + 0x5FF) / 0x600) * 0x600;
	else if (hue > 0x600)
		hue -= (hue / 0x600) * 0x600;

	dst->Lock();

	uint8* dst_pixels = (uint8*) dst->pixels() + dst_rect.x * PF::bytes + dst_rect.y * dst->pitch();
	int pad = dst->pitch() - dst_rect.width * PF::bytes;

	for (int i = 0; i < dst_rect.height; i++) {
		for (int j = 0; j < dst_rect.width; j++) {
			uint8 r, g, b, a;
			PF::get_rgba(dst->format, dst_pixels, r, g, b, a);
			if (a == 0)
				continue;
			RGB_adjust_HSL(r, g, b, hue, sat, lum, loff);
			PF::set_rgba(dst->format, dst_pixels, r, g, b, a);

			dst_pixels += PF::bytes;
		}
		dst_pixels += pad;
	}

	dst->Unlock();

	dst->RefreshCallback();
}

////////////////////////////////////////////////////////////
template <class PF>
void BitmapUtils<PF>::ToneChange(Surface* dst, const Tone &tone) {
	if (tone == Tone()) return;

	dst->Lock();

	uint8* dst_pixels = (uint8*)dst->pixels();
	int pad = dst->pitch() - dst->width() * PF::bytes;

	if (tone.gray == 0) {
		for (int i = 0; i < dst->height(); i++) {
			for (int j = 0; j < dst->width(); j++) {
				uint8 r, g, b, a;
				PF::get_rgba(dst->format, dst_pixels, r, g, b, a);
				if (a == 0) {
					dst_pixels += PF::bytes;
					continue;
				}

				r = (uint8)std::max(std::min(r + tone.red,   255), 0);
				g = (uint8)std::max(std::min(g + tone.green, 255), 0);
				b = (uint8)std::max(std::min(b + tone.blue,  255), 0);

				PF::set_rgba(dst->format, dst_pixels, r, g, b, a);

				dst_pixels += PF::bytes;
			}
			dst_pixels += pad;
		}
	} else {
		double factor = (255 - tone.gray) / 255.0;
		for (int i = 0; i < dst->height(); i++) {
			for (int j = 0; j < dst->width(); j++) {
				uint8 r, g, b, a;
				PF::get_rgba(dst->format, dst_pixels, r, g, b, a);
				if (dst->transparent && a == 0) {
					dst_pixels += PF::bytes;
					continue;
				}

				double gray = r * 0.299 + g * 0.587 + b * 0.114;

				r = (uint8)std::max(std::min((r - gray) * factor + gray + tone.red   + 0.5, 255.0), 0.0);
				g = (uint8)std::max(std::min((g - gray) * factor + gray + tone.green + 0.5, 255.0), 0.0);
				b = (uint8)std::max(std::min((b - gray) * factor + gray + tone.blue  + 0.5, 255.0), 0.0);

				PF::set_rgba(dst->format, dst_pixels, r, g, b, a);

				dst_pixels += PF::bytes;
			}
			dst_pixels += pad;
		}
	}
	
	dst->Unlock();

	dst->RefreshCallback();
}

////////////////////////////////////////////////////////////
template <class PF>
void BitmapUtils<PF>::Flip(Surface* dst, bool horizontal, bool vertical) {
	if (!horizontal && !vertical) return;

	dst->Lock();

	if (horizontal && vertical) {
		int pad = dst->pitch() - dst->width() * PF::bytes;

		uint8* dst_pixels_first = (uint8*)dst->pixels();
		uint8* dst_pixels_last = (uint8*)dst->pixels() + (dst->width() - 1) * PF::bytes + (dst->height() - 1) * dst->pitch();

		uint8 tmp_buffer[4];

		for (int i = 0; i < dst->height() / 2; i++) {
			for (int j = 0; j < dst->width(); j++) {
				if (dst_pixels_first == dst_pixels_last)
					break;
				PF::copy_pixel(tmp_buffer, dst_pixels_first);
				PF::copy_pixel(dst_pixels_first, dst_pixels_last);
				PF::copy_pixel(dst_pixels_last, tmp_buffer);

				dst_pixels_first += PF::bytes;
				dst_pixels_last -= PF::bytes;
			}
			dst_pixels_first += pad;
			dst_pixels_last += pad;
		}
	} else if (horizontal) {
		int pad_left = (dst->width() - dst->width() / 2) * PF::bytes;
		int pad_right = (dst->width() + dst->width() / 2) * PF::bytes;

		uint8* dst_pixels_left = (uint8*)dst->pixels();
		uint8* dst_pixels_right = (uint8*)dst->pixels() + (dst->width() - 1) * PF::bytes;

		uint8 tmp_buffer[4];

		for (int i = 0; i < dst->height(); i++) {
			for (int j = 0; j < dst->width() / 2; j++) {
				if (dst_pixels_left == dst_pixels_right) continue;

				PF::copy_pixel(tmp_buffer, dst_pixels_left);
				PF::copy_pixel(dst_pixels_left, dst_pixels_right);
				PF::copy_pixel(dst_pixels_right, tmp_buffer);

				dst_pixels_left += PF::bytes;
				dst_pixels_right -= PF::bytes;
			}
			dst_pixels_left += pad_left;
			dst_pixels_right += pad_right;
		}
	} else {
		uint8* dst_pixels_up = (uint8*)dst->pixels();
		uint8* dst_pixels_down = (uint8*)dst->pixels() + (dst->height() - 1) * dst->pitch();
		int width = dst->width();
		uint8* tmp_buffer = new uint8[width * PF::bytes];

		for (int i = 0; i < dst->height() / 2; i++) {
			if (dst_pixels_up == dst_pixels_down)
				break;

			PF::copy_pixels(tmp_buffer, dst_pixels_down, width);
			PF::copy_pixels(dst_pixels_down, dst_pixels_up, width);
			PF::copy_pixels(dst_pixels_up, tmp_buffer, width);

			dst_pixels_up += dst->pitch();
			dst_pixels_down -= dst->pitch();
		}

		delete tmp_buffer;
	}

	dst->Unlock();

	dst->RefreshCallback();
}

////////////////////////////////////////////////////////////
template <class PF>
void BitmapUtils<PF>::OpacityChange(Surface* dst, int opacity, const Rect& src_rect) {
	if (opacity == 255)
		return;

	dst->Lock();

	uint8* dst_pixels = (uint8*) dst->pixels() + src_rect.y * dst->pitch() + src_rect.x * PF::bytes;
	int pad = dst->pitch() - dst->width() * PF::bytes;

	for (int j = 0; j < src_rect.height; j++) {
		for (int i = 0; i < src_rect.width; i++) {
			uint8 a = PF::get_alpha(dst->format, dst_pixels);
			a = (uint8) ((a * opacity) / 255);
			PF::set_alpha(dst->format, dst_pixels, a);
			dst_pixels += PF::bytes;
		}

		dst_pixels += pad;
	}
	
	dst->Unlock();

	dst->RefreshCallback();
}

template class BitmapUtils<PixelFormat<32,false,true,false, true, 8,16,8,8,8,0,8,24> >;
template class BitmapUtils<PixelFormat<32,false,true,false, true, 8,8,8,16,8,24,8,0> >;
template class BitmapUtils<PixelFormat<16,true, true, false,false,0,0,0,0,0,0,0,0> >;
template class BitmapUtils<PixelFormat<24,true, true, false,true, 0,0,0,0,0,0,0,0> >;
template class BitmapUtils<PixelFormat<32,true, true, false,true, 0,0,0,0,0,0,0,0> >;
template class BitmapUtils<PixelFormat<16,true, false,true ,false,0,0,0,0,0,0,0,0> >;
template class BitmapUtils<PixelFormat<24,true, false,true ,true, 0,0,0,0,0,0,0,0> >;
template class BitmapUtils<PixelFormat<32,true, false,true ,true, 0,0,0,0,0,0,0,0> >;
template class BitmapUtils<PixelFormat<16,true, false,false,false,0,0,0,0,0,0,0,0> >;
template class BitmapUtils<PixelFormat<24,true, false,false,true, 0,0,0,0,0,0,0,0> >;
template class BitmapUtils<PixelFormat<32,true, false,false,true, 0,0,0,0,0,0,0,0> >;
