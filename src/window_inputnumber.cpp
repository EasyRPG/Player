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
Window_InputNumber::Window_InputNumber(unsigned int idigits_max) : 
	Window_Base(0, 0, 0, 0), digits_max(idigits_max) {
	number = 0;
	Bitmap* dummy_bitmap = new Bitmap(16, 16);
	cursor_width = dummy_bitmap->GetTextSize("0").width + 4;
	delete dummy_bitmap;
	SetWidth(cursor_width * digits_max + 16);
	SetHeight(32);
	contents = new Bitmap(GetWidth() - 16, GetHeight() - 16);
	z += 9999;
	opacity = 0;
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
	Rect rect(0, 0, contents->GetWidth(), 16);
	contents->FillofColor(rect, windowskin->GetColorKey());
	contents->SetColorKey(windowskin->GetColorKey());
	contents->GetFont()->color = Color::Default;
	char s[20];
	sprintf(s, "%0*d", digits_max, number);
	
	for (unsigned int i = 0; i < digits_max; ++i) {
		char c[2] = {s[i], '\0'}; 
		rect.x = i * cursor_width + 2;
		contents->TextDraw(rect, c, Bitmap::align_left);
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
void Window_InputNumber::SetNumber(unsigned int inumber) {
	int num = 1;
	for (int i = 0; i < digits_max; ++i) {
		num *= 10;
	}
	number = min(max(inumber, 0), num - 1);
	Refresh();
}

////////////////////////////////////////////////////////////
/// Update Cursor Rect
////////////////////////////////////////////////////////////
void Window_InputNumber::UpdateCursorRect() {
	cursor_rect.Set(index * cursor_width, 0, cursor_width, 16);
}

////////////////////////////////////////////////////////////
/// Update
////////////////////////////////////////////////////////////
void Window_InputNumber::Update() {
	Window_Base::Update();
	if (Input::IsRepeated(Input::DOWN) || Input::IsRepeated(Input::UP)) {
		Game_System::SePlay(Data::system.cursor_se);

		int place = 1;
		for (int i = 0; i < (digits_max - 1 - index); ++i) {
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
