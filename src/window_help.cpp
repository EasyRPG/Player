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
#include "window_help.h"

////////////////////////////////////////////////////////////
Window_Help::Window_Help() :
	Window_Base(0, 0, 320, 32),	text(""), align(Bitmap::align_left) {
	contents = new Bitmap(width - 16, height - 16);
	Rect rect(0, 0, contents->GetWidth(), contents->GetHeight());
	contents->FillofColor(rect, windowskin->GetColorKey());
	contents->SetColorKey(windowskin->GetColorKey());
}

////////////////////////////////////////////////////////////
Window_Help::~Window_Help() {
}

////////////////////////////////////////////////////////////
void Window_Help::SetText(std::string text,	Bitmap::TextAlignment align) {
	if (this->text != text || this->align != align) {
		Rect rect(0, 0, contents->GetWidth(), contents->GetHeight());
		contents->FillofColor(rect, windowskin->GetColorKey());
		contents->SetColorKey(windowskin->GetColorKey());

		this->text = text;
		this->align = align;

		contents->GetFont()->color = 0;
		contents->TextDraw(rect, text, align);
	}
}
