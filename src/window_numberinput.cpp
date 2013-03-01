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
#include "window_numberinput.h"
#include "game_system.h"
#include "input.h"
#include "main_data.h"
#include "util_macro.h"
#include "bitmap.h"
#include "font.h"

#include <cstdio>

Window_NumberInput::Window_NumberInput(int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(ix, iy, iwidth, iheight), digits_max(6) {
	number = 0;

	SetContents(Bitmap::Create(width - 16, height - 16));
	contents->SetTransparentColor(windowskin->GetTransparentColor());
	cursor_width = 14;
	SetZ(10001);
	opacity = 0;
	index = 0;
	active = false;

	Refresh();
	UpdateCursorRect();
}

Window_NumberInput::~Window_NumberInput() {
}

void Window_NumberInput::Refresh() {
	contents->Clear();

	char s[7];
	// Copies digits_max numbers from number-string to s
	sprintf(s, "%0*d", digits_max, number);

	for (int i = 0; i < digits_max; ++i) {
		char c[2] = {s[i], '\0'};
		int x = i * (cursor_width - 2) + 12;
		contents->TextDraw(x, 2, Font::ColorDefault, c);
	}
}

int Window_NumberInput::GetNumber() {
	return number;
}

void Window_NumberInput::SetNumber(unsigned inumber) {
	unsigned num = 1;
	for (int i = 0; i < digits_max; ++i) {
		num *= 10;
	}
	number = min(max(inumber, (unsigned)0), num - 1);
	index = 0;
	Refresh();
}

int Window_NumberInput::GetMaxDigits() {
	return digits_max;
}

void Window_NumberInput::SetMaxDigits(int idigits_max) {
	// Only accepts values between 1 and 6 as RPGM2K
	digits_max =
		(idigits_max > 6) ? 6 :
		(idigits_max <= 0) ? 1 :
		idigits_max;
	index = 0;
	Refresh();
}

void Window_NumberInput::UpdateCursorRect() {
	cursor_rect.Set(index * (cursor_width - 2) + 8, 0, cursor_width, 16);
}

void Window_NumberInput::Update() {
	Window_Selectable::Update();
	if (active) {
		if (Input::IsRepeated(Input::DOWN) || Input::IsRepeated(Input::UP)) {
			Game_System::SePlay(Data::system.cursor_se);

			int place = 1;
			for (int i = 0; i < (digits_max - 1 - (int)index); ++i) {
				place *= 10;
			}
			int n = number / place % 10;
			number -= n * place;
			if (Input::IsRepeated(Input::UP)) {
				n = (n + 1) % 10;
			}
			if (Input::IsRepeated(Input::DOWN)) {
				n = (n + 9) % 10;
			}
			number += n * place;
			Refresh();
		}

		if (Input::IsRepeated(Input::RIGHT)) {
			if (digits_max >= 2) {
				Game_System::SePlay(Data::system.cursor_se);
				index = (index + 1) % digits_max;
			}
		}

		if (Input::IsRepeated(Input::LEFT)) {
			Game_System::SePlay(Data::system.cursor_se);
			index = (index + digits_max - 1) % digits_max;
		}

		UpdateCursorRect();
	}
}
