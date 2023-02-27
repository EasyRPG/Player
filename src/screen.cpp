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
			flash = Bitmap::Create(Player::screen_width, Player::screen_height, flash_color);
		} else {
			flash->Fill(flash_color);
		}
		dst.Blit(0, 0, *flash, flash->GetRect(), 255);
	}
}
