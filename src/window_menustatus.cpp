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
#include "graphics.h"
#include "sdl_ui.h"
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
	
	Rect rect(0, 0, contents->GetWidth(), contents->GetHeight());
	contents->FillofColor(rect, windowskin->GetColorKey());
	contents->SetColorKey(windowskin->GetColorKey());

	Color c = windowskin->GetPixel(0, 32);
	DisplaySdlUi->SetBackcolor(c.Get());

	int y = 0;
	for (size_t i = 0; i < Game_Party::GetActors().size(); ++i)
	{
		DrawActorGraphic(Game_Party::GetActors()[i], 0, i*48 + y);

		DrawActorName(Game_Party::GetActors()[i], 48 + 8, i*48 + 2 + y);
		DrawActorClass(Game_Party::GetActors()[i], 48 + 8 + 88, i*48 + 2 + y);
		DrawActorLevel(Game_Party::GetActors()[i], 48 + 8, i*48 + 2 + 16 + y);
		DrawActorState(Game_Party::GetActors()[i], 48 + 8 + 42, i*48 + 2 + 16 + y);
		DrawActorExp(Game_Party::GetActors()[i], 48 + 8, i*48 + 2 + 16 + 16 + y);
		DrawActorHp(Game_Party::GetActors()[i], 48 + 8 + 106, i*48 + 2 + 16 + y);
		DrawActorSp(Game_Party::GetActors()[i], 48 + 8 + 106, i*48 + 2 + 16 + 16 + y);

		y += 10;
	}
}
