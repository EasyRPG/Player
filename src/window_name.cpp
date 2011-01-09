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
#include "window_name.h"

////////////////////////////////////////////////////////////
Window_Name::Window_Name(int ix, int iy, int iwidth, int iheight) :
	Window_Base(ix, iy, iwidth, iheight) {

	SetContents(Bitmap::CreateBitmap(width - 16, height - 16));
	contents->SetTransparentColor(windowskin->GetTransparentColor());

	name.clear();

	Refresh();
}

////////////////////////////////////////////////////////////
Window_Name::~Window_Name() {
}

////////////////////////////////////////////////////////////
void Window_Name::Refresh() {
	contents->Clear();
	contents->GetFont()->color = Font::ColorDefault;
	contents->TextDraw(2, 2, name);
}

void Window_Name::Set(const std::string& text) {
	name = text;
	Refresh();
}

void Window_Name::Append(const std::string& text) {
	name += text;
	Refresh();
}

void Window_Name::Erase() {
	if (name.size() < 1)
		return;
	std::string::iterator it = name.end();
	name.erase(--it);
	Refresh();
}

const std::string& Window_Name::Get() {
	return name;
}

