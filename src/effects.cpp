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

// Rotate, Zoom, Opacity
void Bitmap::EffectsBlit(const Matrix &fwd, Bitmap const& src, Rect const& src_rect, Opacity const& opacity) {
	if (opacity.IsTransparent())
		return;

	Rect dst_rect = TransformRectangle(fwd, src_rect);
	dst_rect.Adjust(GetRect());
	if (dst_rect.IsEmpty())
		return;

	Matrix inv = fwd.Inverse();

	TransformBlit(dst_rect, src, src_rect, inv, opacity);
}

// Waver, Zoom, Opacity
void Bitmap::EffectsBlit(int x, int y, int ox, int oy,
						   Bitmap const& src, Rect const& src_rect,
						   Opacity const& opacity,
						   double zoom_x, double zoom_y,
						   int waver_depth, double waver_phase) {
	WaverBlit(x - ox * zoom_x, y - oy * zoom_y, zoom_x, zoom_y, src, src_rect,
				waver_depth, waver_phase, opacity);
}

// Zoom, Opacity
void Bitmap::EffectsBlit(int x, int y, int ox, int oy,
						   Bitmap const& src, Rect const& src_rect,
						   double zoom_x, double zoom_y,
						   Opacity const& opacity) {
	if (zoom_x != 1.0 || zoom_y != 1.0) {
		Rect dst_rect(
			x - static_cast<int>(std::floor(ox * zoom_x)),
			y - static_cast<int>(std::floor(oy * zoom_y)),
			static_cast<int>(std::floor(src_rect.width * zoom_x)),
			static_cast<int>(std::floor(src_rect.height * zoom_y)));
		StretchBlit(dst_rect, src, src_rect, opacity);
	}
	else
		Blit(x - ox, y - oy, src, src_rect, opacity);
}

// Opacity
void Bitmap::EffectsBlit(int x, int y, int ox, int oy,
						   Bitmap const& src, Rect const& src_rect,
						   Opacity const& opacity) {
	Blit(x - ox, y - oy, src, src_rect, opacity);
}

void Bitmap::EffectsBlit(int x, int y, int ox, int oy,
						   Bitmap const& src, Rect const& src_rect,
						   Opacity const& opacity,
						   double zoom_x, double zoom_y, double angle,
						   int waver_depth, double waver_phase) {
	bool rotate = angle != 0.0;
	bool scale = zoom_x != 1.0 || zoom_y != 1.0;
	bool waver = waver_depth != 0;

	if (waver) {
		EffectsBlit(x, y, ox, oy, src, src_rect,
					opacity,
					zoom_x, zoom_y,
					waver_depth, waver_phase);
	}
	else if (rotate) {
		Matrix fwd = Matrix::Setup(-angle, zoom_x, zoom_y, ox, oy, x, y);
		EffectsBlit(fwd, src, src_rect, opacity);
	}
	else if (scale) {
		EffectsBlit(x, y, ox, oy, src, src_rect, zoom_x, zoom_y, opacity);
	}
	else {
		EffectsBlit(x, y, ox, oy, src, src_rect, opacity);
	}
}

// Tone, Zoom, Opacity
void Bitmap::EffectsBlit(int x, int y, int ox, int oy, Bitmap const& src, Rect const& src_rect_,
						   Opacity const& opacity, const Tone& tone,
						   double zoom_x, double zoom_y) {
	Rect src_rect = src_rect_;
	bool scale = zoom_x != 1.0 || zoom_y != 1.0;
	bool tone_change = tone != Tone();

	Bitmap const* draw = &src;
	BitmapRef draw_;

	if (tone_change) {
		if (!scale && !opacity.IsOpaque()) {
			ToneBlit(x - ox, y - oy, src, src_rect, tone, opacity);
			return;
		}

		bool transparent = src.GetTransparent();
		draw_ = Create(src_rect.width, src_rect.height, transparent);
		if (transparent)
			draw_->Clear();
		draw_->ToneBlit(0, 0, src, src_rect, tone, opacity);
		draw = draw_.get();
		src_rect.x = 0;
		src_rect.y = 0;
	}

	if (scale) {
		Rect dst_rect(x - ox * zoom_x, y - oy * zoom_y,
					  src_rect.width * zoom_x, src_rect.height * zoom_y);
		StretchBlit(dst_rect, *draw, src_rect, opacity);
	}
	else
		Blit(x - ox, y - oy, *draw, src_rect, opacity);
}
