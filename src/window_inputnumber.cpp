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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "window_inputnumber.h"
#include "game_system.h"
#include "input.h"
#include "main_data.h"
#include "util_macro.h"

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
Window_InputNumber::Window_InputNumber(int idigits_max) : 
	Window_Base(0, 160, 320, 80), digits_max(idigits_max) {
	//only accepts velues between 1 and 6 as RPGM2K
	digits_max = (digits_max > 6) ? 6 : (digits_max <= 0) ? 1 : digits_max;
	number = 0;
	Bitmap* dummy_bitmap = Bitmap::CreateBitmap(16, 16);
	cursor_width = dummy_bitmap->GetTextSize("0").width + 4;
	delete dummy_bitmap;

	contents = Bitmap::CreateBitmap(GetWidth() - 16, GetHeight() - 16);
	contents->SetTransparentColor(windowskin->GetTransparentColor());

	z += 9999;
	index = 0;

	Refresh();
	UpdateCursorRect();
}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
Window_InputNumber::~Window_InputNumber() {
}

////////////////////////////////////////////////////////////
/// Refresh
////////////////////////////////////////////////////////////
void Window_InputNumber::Refresh() {
	contents->Clear();

	contents->GetFont()->color = Font::ColorDefault;
	char s[6];
	sprintf(s, "%0*d", digits_max, number);
	
	Rect rect_text(0, 0, contents->GetWidth(), 16);

	for (int i = 0; i < digits_max; ++i) {
		char c[2] = {s[i], '\0'}; 
		rect_text.x = i * cursor_width + 10;
		contents->TextDraw(rect_text, c, Bitmap::TextAlignLeft);
	}	
}

////////////////////////////////////////////////////////////
/// Get Number
////////////////////////////////////////////////////////////
int Window_InputNumber::GetNumber() {
	return number;
}

////////////////////////////////////////////////////////////
/// Set Number
////////////////////////////////////////////////////////////
void Window_InputNumber::SetNumber(uint inumber) {
	uint num = 1;
	for (int i = 0; i < digits_max; ++i) {
		num *= 10;
	}
	number = min(max(inumber, (uint)0), num - 1);
	Refresh();
}

////////////////////////////////////////////////////////////
/// Update Cursor Rect
////////////////////////////////////////////////////////////
void Window_InputNumber::UpdateCursorRect() {
	cursor_rect.Set(index * cursor_width + 8, 0, cursor_width, 16);
}

////////////////////////////////////////////////////////////
/// Update
////////////////////////////////////////////////////////////
void Window_InputNumber::Update() {
	Window_Base::Update();
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
