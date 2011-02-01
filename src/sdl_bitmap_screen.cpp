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
#ifdef USE_SDL_BITMAP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "sdl_bitmap_screen.h"
#include "bitmap_utils.h"
#include "baseui.h"

////////////////////////////////////////////////////////////
SdlBitmapScreen::SdlBitmapScreen(Bitmap* bitmap, bool delete_bitmap) :
	BitmapScreen(bitmap, delete_bitmap) {}

////////////////////////////////////////////////////////////
SdlBitmapScreen::~SdlBitmapScreen() {}

////////////////////////////////////////////////////////////
void SdlBitmapScreen::BlitScreenIntern(Bitmap* draw_bitmap, int x, int y, Rect src_rect) {
	#ifdef USE_ALPHA
	BitmapScreen::BlitScreenIntern(draw_bitmap, x, y, src_rect);
	#else
	if ((opacity_top_effect == 255 || bush_effect >= src_rect_effect.height) &&
		(opacity_bottom_effect == 255 || bush_effect <= 0)) {
		BitmapScreen::BlitScreenIntern(draw_bitmap, x, y, src_rect);
		return;
	}

	if (!draw_bitmap)
		return;

	Surface* temp = NULL;

	if (angle_effect != 0.0) {
		Matrix fwd = Matrix::Setup(
			angle_effect * 3.14159 / 180, zoom_x_effect, zoom_y_effect,
			src_rect.x, src_rect.y, x, y);
		Rect rect = BitmapUtils::TransformRectangle(fwd, Rect(0, 0, src_rect.width, src_rect.height));
		//rect.Adjust(dst->GetRect());
		//if (rect.IsEmpty())
		//	return;
		fwd.PreMultiply(Matrix::Translation(-rect.x, -rect.y));
		Matrix inv = fwd.Inverse();
		Surface* temp = Surface::CreateSurface(rect.width, rect.height);
		temp->Clear();
		temp->TransformBlit(temp->GetRect(), draw_bitmap, src_rect, inv);
		x += rect.x;
		y += rect.y;
		draw_bitmap = temp;
	}
	else if (waver_effect_depth > 0) {
		Surface* temp = Surface::CreateSurface(src_rect.width + 2 * waver_effect_depth, src_rect.height);
		temp->Clear();
		temp->WaverBlit(waver_effect_depth, 0, draw_bitmap, src_rect, waver_effect_depth, waver_effect_phase);
		x -= waver_effect_depth;
		draw_bitmap = temp;
	}

	Surface* dst = DisplayUi->GetDisplaySurface();

	if (bush_effect < src_rect_effect.height) {
		Rect blit_rect = src_rect;
		blit_rect.height -= bush_effect;
		dst->Blit(x, y, draw_bitmap, blit_rect, opacity_top_effect);
	}

	if (bush_effect > 0) {
		Rect blit_rect = src_rect;
		blit_rect.y = src_rect.height - bush_effect;
		blit_rect.height = bush_effect;
		dst->Blit(x, y, draw_bitmap, blit_rect, opacity_bottom_effect);
	}

	if (temp != NULL)
		delete temp;

	#endif
}

////////////////////////////////////////////////////////////
Bitmap* SdlBitmapScreen::Refresh(Rect& rect) {
	#ifdef USE_ALPHA
	return BitmapScreen::Refresh(rect);
	#else
	int save_opacity_top_effect = opacity_top_effect;
	int save_opacity_bottom_effect = opacity_bottom_effect;

	opacity_top_effect = 255;
	opacity_bottom_effect = 255;

	Bitmap* result = BitmapScreen::Refresh(rect);

	opacity_top_effect = save_opacity_top_effect;
	opacity_bottom_effect = save_opacity_bottom_effect;

	return result;
	#endif
}

#endif

