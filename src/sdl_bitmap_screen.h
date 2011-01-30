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

#ifndef _SDL_BITMAP_SCREEN_H_
#define _SDL_BITMAP_SCREEN_H_

#include "system.h"
#ifdef USE_SDL_BITMAP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "bitmap_screen.h"
#include "surface.h"
#include "SDL.h"

////////////////////////////////////////////////////////////
/// SdlBitmapScreen class.
////////////////////////////////////////////////////////////
class SdlBitmapScreen : public BitmapScreen {
public:
	SdlBitmapScreen(Bitmap* source, bool delete_bitmap);
	~SdlBitmapScreen();

	void SetBitmap(Bitmap* bitmap, bool delete_bitmap);

	void BlitScreen(int x, int y);
	void BlitScreen(int x, int y, Rect src_rect);
	void BlitScreenTiled(Rect src_rect, Rect dst_rect);

	void ClearEffects();

	void SetSrcRect(Rect src_rect);

protected:
	static void BlitScreenIntern(SDL_Surface* surface, int x, int y, Rect src_rect, int opacity);

	void Refresh(Rect& rect);

	void CalcRotatedSize(int &width, int &height);
	void CalcZoomedSize(int &width, int &height);

	Bitmap* bitmap_effects;
	Rect bitmap_effects_src_rect;
	Rect bitmap_effects_rect;
	int origin_x;
	int origin_y;
};

#endif

#endif
