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
#include "baseui.h"
#include "bitmap.h"
#include "color.h"
#include "game_screen.h"
#include "graphics.h"
#include "main_data.h"
#include "screen.h"

Screen::Screen() {
	Graphics::RegisterDrawable(this);

	default_tone = Tone(128, 128, 128, 128);
}

Screen::~Screen() {
	Graphics::RemoveDrawable(this);
}

int Screen::GetZ() const {
	return z;
}

DrawableType Screen::GetType() const {
	return type;
}

void Screen::Update() {
}

void Screen::Draw() {
	BitmapRef dst = DisplayUi->GetDisplaySurface();

	Tone tone = Main_Data::game_screen->GetTone();

	if (tone != default_tone) {
		dst->ToneBlit(0, 0, *dst, Rect(0, 0, 320, 240), tone);
	}

	int flash_time_left;
	int flash_current_level;
	Color flash_color = Main_Data::game_screen->GetFlash(flash_current_level, flash_time_left);

	if (flash_time_left > 0) {
		if (!flash) {
			flash = Bitmap::Create(320, 240, flash_color);
		} else {
			flash->Fill(flash_color);
		}
		dst->Blit(0, 0, *flash, flash->GetRect(), flash_current_level);
	}
}
