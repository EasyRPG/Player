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
#include <sstream>
#include "game_system.h"
#include "input.h"
#include "window_shopnumber.h"

////////////////////////////////////////////////////////////
Window_ShopNumber::Window_ShopNumber(int ix, int iy, int iwidth, int iheight) : 
	Window_Base(ix, iy, iwidth, iheight),
	lo(1), hi(1), number(1) {

	SetContents(Bitmap::CreateBitmap(width - 16, height - 16));
	contents->SetTransparentColor(windowskin->GetTransparentColor());
	SetZ(9999);

	Refresh();
	SetCursorRect(Rect(138, 40, 20, 16));
}

////////////////////////////////////////////////////////////
Window_ShopNumber::~Window_ShopNumber() {
}

////////////////////////////////////////////////////////////
void Window_ShopNumber::SetItemValue(unsigned int val) {
	item_value = val;
	Refresh();
}

////////////////////////////////////////////////////////////
void Window_ShopNumber::SetItemName(const std::string& name) {
	item_name = name;
	Refresh();
}

////////////////////////////////////////////////////////////
void Window_ShopNumber::Refresh() {
	std::ostringstream numstr, valstr;
	numstr << number;
	valstr << number * item_value;

	contents->Clear();

	contents->GetFont()->color = Font::ColorDefault;
	contents->TextDraw(0, 40 + 2, item_name);
	contents->TextDraw(124, 40 + 2, "x");
	contents->TextDraw(138 + 2, 40 + 2, numstr.str());

	contents->GetFont()->color = 1;
	int cx = contents->GetWidth() - (Data::terms.gold.size() + 1) * 6;
	contents->TextDraw(cx, 72 + 2, Data::terms.gold);

	contents->GetFont()->color = Font::ColorDefault;
	cx -=  (valstr.str().size()) * 6;
	contents->TextDraw(cx, 72 + 2, valstr.str());
}

////////////////////////////////////////////////////////////
int Window_ShopNumber::GetNumber() const {
	return number;
}

////////////////////////////////////////////////////////////
int Window_ShopNumber::GetTotal() const {
	return number * item_value;
}

////////////////////////////////////////////////////////////
void Window_ShopNumber::SetNumber(unsigned int val) {
	number = std::min(std::max(val, lo), hi);
	Refresh();
}

////////////////////////////////////////////////////////////
void Window_ShopNumber::SetRange(unsigned int nlo, unsigned int nhi) {
	lo = nlo;
	hi = nhi;
	number = std::min(std::max(number, lo), hi);
	Refresh();
}

////////////////////////////////////////////////////////////
void Window_ShopNumber::Update() {
	Window_Base::Update();

	if (active) {
		if (Input::IsRepeated(Input::LEFT)) {
			if (number > lo) {
				Game_System::SePlay(Data::system.cursor_se);
				number--;
			}
			else {
				number = lo;
			}
			Refresh();
		}

		if (Input::IsRepeated(Input::RIGHT)) {
			if (number < hi) {
				Game_System::SePlay(Data::system.cursor_se);
				number++;
			}
			else {
				number = hi;
			}

			Refresh();
		}
		if (Input::IsRepeated(Input::UP)) {
			if (number < hi) {
				Game_System::SePlay(Data::system.cursor_se);
				number = hi;
			}
			Refresh();
		}
		if (Input::IsRepeated(Input::DOWN)) {
			if (number > lo) {
				Game_System::SePlay(Data::system.cursor_se);
				number = lo;
			}
			Refresh();
		}
	}
}
