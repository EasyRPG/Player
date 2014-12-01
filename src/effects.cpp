/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

// Headers
#include <cmath>
#include "bitmap.h"

// Rotate, Zoom, Single Opacity
void Bitmap::EffectsBlit(const Matrix &fwd, Bitmap const& src, Rect const& src_rect, int opacity) {
	if (opacity <= 0)
		return;

	Rect dst_rect = TransformRectangle(fwd, src_rect);
	dst_rect.Adjust(GetRect());
	if (dst_rect.IsEmpty())
		return;

	Matrix inv = fwd.Inverse();

	TransformBlit(dst_rect, src, src_rect, inv, opacity);
}

// Rotate, Zoom, Split Opacity
void Bitmap::EffectsBlit(const Matrix &fwd, Bitmap const& src, Rect const& src_rect,
						  int top_opacity, int bottom_opacity, int opacity_split) {
	if (opacity_split <= 0)
		EffectsBlit(fwd, src, src_rect,  top_opacity);
	else if (opacity_split >= src_rect.height)
		EffectsBlit(fwd, src, src_rect,  bottom_opacity);
	else {
		Rect blit_rect = src_rect;
		blit_rect.height -= opacity_split;
		EffectsBlit(fwd, src, blit_rect,  top_opacity);

		blit_rect = src_rect;
		blit_rect.y = src_rect.height - opacity_split;
		blit_rect.height = opacity_split;
		EffectsBlit(fwd, src, blit_rect,  bottom_opacity);
	}
}

// Waver, Zoom, Single Opacity
void Bitmap::EffectsBlit(int x, int y, Bitmap const& src, Rect const& src_rect,
						   double zoom_x, double zoom_y,
						   int opacity,
						   int waver_depth, double waver_phase) {
	WaverBlit(x, y, zoom_x, zoom_y, src, src_rect, waver_depth, waver_phase, opacity);
}

// Waver, Zoom, Split Opacity
void Bitmap::EffectsBlit(int x, int y, Bitmap const& src, Rect const& src_rect,
						   int top_opacity, int bottom_opacity, int opacity_split,
						   double zoom_x, double zoom_y,
						   int waver_depth, double waver_phase) {
	if (opacity_split <= 0)
		EffectsBlit(x, y, src, src_rect, zoom_x, zoom_y,
					top_opacity, waver_depth, waver_phase);
	else if (opacity_split >= src_rect.height)
		EffectsBlit(x, y, src, src_rect, zoom_x, zoom_y,
					bottom_opacity, waver_depth, waver_phase);
	else {
		Rect blit_rect = src_rect;
		blit_rect.height -= opacity_split;
		EffectsBlit(x, y, src, blit_rect, zoom_x, zoom_y,
					top_opacity, waver_depth, waver_phase);

		blit_rect.y += blit_rect.height;
		blit_rect.height = opacity_split;
		y += static_cast<int>(std::floor((src_rect.height - opacity_split) * zoom_y));
		EffectsBlit(x, y, src, blit_rect, zoom_x, zoom_y,
					bottom_opacity, waver_depth, waver_phase);
	}
}

// Zoom, Single Opacity
void Bitmap::EffectsBlit(int x, int y, Bitmap const& src, Rect const& src_rect,
						   double zoom_x, double zoom_y,
						   int opacity) {
	if (zoom_x != 1.0 || zoom_y != 1.0) {
		Rect dst_rect(x, y, 
			static_cast<int>(std::floor(src_rect.width * zoom_x)),
			static_cast<int>(std::floor(src_rect.height * zoom_y)));
		StretchBlit(dst_rect, src, src_rect, opacity);
	}
	else
		Blit(x, y, src, src_rect, opacity);
}

