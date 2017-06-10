/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

// Headers
#include <iomanip>
#include <sstream>
#include "window_base.h"
#include "cache.h"
#include "data.h"
#include "game_system.h"
#include "bitmap.h"
#include "font.h"
#include "player.h"

Window_Base::Window_Base(int x, int y, int width, int height) {
	windowskin_name = Game_System::GetSystemName();
	if (!windowskin_name.empty()) {
		SetWindowskin(Cache::System(windowskin_name));
	} else {
		SetWindowskin(Bitmap::Create(160, 80, false));
	}

	SetX(x);
	SetY(y);
	SetWidth(width);
	SetHeight(height);
	SetStretch(Game_System::GetMessageStretch() == RPG::System::Stretch_stretch);
	SetZ(Priority_Window);
}

void Window_Base::Update() {
	Window::Update();
	if (Game_System::GetSystemName() != windowskin_name) {
		windowskin_name = Game_System::GetSystemName();
		SetWindowskin(Cache::System(windowskin_name));
	}
	SetStretch(Game_System::GetMessageStretch() == RPG::System::Stretch_stretch);
}

void Window_Base::OnFaceReady(FileRequestResult* result, int face_index, int cx, int cy, bool flip) {
	BitmapRef faceset = Cache::Faceset(result->file);

	Rect src_rect(
		(face_index % 4) * 48,
		face_index / 4 * 48,
		48,
		48
		);

	if (flip) {
		contents->FlipBlit(cx, cy, *faceset, src_rect, true, false, Opacity::opaque);
	}
	else {
		contents->Blit(cx, cy, *faceset, src_rect, 255);
	}
}

void Window_Base::DrawFace(const std::string& face_name, int face_index, int cx, int cy, bool flip) {
	if (face_name.empty()) { return; }

	FileRequestAsync* request = AsyncHandler::RequestFile("FaceSet", face_name);
	face_request_ids.push_back(request->Bind(&Window_Base::OnFaceReady, this, face_index, cx, cy, flip));
	request->Start();
}

void Window_Base::DrawActorFace(Game_Actor* actor, int cx, int cy) {
	DrawFace(actor->GetFaceName(), actor->GetFaceIndex(), cx, cy);
}

void Window_Base::DrawActorName(Game_Battler* actor, int cx, int cy) {
	contents->TextDraw(cx, cy, Font::ColorDefault, actor->GetName());
}

void Window_Base::DrawActorTitle(Game_Actor* actor, int cx, int cy) {
	contents->TextDraw(cx, cy, Font::ColorDefault, actor->GetTitle());
}

void Window_Base::DrawActorClass(Game_Actor* actor, int cx, int cy) {
	contents->TextDraw(cx, cy, Font::ColorDefault, actor->GetClassName());
}

void Window_Base::DrawActorLevel(Game_Actor* actor, int cx, int cy) {
	// Draw LV-String
	contents->TextDraw(cx, cy, 1, Data::terms.lvl_short);

	// Draw Level of the Actor
	std::stringstream ss;
	ss << actor->GetLevel();
	contents->TextDraw(cx + 24, cy, Font::ColorDefault, ss.str(), Text::AlignRight);
}

void Window_Base::DrawActorState(Game_Battler* actor, int cx, int cy) {
	std::vector<int16_t> states = actor->GetStates();

	// Unit has Normal state if no state is set
	const RPG::State* state = actor->GetSignificantState();
	if (!state) {
		contents->TextDraw(cx, cy, Font::ColorDefault, Data::terms.normal_status);
	} else {
		contents->TextDraw(cx, cy, state->color, state->name);
	}
}

void Window_Base::DrawActorExp(Game_Actor* actor, int cx, int cy) {
	// Draw EXP-String
	if (Player::IsRPG2k()) {
		contents->TextDraw(cx, cy, 1, Data::terms.exp_short);
	}

	// Current Exp of the Actor
	// ------/------
	std::stringstream ss;
	ss << std::setfill(' ') << std::setw(6) << actor->GetExpString();

	// Delimiter
	ss << '/';

	// Exp for Level up
	ss << std::setfill(' ') << std::setw(6) << actor->GetNextExpString();
	contents->TextDraw(cx + (Player::IsRPG2k() ? 12 : 0), cy, Font::ColorDefault, ss.str(), Text::AlignLeft);
}

