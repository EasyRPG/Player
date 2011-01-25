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
#include <iomanip>
#include <sstream>
#include "window_savefile.h"
#include "game_system.h"
#include "input.h"

////////////////////////////////////////////////////////////
Window_SaveFile::Window_SaveFile(int ix, int iy, int iwidth, int iheight) : 
	Window_Base(ix, iy, iwidth, iheight) {
	index = 0;

	SetContents(Surface::CreateSurface(width - 8, height - 16));
	contents->SetTransparentColor(windowskin->GetTransparentColor());
	SetZ(9999);

	Refresh();
	UpdateCursorRect();
}

Window_SaveFile::~Window_SaveFile() {
}

void Window_SaveFile::UpdateCursorRect() {
	SetCursorRect(GetActive() ? Rect(0, 0, 48, 16) : Rect());
}

void Window_SaveFile::SetIndex(int id) {
	index = id;
}

void Window_SaveFile::SetParty(const std::vector<Game_Actor*>& actors) {
	party = actors;
}

void Window_SaveFile::Refresh() {
	contents->Clear();

	contents->GetFont()->color = Font::ColorDefault;

	std::ostringstream out;
	out << "File " << std::setw(2) << std::setfill(' ') << index + 1;
	contents->TextDraw(4, 0+2, out.str());

	if (party.empty())
		return;

	Game_Actor* actor = party[0];
	contents->TextDraw(8, 16+2, actor->GetName());

	contents->GetFont()->color = 1;
	contents->TextDraw(8, 32+2, Data::terms.lvl_short);

	contents->GetFont()->color = Font::ColorDefault;
	int lx = Data::terms.lvl_short.size() * 6;
	out.str("");
	out << std::setw(2) << std::setfill(' ') << actor->GetLevel();
	contents->TextDraw(8+lx, 32+2, out.str());

	contents->GetFont()->color = 1;
	contents->TextDraw(42, 32+2, Data::terms.hp_short);

	contents->GetFont()->color = Font::ColorDefault;
	int hx = Data::terms.hp_short.size() * 6;
	out.str("");
	out << actor->GetHp();
	contents->TextDraw(42+hx, 32+2, out.str());

	for (int i = 0; i < 4 && (size_t) i < party.size(); i++)
		DrawActorFace(party[i], 88 + i * 56, 0);
}

void Window_SaveFile::Update() {
	Window_Base::Update();
	UpdateCursorRect();
}