// Zoom, Split Opacity
void Bitmap::EffectsBlit(int x, int y, Bitmap const& src, Rect const& src_rect,
						   double zoom_x, double zoom_y,
						   int top_opacity, int bottom_opacity, int opacity_split) {
	if (opacity_split <= 0)
		EffectsBlit(x, y, src, src_rect, zoom_x, zoom_y,
					top_opacity);
	else if (opacity_split >= src_rect.height)
		EffectsBlit(x, y, src, src_rect, zoom_x, zoom_y,
					bottom_opacity);
	else {
		Rect blit_rect = src_rect;
		blit_rect.height -= opacity_split;
		EffectsBlit(x, y, src, blit_rect, zoom_x, zoom_y,
					top_opacity);

		blit_rect.y += blit_rect.height;
		blit_rect.height = opacity_split;
		y += static_cast<int>(std::floor((src_rect.height - opacity_split) * zoom_y));
		EffectsBlit(x, y, src, blit_rect, zoom_x, zoom_y,
					bottom_opacity);
	}
}

// Single Opacity
void Bitmap::EffectsBlit(int x, int y, Bitmap const& src, Rect const& src_rect,
						   int opacity) {
	Blit(x, y, src, src_rect, opacity);
}

// Split Opacity
void Bitmap::EffectsBlit(int x, int y, Bitmap const& src, Rect const& src_rect,
						   int top_opacity, int bottom_opacity, int opacity_split) {
	if (opacity_split <= 0)
		EffectsBlit(x, y, src, src_rect, top_opacity);
	else if (opacity_split >= src_rect.height)
		EffectsBlit(x, y, src, src_rect, bottom_opacity);
	else {
		Rect blit_rect = src_rect;
		blit_rect.height -= opacity_split;
		EffectsBlit(x, y, src, blit_rect, top_opacity);

		blit_rect.y += blit_rect.height;
		blit_rect.height = opacity_split;
		y += src_rect.height - opacity_split;
		EffectsBlit(x, y, src, blit_rect, bottom_opacity);
	}
}

void Bitmap::EffectsBlit(int x, int y, Bitmap const& src, Rect const& src_rect,
						   int top_opacity, int bottom_opacity, int opacity_split,
						   double zoom_x, double zoom_y, double angle,
						   int waver_depth, double waver_phase) {
	bool rotate = angle != 0.0;
	bool scale = zoom_x != 1.0 || zoom_y != 1.0;
	bool waver = waver_depth != 0;

	if (waver) {
		EffectsBlit(x, y, src, src_rect,
					top_opacity, bottom_opacity, opacity_split,
					zoom_x, zoom_y,
					waver_depth, waver_phase);
	}
	else if (rotate) {
		Matrix fwd = Matrix::Setup(-angle, zoom_x, zoom_y,
			(src_rect.width - src_rect.x) / 2, (src_rect.height - src_rect.y) / 2, 
			x + src_rect.width * zoom_x / 2, y + src_rect.height * zoom_y / 2);
		EffectsBlit(fwd, src, src_rect, top_opacity, bottom_opacity, opacity_split);
	}
	else if (scale) {
		EffectsBlit(x, y, src, src_rect, zoom_x, zoom_y,
					top_opacity, bottom_opacity, opacity_split);
	}
	else {
		EffectsBlit(x, y, src, src_rect,
					top_opacity, bottom_opacity, opacity_split);
	}
}

// Tone, Zoom, Single Opacity
void Bitmap::EffectsBlit(int x, int y, Bitmap const& src, Rect const& src_rect_,
						   int opacity, const Tone& tone,
						   double zoom_x, double zoom_y) {
	Rect src_rect = src_rect_;
	bool scale = zoom_x != 1.0 || zoom_y != 1.0;
	bool tone_change = tone != Tone();

	Bitmap const* draw = &src;
	BitmapRef draw_;

	if (tone_change) {
		if (!scale && opacity < 255) {
			ToneBlit(x, y, src, src_rect, tone);
			return;
		}

		bool transparent = src.GetTransparent();
		draw_ = Create(src_rect.width, src_rect.height, transparent);
		if (transparent)
			draw_->Clear();
		draw_->ToneBlit(0, 0, src, src_rect, tone);
		draw = draw_.get();
		src_rect.x = 0;
		src_rect.y = 0;
	}

	if (scale) {
		Rect dst_rect(x, y, src_rect.width * zoom_x, src_rect.height * zoom_y);
		StretchBlit(dst_rect, *draw, src_rect, opacity);
	}
	else
		Blit(x, y, *draw, src_rect, opacity);
}
