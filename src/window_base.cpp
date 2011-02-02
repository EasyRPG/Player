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
#include <iomanip>
#include <sstream>
#include "window_base.h"
#include "cache.h"
#include "data.h"
#include "game_system.h"

////////////////////////////////////////////////////////////
Window_Base::Window_Base(int x, int y, int width, int height) {
	windowskin_name = Game_System::GetSystemName();
	SetWindowskin(Cache::System(windowskin_name), false);

	SetX(x);
	SetY(y);
	SetWidth(width);
	SetHeight(height);
	SetZ(100);
}

////////////////////////////////////////////////////////////
void Window_Base::Update() {
	Window::Update();
	if (Game_System::GetSystemName() != windowskin_name) {
		windowskin_name = Game_System::GetSystemName();
		SetWindowskin(Cache::System(windowskin_name), false);
		contents->SetTransparentColor(windowskin->GetTransparentColor());
	}
}

////////////////////////////////////////////////////////////
void Window_Base::DrawFace(std::string face_name, int face_index, int cx, int cy, bool flip) {
	Bitmap* faceset = Cache::Faceset(face_name);

	Rect src_rect(
		(face_index % 4) * 48,
		face_index / 4 * 48,
		48,
		48
	);

	if (flip) {
		contents->FlipBlit(cx, cy, faceset, src_rect, true, false);
	} else {
		contents->Blit(cx, cy, faceset, src_rect, 255);
	}
}

void Window_Base::DrawActorFace(Game_Actor* actor, int cx, int cy) {
	DrawFace(actor->GetFaceName(), actor->GetFaceIndex(), cx, cy);
}

void Window_Base::DrawActorName(Game_Actor* actor, int cx, int cy) {
	contents->TextDraw(cx, cy, Font::ColorDefault, actor->GetName());
}

void Window_Base::DrawActorTitle(Game_Actor* actor, int cx, int cy) {

}

void Window_Base::DrawActorClass(Game_Actor* actor, int cx, int cy) {
	contents->TextDraw(cx, cy, Font::ColorDefault, actor->GetTitle());
}

void Window_Base::DrawActorLevel(Game_Actor* actor, int cx, int cy) {
	// Draw LV-String
	contents->TextDraw(cx, cy, 1, Data::terms.lvl_short);

	// Draw Level of the Actor
	std::stringstream ss;
	ss << actor->GetLevel();
	contents->TextDraw(cx + 24, cy, Font::ColorDefault, ss.str(), Surface::TextAlignRight);
}

void Window_Base::DrawActorState(Game_Actor* actor, int cx, int cy) {
	std::vector<int> states = actor->GetStates();

	// Unit has Normal state if no state is set
	if (states.size() == 0) {
		contents->TextDraw(cx, cy, Font::ColorDefault, Data::terms.normal_status);
	} else {
		int highest_priority = 0;
		int state = 0;

		// Display the state with the highest priority
		for (size_t i = 0; i < states.size(); ++i) {
			if (Data::states[states[i]].priority > highest_priority) {
				state = i;
			}
		}

		contents->TextDraw(cx, cy, Data::states[state].color, Data::states[state].name);
	}
}

void Window_Base::DrawActorExp(Game_Actor* actor, int cx, int cy) {
	// Draw EXP-String
	contents->TextDraw(cx, cy, 1, Data::terms.exp_short);

	// Current Exp of the Actor
	// ------/------
	std::stringstream ss;
	ss << std::setfill(' ') << std::setw(6) << actor->GetExpString();

	// Delimiter
	ss << '/';

	// Exp for Level up
	ss << std::setfill(' ') << std::setw(6) << actor->GetNextExpString();
	contents->TextDraw(cx + 12, cy, Font::ColorDefault, ss.str(), Surface::TextAlignLeft);
}

