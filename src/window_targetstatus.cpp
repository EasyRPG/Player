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
#include "window_targetstatus.h"
#include "game_party.h"

////////////////////////////////////////////////////////////
Window_TargetStatus::Window_TargetStatus(int ix, int iy, int iwidth, int iheight) :
	Window_Base(ix, iy, iwidth, iheight), id(-1), use_item(false) {

	contents = Bitmap::CreateBitmap(width - 16, height - 16);
	contents->SetTransparentColor(windowskin->GetTransparentColor());
}

////////////////////////////////////////////////////////////
Window_TargetStatus::~Window_TargetStatus() {
}

////////////////////////////////////////////////////////////
void Window_TargetStatus::Refresh() {
	Rect rect(0, 0, contents->GetWidth(), 16);

	contents->ClearRect(rect);

	if (id < 0) {
		return;
	}

	contents->GetFont()->color = 1;

	if (use_item) {
		contents->TextDraw(rect, Data::terms.possessed_items);
	} else {
		contents->TextDraw(rect, Data::terms.sp_cost);
	}

	std::stringstream ss;
	if (use_item) {
		ss << Game_Party::ItemNumber(id);
	} else {
		ss << Data::skills[id - 1].sp_cost;
	}

	contents->GetFont()->color = Font::ColorDefault;
	contents->TextDraw(rect, ss.str(), Bitmap::TextAlignRight);
}

////////////////////////////////////////////////////////////
void Window_TargetStatus::SetData(int id, bool is_item) {
	this->id = id;
	use_item = is_item;

	Refresh();
}
