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

////////////////////////////////////////////////////////////
/// SdlBitmapScreen class.
////////////////////////////////////////////////////////////
class SdlBitmapScreen : public BitmapScreen {
public:
	SdlBitmapScreen(Bitmap* source, bool delete_bitmap);
	~SdlBitmapScreen();

protected:
	void BlitScreenIntern(Bitmap* draw_bitmap, int x, int y, Rect src_rect, bool needs_scale);
	Bitmap* Refresh(Rect& rect, bool& needs_scale);
};

#endif

#endif
