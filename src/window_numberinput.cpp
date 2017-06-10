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
#include "player.h"

#include <cstdio>

Window_NumberInput::Window_NumberInput(int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(ix, iy, iwidth, iheight),
	digits_max(Player::IsRPG2k() ? 6 : 7) {
	number = 0;
	plus = true;

	SetContents(Bitmap::Create(width - 16, height - 16));
	cursor_width = 14;
	// Above the message window
	SetZ(Priority_Window + 150);
	opacity = 0;
	index = 0;
	active = false;
	show_operator = false;

	Refresh();
	UpdateCursorRect();
}

void Window_NumberInput::Refresh() {
	contents->Clear();

	char s[9];
	// Copies digits_max numbers from number-string to s
	std::string format_string = "";
	if (show_operator) {
		format_string += plus ? "+" : "-";
	}

	format_string += "%0*d";

	sprintf(s, format_string.c_str(), digits_max, number);

	for (int i = 0; i < digits_max + (int)show_operator; ++i) {
		char c[2] = {s[i], '\0'};
		int x = i * (cursor_width - 2) + (show_operator ? 2 : 12);
		contents->TextDraw(x, 2, Font::ColorDefault, c);
	}
}

int Window_NumberInput::GetNumber() {
	return number * (plus ? 1 : -1);
}

void Window_NumberInput::SetNumber(int inumber) {
	int num = 1;
	for (int i = 0; i < digits_max; ++i) {
		num *= 10;
	}
	number = min(max(abs(inumber), 0), num - 1);
	index = 0;

	plus = inumber >= 0;

	UpdateCursorRect();
	Refresh();
}

int Window_NumberInput::GetMaxDigits() {
	return digits_max;
}

void Window_NumberInput::SetMaxDigits(int idigits_max) {
	// Only accepts values between 1 and 6 (or 7) as RPG2K (or RPG2k3)
	int top = Player::IsRPG2k() ? 6 : 7;
	digits_max =
		(idigits_max > top) ? top :
		(idigits_max <= 0) ? 1 :
		idigits_max;
	index = 0;
	Refresh();
}

bool Window_NumberInput::GetShowOperator() {
	return show_operator;
}

void Window_NumberInput::SetShowOperator(bool show) {
	show_operator = show;
}

void Window_NumberInput::UpdateCursorRect() {
	cursor_rect.Set(index * (cursor_width - 2) + (show_operator ? -2 : 8), 0, cursor_width, 16);
}

void Window_NumberInput::Update() {
	Window_Selectable::Update();
	if (active) {
		if (Input::IsRepeated(Input::DOWN) || Input::IsRepeated(Input::UP)) {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cursor));

			if (show_operator && index == 0) {
				plus = !plus;
			} else {
				int place = 1;
				for (int i = 0; i < (digits_max - 1 - (int)index + (int)show_operator); ++i) {
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
			}
			Refresh();
		}

		if (Input::IsRepeated(Input::RIGHT)) {
			if (digits_max >= 2) {
				Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cursor));
				index = (index + 1) % (digits_max + (int)show_operator);
			}
		}

		if (Input::IsRepeated(Input::LEFT)) {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cursor));
			index = (index + digits_max - 1 + (int)show_operator) % (digits_max + (int)show_operator);
		}

		UpdateCursorRect();
	}
}
