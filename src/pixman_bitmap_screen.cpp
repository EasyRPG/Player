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

#include "system.h"
#ifdef USE_PIXMAN_BITMAP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <cmath>
#include <iostream>
#include "pixman_bitmap_screen.h"
#include "pixman_bitmap.h"
#include "sdl_ui.h"

////////////////////////////////////////////////////////////
PixmanBitmapScreen::PixmanBitmapScreen(Bitmap* bitmap, bool delete_bitmap) :
	BitmapScreen(bitmap, delete_bitmap),
	bitmap_effects(NULL) {}

////////////////////////////////////////////////////////////
PixmanBitmapScreen::~PixmanBitmapScreen() {
	if (bitmap_effects != NULL)
		delete bitmap_effects;
}

////////////////////////////////////////////////////////////
void PixmanBitmapScreen::BlitScreen(int x, int y) {
	BlitScreen(x, y, Rect(0, 0, src_rect_effect.width, src_rect_effect.height));
}

////////////////////////////////////////////////////////////
void PixmanBitmapScreen::BlitScreen(int x, int y, Rect src_rect) {
	if (bitmap == NULL || (opacity_top_effect <= 0 && opacity_bottom_effect <= 0))
		return;

	src_rect = src_rect_effect.GetSubRect(src_rect);
	Refresh(src_rect);

	if (bitmap_effects == NULL) return;

	x -= origin_x;
	y -= origin_y;

	BlitScreenIntern(x, y, src_rect);
}

////////////////////////////////////////////////////////////
void PixmanBitmapScreen::BlitScreenTiled(Rect src_rect, Rect dst_rect) {
	if (bitmap == NULL || (opacity_top_effect <= 0 && opacity_bottom_effect <= 0))
		return;

	src_rect = src_rect_effect.GetSubRect(src_rect);
	Refresh(src_rect);

	int x1 = dst_rect.x + dst_rect.width;
	int y1 = dst_rect.y + dst_rect.height;
	for (int y = dst_rect.y; y < y1; y += src_rect.height) {
		for (int x = dst_rect.x; x < x1; x += src_rect.width) {
			Rect rect = src_rect;
			if (y + src_rect.height > y1)
				src_rect.height = y1 - y;
			if (x + src_rect.width > x1)
				src_rect.width = x1 - x;
			BlitScreenIntern(x, y, src_rect);
		}
	}
}

////////////////////////////////////////////////////////////
void PixmanBitmapScreen::BlitScreenIntern(int x, int y, Rect src_rect) {
	DisplaySdlUi->GetDisplaySurface()->Blit(x, y, bitmap_effects, src_rect, 255);
}

////////////////////////////////////////////////////////////
void PixmanBitmapScreen::Refresh(Rect& rect) {
	origin_x = 0;
	origin_y = 0;

	if (!needs_refresh && bitmap_effects != NULL && rect == bitmap_effects_src_rect) {
		rect = bitmap_effects->GetRect();
		return;
	}

	bitmap_effects_src_rect = rect;
	needs_refresh = false;

	if (bitmap == NULL)
		return;

	rect.Adjust(bitmap->GetWidth(), bitmap->GetHeight());

	if (rect.IsOutOfBounds(bitmap->GetWidth(), bitmap->GetHeight()))
		return;

	if (bitmap_effects != NULL) {
		delete bitmap_effects;
		bitmap_effects = NULL;
	}

	Surface *surface_effects = Surface::CreateSurface(rect.width, rect.height, true);

	surface_effects->Blit(0, 0, bitmap, rect, 255);
	surface_effects->ToneChange(tone_effect);
	surface_effects->Flip(flipx_effect, flipy_effect);

	if (opacity_top_effect < 255 && bush_effect < surface_effects->GetHeight()) {
		Rect src_rect = src_rect_effect;
		src_rect.height -= bush_effect;
		src_rect.x -= rect.x - src_rect_effect.x;
		src_rect.y -= rect.y - src_rect_effect.y;
		src_rect.Adjust(rect.width, rect.height);
		surface_effects->OpacityChange(opacity_top_effect, src_rect);
	}

	if (opacity_bottom_effect < 255 && bush_effect > 0) {
		Rect src_rect = src_rect_effect;
		src_rect.y += src_rect_effect.height - bush_effect;
		src_rect.height = bush_effect;
		src_rect.x -= rect.x - src_rect_effect.x;
		src_rect.y -= rect.y - src_rect_effect.y;
		src_rect.Adjust(rect.width, rect.height);
		surface_effects->OpacityChange(opacity_bottom_effect, src_rect);
	}

	bitmap_effects = surface_effects;
	rect = bitmap_effects->GetRect();

	if (zoom_x_effect == 1.0 && zoom_y_effect == 1.0 && angle_effect == 0.0 && waver_effect_depth == 0)
		return;

	int zoomed_width  = (int)(bitmap_effects->GetWidth()  * zoom_x_effect);
	int zoomed_height = (int)(bitmap_effects->GetHeight() * zoom_y_effect);
	Bitmap* fx2;

	if (angle_effect == 0.0) {
		fx2 = bitmap_effects->Resample(zoomed_width, zoomed_height, bitmap_effects->GetRect());
	}
	else {
		fx2 = bitmap_effects->RotateScale(
			angle_effect * 3.14159 / 180, zoomed_width, zoomed_height);
		origin_x = (fx2->GetWidth() - zoomed_width) / 2;
		origin_y = (fx2->GetHeight() - zoomed_height) / 2;
	}

	delete bitmap_effects;
	bitmap_effects = fx2;
	rect = bitmap_effects->GetRect();

	if (waver_effect_depth == 0)
		return;

	fx2 = bitmap_effects->Waver(waver_effect_depth, waver_effect_phase);

	delete bitmap_effects;
	bitmap_effects = fx2;
	rect = bitmap_effects->GetRect();
}

#endif

