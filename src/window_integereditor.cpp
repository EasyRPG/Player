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
#include "window_integereditor.h"
#include "bitmap.h"
#include "font.h"
#include "input.h"
#include "game_system.h"
#include <sstream>

Window_IntegerEditor::Window_IntegerEditor(int icolumns) :
	Window_Selectable((105), (104), (110), (32)) {

	column_max = icolumns;

	item_max = column_max;

	SetContents(Bitmap::Create(this->width - 8, 16));
	contents->SetTransparentColor(windowskin->GetTransparentColor());

	index = 1;
	SetVisible(false);
}

void Window_IntegerEditor::Refresh() {
	contents->Clear();
	for (int i = 0; i < item_max; i++) {
		DrawDigit(i, Font::ColorDefault);
	}
}

void Window_IntegerEditor::Update() {
		Window_Base::Update();
	if (active && item_max > 0 && index >= 0) {
		if (Input::IsRepeated(Input::DOWN)) {
			Game_System::SePlay(Main_Data::game_data.system.cursor_se);
			ReduceDigit(index);
			Refresh();
		}
		if (Input::IsRepeated(Input::UP)) {
			Game_System::SePlay(Main_Data::game_data.system.cursor_se);
			RiseDigit(index);
			Refresh();
		}
		if (Input::IsRepeated(Input::RIGHT)) {
			if (column_max >= 2 && index < item_max - 1) {
				Game_System::SePlay(Main_Data::game_data.system.cursor_se);
				index += 1;
			}
		}
		if (Input::IsRepeated(Input::LEFT)) {
			if (column_max >= 2 && index > 0) {
				Game_System::SePlay(Main_Data::game_data.system.cursor_se);
				index -= 1;
			}
		}
	}
	if (active && help_window != NULL) {
		UpdateHelp();
	}
	UpdateCursorRect();
}

int Window_IntegerEditor::GetValue() {
	int val = 0;
	val += digits[7]*1;
	val += digits[6]*10;
	val += digits[5]*100;
	val += digits[4]*1000;
	val += digits[3]*10000;
	val += digits[2]*100000;
	val += digits[1]*1000000;
	val *= digits[0];
	return val;
}

void Window_IntegerEditor::SetValue(int ivalue) {
	if (ivalue < 0) {
		digits[0] = -1;
		ivalue *= -1;
	} else
		digits[0] = 1;
	digits[7] = ivalue % 10;
	digits[6] = (ivalue % 100) / 10;
	digits[5] = (ivalue % 1000) / 100;
	digits[4] = (ivalue % 10000) / 1000;
	digits[3] = (ivalue % 100000) / 10000;
	digits[2] = (ivalue % 1000000) / 100000;
	digits[1] = (ivalue % 10000000) / 1000000;
	SetIndex(column_max - 1);
	Refresh();
}

void Window_IntegerEditor::DrawDigit(int index, Font::SystemColor color) {
	std::stringstream ss;
	if (index == 0)
		ss << ((digits[0] == 1) ? "+" : "-");
	else
		ss << digits[index];
	contents->TextDraw(12*index,2,9,9, color, ss.str());
}

void Window_IntegerEditor::RiseDigit(int index) {
	if (index > 7 || index < 0)
		return;
	if (index == 0){
		digits[0] *= -1;
		return;
	}
	digits[index]++;
	if (digits[index] > 9){
		digits[index] = 0;
	}
}

void Window_IntegerEditor::ReduceDigit(int index) {
	if (index > 7 || index < 0)
		return;
	if (index == 0){
		digits[0] *= -1;
		return;
	}
	digits[index]--;
	if (digits[index] < 0){
		digits[index] = 9;
	}
}

void Window_IntegerEditor::UpdateCursorRect() {

	if (index < 0) {
		SetCursorRect(Rect());
		return;
	}
	SetCursorRect(Rect(index*12-4,-oy,14,16));
}