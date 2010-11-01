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
#include "window_menustatus.h"
#include "game_party.h"

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
Window_MenuStatus::Window_MenuStatus() : Window_Base(0, 0, 232, 240) {
	contents = new Bitmap(width - 16, height - 16);
	Refresh();
}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
Window_MenuStatus::~Window_MenuStatus() {
}

////////////////////////////////////////////////////////////
/// Refresh
////////////////////////////////////////////////////////////
void Window_MenuStatus::Refresh() {
	int offset = 0;
	for (unsigned i = 0; i < Main_Data::game_party->actors.size(); ++i)
	{
		DrawActorGraphic(Main_Data::game_party->actors[i], 0, i*48 + offset);

		// (48 + 8, 4)
		DrawActorName(Main_Data::game_party->actors[i], 48 + 8, i*48 + 4 + offset);
		offset += 10;
	}
}
