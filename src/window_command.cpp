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
Window_Command::Window_Command(int width, std::vector<std::string> commands) :
	Window_Selectable(0, 0, width, commands.size() * 16 + 16),
	commands(commands) {

	item_max = commands.size();
	index = 0;

	contents = Bitmap::CreateBitmap(width - 16, item_max * 16);

	Refresh();
}

////////////////////////////////////////////////////////////
void Window_Command::Refresh() {
	contents->Clear();
	for (int i = 0; i < item_max; i++) {
		DrawItem(i, Font::ColorDefault);
	}
}

////////////////////////////////////////////////////////////
void Window_Command::DrawItem(int index, Font::SystemColor color) {
	contents->ClearRect(Rect(0, 16 * index, contents->GetWidth() - 0, 16));

	contents->GetFont()->color = color;

	Rect rect(0, 16 * index, contents->GetWidth() - 8, 16);
	contents->TextDraw(rect, commands[index]);
}

////////////////////////////////////////////////////////////
void Window_Command::DisableItem(int i) {
	DrawItem(i, Font::ColorDisabled);
}
