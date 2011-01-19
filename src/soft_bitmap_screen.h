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

#ifndef _SOFT_BITMAP_SCREEN_H_
#define _SOFT_BITMAP_SCREEN_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "bitmap_screen.h"

////////////////////////////////////////////////////////////
/// SoftBitmapScreen class.
////////////////////////////////////////////////////////////
class SoftBitmapScreen : public BitmapScreen {
public:
	SoftBitmapScreen(Bitmap* source);
	SoftBitmapScreen(bool delete_bitmap);
	~SoftBitmapScreen();

	void BlitScreen(int x, int y);
	void BlitScreen(int x, int y, Rect src_rect);
	void BlitScreenTiled(Rect src_rect, Rect dst_rect);

protected:
	void BlitScreenIntern(int x, int y, Rect src_rect);

	Bitmap* bitmap_effects;
	int origin_x;
	int origin_y;

	void Refresh();
};

#endif
