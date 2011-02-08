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
#include "sdl_bitmap.h"
#include "bitmap_utils.h"
#include "baseui.h"

////////////////////////////////////////////////////////////
SdlBitmapScreen::SdlBitmapScreen(Bitmap* bitmap, bool delete_bitmap) :
	BitmapScreen(bitmap, delete_bitmap) {}

////////////////////////////////////////////////////////////
SdlBitmapScreen::~SdlBitmapScreen() {}

////////////////////////////////////////////////////////////
void SdlBitmapScreen::BlitScreenIntern(Bitmap* draw_bitmap, int x, int y, Rect src_rect, bool needs_scale) {
}

////////////////////////////////////////////////////////////
Bitmap* SdlBitmapScreen::Refresh(Rect& rect, bool& needs_scale, int& bush_y) {
	return BitmapScreen::Refresh(rect, needs_scale, bush_y);
}

#endif