void Window_Base::DrawActorHp(Game_Actor* actor, int cx, int cy) {
	// Draw HP-String
	contents->TextDraw(cx, cy, 1, Data::terms.hp_short);

	// Draw Current HP of the Actor
	cx += 12;
	// Color: 0 okay, 4 critical, 5 dead
	int color = Font::ColorDefault;
	if (actor->GetHp() == 0) {
		color = Font::ColorKnockout;
	} else if (actor->GetHp() <= actor->GetMaxHp() / 4) {
		color = Font::ColorCritical;
	}
	std::stringstream ss;
	ss << actor->GetHp();
	contents->TextDraw(cx + 18, cy, color, ss.str(), Surface::TextAlignRight);

	// Draw the /
	cx += 3 * 6;
	contents->TextDraw(cx, cy, Font::ColorDefault, "/");

	// Draw Max Hp
	cx += 6;
	ss.str("");
	ss << actor->GetMaxHp();
	contents->TextDraw(cx + 18, cy, Font::ColorDefault, ss.str(), Surface::TextAlignRight);
}

void Window_Base::DrawActorSp(Game_Actor* actor, int cx, int cy) {
	// Draw SP-String
	contents->TextDraw(cx, cy, 1, Data::terms.sp_short);

	// Draw Current SP of the Actor
	cx += 12;
	// Color: 0 okay, 4 critical/empty
	int color = Font::ColorDefault;
	if (actor->GetMaxSp() != 0 && actor->GetSp() <= actor->GetMaxSp() / 4) {
		color = Font::ColorCritical;
	}
	std::stringstream ss;
	ss << actor->GetSp();
	contents->TextDraw(cx + 18, cy, color, ss.str(), Surface::TextAlignRight);

	// Draw the /
	cx += 3 * 6;
	contents->TextDraw(cx, cy, Font::ColorDefault, "/");

	// Draw Max Sp
	cx += 6;
	ss.str("");
	ss << actor->GetMaxSp();
	contents->TextDraw(cx + 18, cy, Font::ColorDefault, ss.str(), Surface::TextAlignRight);
}

void Window_Base::DrawActorParameter(Game_Actor* actor, int cx, int cy, int type) {
	std::string name;
	int value;
	
	switch (type) {
	case 0:
		name = Data::terms.attack;
		value = actor->GetAtk();
		break;
	case 1:
		name = Data::terms.defense;
		value = actor->GetDef();
		break;
	case 2:
		name = Data::terms.spirit;
		value = actor->GetSpi();
		break;
	case 3:
		name = Data::terms.agility;
		value = actor->GetAgi();
		break;
	default:
		return;
	}

	// Draw Term
	Rect rect = contents->GetTextSize(name);
	contents->TextDraw(cx, cy, 1, name);

	// Draw Value
	std::stringstream ss;
	ss << value;
	contents->TextDraw(cx + 78, cy, Font::ColorDefault, ss.str(), Surface::TextAlignRight);
}

void Window_Base::DrawEquipmentType(Game_Actor* actor, int cx, int cy, int type) {
	std::string name;
	
	switch (type) {
	case 0:
		name = Data::terms.weapon;
		break;
	case 1:
		if (actor->GetTwoSwordsStyle()) {
			name = Data::terms.weapon;
		} else {
			name = Data::terms.shield;
		}
		break;
	case 2:
		name = Data::terms.armor;
		break;
	case 3:
		name = Data::terms.helmet;
		break;
	case 4:
		name = Data::terms.accessory;
		break;
	default:
		return;
	}

	contents->TextDraw(cx, cy, 1, name);
}

void Window_Base::DrawItemName(RPG::Item* item, int cx, int cy, bool enabled) {
	int color = enabled ? Font::ColorDefault : Font::ColorDisabled;
	contents->TextDraw(cx, cy, color, item->name);
}

void Window_Base::DrawSkillName(RPG::Skill* skill, int cx, int cy, bool enabled) {
	int color = enabled ? Font::ColorDefault : Font::ColorDisabled;
	contents->TextDraw(cx, cy, color, skill->name);
}

void Window_Base::DrawCurrencyValue(int money, int cx, int cy) {
	// This function draws right aligned because of the dynamic with of the
	// gold output (cx and cy define the right border)
	std::stringstream gold;
	gold << money;

	Rect gold_text_size = contents->GetTextSize(Data::terms.gold);
	contents->TextDraw(cx, cy, 1, Data::terms.gold, Surface::TextAlignRight);

	contents->TextDraw(cx - gold_text_size.width, cy, Font::ColorDefault, gold.str(), Surface::TextAlignRight);
}