void Window_Base::DrawActorHp(Game_Battler* actor, int cx, int cy, bool draw_max) {
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
	contents->TextDraw(cx + (Player::IsRPG2k() ? 3 : 4) * 6, cy, color, ss.str(), Text::AlignRight);

	if (!draw_max)
		return;

	// Draw the /
	cx += (Player::IsRPG2k() ? 3 : 4) * 6;
	contents->TextDraw(cx, cy, Font::ColorDefault, "/");

	// Draw Max Hp
	cx += 6;
	ss.str("");
	ss << actor->GetMaxHp();
	contents->TextDraw(cx + (Player::IsRPG2k() ? 3 : 4) * 6, cy, Font::ColorDefault, ss.str(), Text::AlignRight);
}

void Window_Base::DrawActorSp(Game_Battler* actor, int cx, int cy, bool draw_max) {
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
	contents->TextDraw(cx + 18, cy, color, ss.str(), Text::AlignRight);

	if (!draw_max)
		return;

	// Draw the /
	cx += 3 * 6;
	contents->TextDraw(cx, cy, Font::ColorDefault, "/");

	// Draw Max Sp
	cx += 6;
	ss.str("");
	ss << actor->GetMaxSp();
	contents->TextDraw(cx + 18, cy, Font::ColorDefault, ss.str(), Text::AlignRight);
}

void Window_Base::DrawActorParameter(Game_Battler* actor, int cx, int cy, int type) {
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
	contents->TextDraw(cx, cy, 1, name);

	// Draw Value
	std::stringstream ss;
	ss << value;
	contents->TextDraw(cx + 78, cy, Font::ColorDefault, ss.str(), Text::AlignRight);
}

void Window_Base::DrawEquipmentType(Game_Actor* actor, int cx, int cy, int type) {
	std::string name;

	switch (type) {
	case 0:
		name = Data::terms.weapon;
		break;
	case 1:
		if (actor->HasTwoWeapons()) {
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

	Rect gold_text_size = Font::Default()->GetSize(Data::terms.gold);
	contents->TextDraw(cx, cy, 1, Data::terms.gold, Text::AlignRight);

	contents->TextDraw(cx - gold_text_size.width, cy, Font::ColorDefault, gold.str(), Text::AlignRight);
}

void Window_Base::DrawGauge(Game_Battler* actor, int cx, int cy) {
	FileRequestAsync* request = AsyncHandler::RequestFile("System2", Data::system.system2_name);
	if (!request->IsReady()) {
		// Gauge refreshed each frame, so we can wait via polling
		request->Start();
		return;
	}

	BitmapRef system2 = Cache::System2(Data::system.system2_name);

	bool full = actor->IsGaugeFull();
	int gauge_w = actor->GetGauge() / 4;

	// Which gauge (0 - 2)
	int gauge_y = 32 + 2 * 16;

	// Three components of the gauge
	Rect gauge_left(0, gauge_y, 16, 16);
	Rect gauge_center(16, gauge_y, 16, 16);
	Rect gauge_right(32, gauge_y, 16, 16);

	// Full or not full bar
	Rect gauge_bar(full ? 64 : 48, gauge_y, 16, 16);

	Rect dst_rect(cx + 16, cy, 25, 16);
	Rect bar_rect(cx + 16, cy, gauge_w, 16);

	contents->Blit(cx + 0, cy, *system2, gauge_left, 255);
	contents->Blit(cx + 16 + 25, cy, *system2, gauge_right, 255);

	contents->StretchBlit(dst_rect, *system2, gauge_center, 255);
	contents->StretchBlit(bar_rect, *system2, gauge_bar, 255);
}
