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
#include "window_command.h"
#include "color.h"

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
Window_Command::Window_Command(int width, std::vector<std::string> icommands) :
	Window_Selectable(0, 0, width, icommands.size() * 16 + 16)
{
	item_max = icommands.size();
	commands = icommands;
	index = 0;

	contents = new Bitmap(width - 16, item_max * 16);
	Refresh();
}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
Window_Command::~Window_Command() {
}

////////////////////////////////////////////////////////////
/// Refresh
////////////////////////////////////////////////////////////
void Window_Command::Refresh() {
	contents->Clear();
	for (int i = 0; i < item_max; i++) {
		DrawItem(i, Color::Default);
	}
}

////////////////////////////////////////////////////////////
/// Draw Item
////////////////////////////////////////////////////////////
void Window_Command::DrawItem(int i, int color) {

	contents->GetFont()->color = color;
	Rect rect(4, 16 * i, contents->GetWidth() - 8, 16);
	Rect rect2(0, 16 * i, contents->GetWidth() - 0, 16);
	//contents->FillRect(rect, Color(0, 0, 0, 0));
	contents->FillofColor(rect2, windowskin->GetColorKey());
	contents->SetColorKey(windowskin->GetColorKey());
	contents->TextDraw(rect, commands[i]);
}

////////////////////////////////////////////////////////////
/// Disable Item
////////////////////////////////////////////////////////////
void Window_Command::DisableItem(int i) {
	DrawItem(i, Color::Disabled);
}
