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
#include <string>
#include "bitmap.h"
#include "color.h"
#include "game_screen.h"
#include "main_data.h"
#include "screen.h"
#include "drawable_mgr.h"

Screen::Screen() : Drawable(Priority_Screen)
{
	DrawableMgr::Register(this);
}

void Screen::Draw(Bitmap& dst) {
	auto flash_color = Main_Data::game_screen->GetFlashColor();
	if (flash_color.alpha > 0) {
		if (!flash) {
			flash = Bitmap::Create(dst.GetWidth(), dst.GetHeight(), flash_color);
		} else {
			flash->Fill(flash_color);
		}
		dst.Blit(0, 0, *flash, flash->GetRect(), 255);
	}

	if (viewport != Rect()) {
		// Clear all parts of the screen that are out-of-bounds
		Rect dst_rect = dst.GetRect();
		int dx = viewport.x - dst_rect.x;
		int dy = viewport.y - dst_rect.y;

		if (dx > 0) {
			// Left and Right
			dst.ClearRect({0, 0, dx, dst.GetHeight()});
			dst.ClearRect({dst.GetWidth() - dx, 0, dx, dst.GetHeight()});
		}

		if (dy > 0) {
			// Top and Bottom
			dst.ClearRect({0, 0, dst.GetWidth(), dy});
			dst.ClearRect({0, dst.GetHeight() - dy, dst.GetWidth(), dy});
		}
	}
}
