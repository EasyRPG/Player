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
#include <string>
#include <sstream>
#include "window_total.h"

////////////////////////////////////////////////////////////
Window_Total::Window_Total(int ix, int iy, int iwidth, int iheight) :
	Window_Base(ix, iy, iwidth, iheight) {

	SetContents(Bitmap::CreateBitmap(width - 16, height - 16));
	contents->SetTransparentColor(windowskin->GetTransparentColor());

	possessed = 0;
	equipped = 0;

	Refresh();
}

////////////////////////////////////////////////////////////
Window_Total::~Window_Total() {
}

////////////////////////////////////////////////////////////
void Window_Total::Refresh() {
	contents->Clear();
	contents->GetFont()->color = Font::ColorDefault;

	std::stringstream possessed_ss;
	possessed_ss << possessed;
	const std::string& possessed_str = possessed_ss.str();
	int possessed_w = contents->GetTextSize(possessed_str).width;

	contents->TextDraw(2, 2, Data::terms.possessed_items);
	contents->TextDraw(contents->GetWidth() - 2 - possessed_w, 2, possessed_str);

	std::stringstream equipped_ss;
	equipped_ss << equipped;
	const std::string& equipped_str = equipped_ss.str();
	int equipped_w = contents->GetTextSize(equipped_str).width;

	contents->TextDraw(2, 18, Data::terms.equipped_items);
	contents->TextDraw(contents->GetWidth() - 2 - equipped_w, 18, equipped_str);
}

void Window_Total::SetPossessed(int val) {
	possessed = val;
	Refresh();
}

void Window_Total::SetEquipped(int val) {
	equipped = val;
	Refresh();
}

