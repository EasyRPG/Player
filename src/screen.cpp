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
#include "data.h"
#include "rpg_savescreen.h"
#include "rpg_terrain.h"
#include "baseui.h"
#include "graphics.h"
#include "cache.h"
#include "screen.h"
#include "bitmap_screen.h"
#include "bitmap.h"
#include "color.h"
#include "game_screen.h"

Screen::Screen(RPG::SaveScreen& data) :
	ID(Graphics::drawable_id++), zobj(NULL), visible(true), data(data) {

	zobj = Graphics::RegisterZObj(z, ID);
	Graphics::RegisterDrawable(ID, this);
}

Screen::~Screen() {
	Graphics::RemoveZObj(ID);
	Graphics::RemoveDrawable(ID);
}

int Screen::GetZ() const {
	return z;
}

unsigned long Screen::GetId() const {
	return ID;
}

DrawableType Screen::GetType() const {
	return type;
}

void Screen::Update() {
}

void Screen::Draw(int /* z_order */) {
	if (!visible)
		return;

	BitmapRef dst = DisplayUi->GetDisplaySurface();

	Tone tone = Tone((int) ((data.tint_current_red) * 128 / 100),
		(int) ((data.tint_current_green) * 128 / 100),
		(int) ((data.tint_current_blue) * 128 / 100),
		(int) ((data.tint_current_sat) * 128 / 100));

	if (tone != Tone(128, 128, 128, 128)) {
		dst->ToneBlit(0, 0, *dst, Rect(0, 0, 320, 240), tone);
	}

	if (data.flash_time_left > 0) {
		BitmapRef flash = Bitmap::Create(320, 240, Color(data.flash_red * 255 / 31, data.flash_green * 255 / 31, data.flash_blue * 255 / 31, 255));
		dst->Blit(0, 0, *flash, flash->GetRect(), (int)(data.flash_current_level * 255 / 31));
	}
}
