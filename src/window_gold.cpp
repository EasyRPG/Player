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
#include "window_gold.h"
#include "game_party.h"
#include "main_data.h"

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
Window_Gold::Window_Gold() : Window_Base(0, 0, 88, 32) {
	contents = new Bitmap(width-16, height-16);

	Refresh();
}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
Window_Gold::~Window_Gold() {
}

////////////////////////////////////////////////////////////
/// Refresh
////////////////////////////////////////////////////////////
void Window_Gold::Refresh() {
	contents->Clear();
	Rect rect(0, 0, contents->GetWidth(), 16);
	std::stringstream gold;
	gold << Game_Party::GetGold();
	gold << Data::terms.gold;
	
	contents->FillofColor(rect, windowskin->GetColorKey());
	contents->SetColorKey(windowskin->GetColorKey());
	contents->TextDraw(rect, gold.str(), Bitmap::align_right);
}
