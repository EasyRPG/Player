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
#include "window_base.h"
#include <sstream>
#include "cache.h"
#include "game_system.h"

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
Window_Base::Window_Base(int ix, int iy, int iwidth, int iheight) {
	windowskin_name = Game_System::GetSystemName();
	SetWindowskin(Cache::System(windowskin_name));

	SetX(ix);
	SetY(iy);
	SetWidth(iwidth);
	SetHeight(iheight);
	SetZ(100);
}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
Window_Base::~Window_Base() {
}

////////////////////////////////////////////////////////////
/// Update
////////////////////////////////////////////////////////////
void Window_Base::Update() {
	Window::Update();
	if (Game_System::GetSystemName() != windowskin_name) {
		windowskin_name = Game_System::GetSystemName();
		SetWindowskin(Cache::System(windowskin_name));
	}
}

////////////////////////////////////////////////////////////
/// Draw helpers
////////////////////////////////////////////////////////////
void Window_Base::DrawActorGraphic(Game_Actor* actor, int cx, int cy) {
	Bitmap* faceset = Cache::Faceset(actor->GetFaceName());
	int face_index = actor->GetFaceIndex();

	Rect dst_rect(
		(face_index % 4) * 48,
		face_index / 4 * 48,
		48,
		48
	);

	contents->Blit(cx, cy, faceset, dst_rect, 255);
}

void Window_Base::DrawActorName(Game_Actor* actor, int cx, int cy) {
	Rect rect = contents->GetTextSize(actor->GetName());
	rect.x = cx; rect.y = cy;
	contents->GetFont()->color = 0;
	contents->TextDraw(rect, actor->GetName());
}

void Window_Base::DrawActorTitle(Game_Actor* actor, int cx, int cy) {

}

void Window_Base::DrawActorClass(Game_Actor* actor, int cx, int cy) {
	Rect rect = contents->GetTextSize(actor->GetTitle());
	rect.x = cx; rect.y = cy;
	contents->GetFont()->color = 0;
	contents->TextDraw(rect, actor->GetTitle());
}

void Window_Base::DrawActorLevel(Game_Actor* actor, int cx, int cy) {
	// Draw LV-String
	Rect rect(cx, cy, 12, 12);
	contents->GetFont()->color = 1;
	contents->TextDraw(rect, Data::terms.lvl_short);

	// Draw Level of the Actor
	std::stringstream ss;
	ss << actor->GetLevel();
	rect.x = cx + 12;
	contents->GetFont()->color = 0;
	contents->TextDraw(rect, ss.str(), Bitmap::align_right);
}

void Window_Base::DrawActorState(Game_Actor* actor, int cx, int cy) {
	std::vector<int> states = actor->GetStates();

	// Unit has Normal state if no state is set
	if (states.size() == 0) {
		Rect rect = contents->GetTextSize(Data::terms.normal_status);
		rect.x = cx; rect.y = cy;
		contents->GetFont()->color = 0;
		contents->TextDraw(rect, Data::terms.normal_status);
	} else {
		int highest_priority = 0;
		int state = 0;

		// Display the state with the highest priority
		for (int i = 0; i < states.size(); ++i) {
			if (Data::states[states[i]].priority > highest_priority) {
				state = i;
			}
		}

		Rect rect = contents->GetTextSize(Data::states[state].name);
		rect.x = cx; rect.y = cy;
		contents->GetFont()->color = Data::states[state].color;
		contents->TextDraw(rect, Data::states[state].name);
	}
}

void Window_Base::DrawActorState(Game_Actor* actor, int cx, int cy, int width) {
	
}

void Window_Base::DrawActorExp(Game_Actor* actor, int cx, int cy) {
	// Draw EXP-String
	Rect rect(cx, cy, 12, 12);
	contents->GetFont()->color = 1;
	contents->TextDraw(rect, Data::terms.exp_short);

	// Draw Current Exp of the Actor
	// ------/------
	Rect rect2(cx + 12, cy, 6*6, 12);
	contents->GetFont()->color = 0;
	contents->TextDraw(rect2, actor->GetExpString(), Bitmap::align_right);

	// Draw the /
	rect2.x += 6*6;
	contents->TextDraw(rect2, "/");

	// Draw Exp for Level up
	rect2.x += 6;
	contents->TextDraw(rect2, actor->GetNextExpString(), Bitmap::align_right);
}

void Window_Base::DrawActorHp(Game_Actor* actor, int cx, int cy) {
	// Draw HP-String
	Rect rect(cx, cy, 12, 12);
	contents->GetFont()->color = 1;
	contents->TextDraw(rect, Data::terms.hp_short);

	// Draw Current HP of the Actor
	Rect rect2(cx + 12, cy, 3*6, 12);
	// Color: 0 okay, 4 critical, 5 dead
	int color = 0;
	if (actor->GetHp() == 0) {
		color = 5;
	} else if (actor->GetHp() <= actor->GetMaxHp() / 4) {
		color = 4;
	}
	contents->GetFont()->color = color;
	std::stringstream ss;
	ss << actor->GetHp();
	contents->TextDraw(rect2, ss.str(), Bitmap::align_right);

	// Draw the /
	rect2.x += 3*6;
	contents->GetFont()->color = 0;
	contents->TextDraw(rect2, "/");

	// Draw Max Hp
	rect2.x += 6;
	ss.str("");
	ss.clear();
	ss << actor->GetMaxHp();
	contents->TextDraw(rect2, ss.str(), Bitmap::align_right);
}
void Window_Base::DrawActorHp(Game_Actor* actor, int cx, int cy, int width) {
	
}

void Window_Base::DrawActorSp(Game_Actor* actor, int cx, int cy) {
	// Draw SP-String
	Rect rect(cx, cy, 12, 12);
	contents->GetFont()->color = 1;
	contents->TextDraw(rect, Data::terms.sp_short);

	// Draw Current SP of the Actor
	Rect rect2(cx + 12, cy, 3*6, 12);
	// Color: 0 okay, 4 critical/empty
	int color = 0;
	if (actor->GetSp() <= actor->GetMaxSp() / 4) {
		color = 4;
	}
	contents->GetFont()->color = color;
	std::stringstream ss;
	ss << actor->GetSp();
	contents->TextDraw(rect2, ss.str(), Bitmap::align_right);

	// Draw the /
	rect2.x += 3*6;
	contents->GetFont()->color = 0;
	contents->TextDraw(rect2, "/");

	// Draw Max Sp
	rect2.x += 6;
	ss.str("");
	ss.clear();
	ss << actor->GetMaxSp();
	contents->TextDraw(rect2, ss.str(), Bitmap::align_right);
}

void Window_Base::DrawActorSp(Game_Actor* actor, int cx, int cy, int width) {
	
}

void Window_Base::DrawActorParameter(Game_Actor* actor, int cx, int cy, int type) {
	
}

void Window_Base::DrawItemName(RPG::Item* item, int cx, int cy) {
	
}
