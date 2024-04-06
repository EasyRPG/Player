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
#include <lcf/data.h>
#include "game_system.h"
#include "bitmap.h"
#include "font.h"
#include "player.h"

Window_Base::Window_Base(Scene* parent, WindowType type, int x, int y, int width, int height, Drawable::Flags flags)
	: Window(parent, type, flags)
{
	SetWindowskin(Cache::SystemOrBlack());

	SetX(x);
	SetY(y);
	SetWidth(width);
	SetHeight(height);
	SetStretch(Main_Data::game_system->GetMessageStretch() == lcf::rpg::System::Stretch_stretch);
	SetZ(Priority_Window);
}

Window_Base::Window_Base(Scene* parent, int x, int y, int width, int height, Drawable::Flags flags)
	: Window(parent, WindowType::Unknown, flags)
{
	SetWindowskin(Cache::SystemOrBlack());

	SetX(x);
	SetY(y);
	SetWidth(width);
	SetHeight(height);
	SetStretch(Main_Data::game_system->GetMessageStretch() == lcf::rpg::System::Stretch_stretch);
	SetZ(Priority_Window);
}

bool Window_Base::InitMovement(int old_x, int old_y, int new_x, int new_y, int duration) {
	current_frame = 0;
	old_position[0] = old_x;
	old_position[1] = old_y;
	new_position[0] = new_x;
	new_position[1] = new_y;
	SetX(old_position[0]);
	SetY(old_position[1]);
	total_frames = 0;
	if (old_x != new_x || old_y != new_y) {
		total_frames = duration;
		return true;
	}
	return false;
}

bool Window_Base::IsMovementActive() {
	return total_frames > 0 && current_frame <= total_frames;
}

void Window_Base::Update() {
	Window::Update();
	if (IsSystemGraphicUpdateAllowed()) {
		SetWindowskin(Cache::SystemOrBlack());
		SetStretch(Main_Data::game_system->GetMessageStretch() == lcf::rpg::System::Stretch_stretch);
	}
	UpdateMovement();
}

void Window_Base::UpdateMovement() {
	if (!IsMovementActive()) {
		return;
	}
	current_frame++;
	if (IsMovementActive()) {
		SetX(old_position[0] + (new_position[0] - old_position[0]) * current_frame / total_frames);
		SetY(old_position[1] + (new_position[1] - old_position[1]) * current_frame / total_frames);
	} else {
		SetX(new_position[0]);
		SetY(new_position[1]);
	}
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
		contents->FlipBlit(cx, cy, *faceset, src_rect, true, false, Opacity::Opaque());
	}
	else {
		contents->Blit(cx, cy, *faceset, src_rect, 255);
	}
}

// All these functions assume that the input is valid

void Window_Base::DrawFace(StringView face_name, int face_index, int cx, int cy, bool flip) {
	if (face_name.empty()) { return; }

	FileRequestAsync* request = AsyncHandler::RequestFile("FaceSet", face_name);
	request->SetGraphicFile(true);
	face_request_ids.push_back(request->Bind(&Window_Base::OnFaceReady, this, face_index, cx, cy, flip));
	request->Start();
}

void Window_Base::DrawActorFace(const Game_Actor& actor, int cx, int cy) {
	DrawFace(actor.GetFaceName(), actor.GetFaceIndex(), cx, cy);
}

void Window_Base::DrawActorName(const Game_Battler& actor, int cx, int cy) const {
	contents->TextDraw(cx, cy, Font::ColorDefault, actor.GetName());
}

void Window_Base::DrawActorTitle(const Game_Actor& actor, int cx, int cy) const {
	contents->TextDraw(cx, cy, Font::ColorDefault, actor.GetTitle());
}

void Window_Base::DrawActorClass(const Game_Actor& actor, int cx, int cy) const {
	contents->TextDraw(cx, cy, Font::ColorDefault, actor.GetClassName());
}

void Window_Base::DrawActorLevel(const Game_Actor& actor, int cx, int cy) const {
	// Draw LV-String
	contents->TextDraw(cx, cy, 1, lcf::Data::terms.lvl_short);

	// Draw Level of the Actor
	contents->TextDraw(cx + (lcf::Data::system.easyrpg_max_level >= 100 ? 30 : 24), cy, Font::ColorDefault, std::to_string(actor.GetLevel()), Text::AlignRight);
}

void Window_Base::DrawActorState(const Game_Battler& actor, int cx, int cy) const {
	// Unit has Normal state if no state is set
	const lcf::rpg::State* state = actor.GetSignificantState();
	if (!state) {
		contents->TextDraw(cx, cy, Font::ColorDefault, lcf::Data::terms.normal_status);
	} else {
		contents->TextDraw(cx, cy, state->color, state->name);
	}
}

void Window_Base::DrawActorExp(const Game_Actor& actor, int cx, int cy) const {
	// Draw EXP-String
	int width = 7;
	if (actor.MaxExpValue() < 1000000) {
		width = 6;
		contents->TextDraw(cx, cy, 1, lcf::Data::terms.exp_short);
	}

	// Current Exp of the Actor
	// ------/------
	std::stringstream ss;
	ss << std::setfill(' ') << std::setw(width) << actor.GetExpString();

	// Delimiter
	ss << '/';

	// Exp for Level up
	ss << std::setfill(' ') << std::setw(width) << actor.GetNextExpString();
	contents->TextDraw(cx + (width == 6 ? 12 : 0), cy, Font::ColorDefault, ss.str(), Text::AlignLeft);
}

void Window_Base::DrawActorHp(const Game_Battler& actor, int cx, int cy, int digits, bool draw_max) const {
	// Draw HP-String
	contents->TextDraw(cx, cy, 1, lcf::Data::terms.hp_short);

	// Draw Current HP of the Actor
	cx += 12;
	// Color: 0 okay, 4 critical, 5 dead
	int color = GetValueFontColor(actor.GetHp(), actor.GetMaxHp(), true);
	auto dx = digits * 6;
	contents->TextDraw(cx + dx, cy, color, std::to_string(actor.GetHp()), Text::AlignRight);

	if (!draw_max)
		return;

	// Draw the /
	cx += dx;
	contents->TextDraw(cx, cy, Font::ColorDefault, "/");

	// Draw Max Hp
	cx += 6;
	contents->TextDraw(cx + dx, cy, Font::ColorDefault, std::to_string(actor.GetMaxHp()), Text::AlignRight);
}

void Window_Base::DrawActorSp(const Game_Battler& actor, int cx, int cy, int digits, bool draw_max) const {
	// Draw SP-String
	contents->TextDraw(cx, cy, 1, lcf::Data::terms.sp_short);

	// Draw Current SP of the Actor
	cx += 12;
	// Color: 0 okay, 4 critical/empty
	int color = GetValueFontColor(actor.GetSp(), actor.GetMaxSp(), false);
	auto dx = digits * 6;
	contents->TextDraw(cx + dx, cy, color, std::to_string(actor.GetSp()), Text::AlignRight);

	if (!draw_max)
		return;

	// Draw the /
	cx += dx;
	contents->TextDraw(cx, cy, Font::ColorDefault, "/");

	// Draw Max Sp
	cx += 6;
	contents->TextDraw(cx + dx, cy, Font::ColorDefault, std::to_string(actor.GetMaxSp()), Text::AlignRight);
}

void Window_Base::DrawActorParameter(const Game_Battler& actor, int cx, int cy, int type) const {
	StringView name;
	int value;

	switch (type) {
	case 0:
		name = lcf::Data::terms.attack;
		value = actor.GetAtk();
		break;
	case 1:
		name = lcf::Data::terms.defense;
		value = actor.GetDef();
		break;
	case 2:
		name = lcf::Data::terms.spirit;
		value = actor.GetSpi();
		break;
	case 3:
		name = lcf::Data::terms.agility;
		value = actor.GetAgi();
		break;
	default:
		return;
	}

	// Draw Term
	contents->TextDraw(cx, cy, 1, name);

	// Draw Value
	contents->TextDraw(cx + 78, cy, Font::ColorDefault, std::to_string(value), Text::AlignRight);
}

void Window_Base::DrawEquipmentType(const Game_Actor& actor, int cx, int cy, int type) const {
	StringView name;

	switch (type) {
	case 0:
		name = lcf::Data::terms.weapon;
		break;
	case 1:
		if (actor.HasTwoWeapons()) {
			name = lcf::Data::terms.weapon;
		} else {
			name = lcf::Data::terms.shield;
		}
		break;
	case 2:
		name = lcf::Data::terms.armor;
		break;
	case 3:
		name = lcf::Data::terms.helmet;
		break;
	case 4:
		name = lcf::Data::terms.accessory;
		break;
	default:
		return;
	}

	contents->TextDraw(cx, cy, 1, name);
}

void Window_Base::DrawItemName(const lcf::rpg::Item& item, int cx, int cy, bool enabled) const {
	int color = enabled ? Font::ColorDefault : Font::ColorDisabled;

	contents->TextDraw(cx, cy, color, item.name);
}

void Window_Base::DrawSkillName(const lcf::rpg::Skill& skill, int cx, int cy, bool enabled) const {
	int color = enabled ? Font::ColorDefault : Font::ColorDisabled;

	contents->TextDraw(cx, cy, color, skill.name);
}

void Window_Base::DrawCurrencyValue(int money, int cx, int cy) const {
	// This function draws right aligned because of the dynamic with of the
	// gold output (cx and cy define the right border)
	std::stringstream gold;
	gold << money;

	Rect gold_text_size = Text::GetSize(*Font::Default(), lcf::Data::terms.gold);
	contents->TextDraw(cx, cy, 1, lcf::Data::terms.gold, Text::AlignRight);

	contents->TextDraw(cx - gold_text_size.width, cy, Font::ColorDefault, gold.str(), Text::AlignRight);
}

void Window_Base::DrawGauge(const Game_Battler& actor, int cx, int cy, int alpha) const {
	BitmapRef system2 = Cache::System2();
	if (!system2) {
		return;
	}

	bool full = actor.IsAtbGaugeFull();

	// Which gauge (0 - 2)
	int gauge_y = 32 + 2 * 16;

	// Three components of the gauge
	Rect gauge_left(0, gauge_y, 16, 16);
	Rect gauge_center(16, gauge_y, 16, 16);
	Rect gauge_right(32, gauge_y, 16, 16);

	Rect dst_rect(cx + 16, cy, 25, 16);

	contents->Blit(cx + 0, cy, *system2, gauge_left, alpha);
	contents->Blit(cx + 16 + 25, cy, *system2, gauge_right, alpha);
	contents->StretchBlit(dst_rect, *system2, gauge_center, alpha);

	const auto atb = actor.GetAtbGauge();
	const auto gauge_w = 25 * atb / actor.GetMaxAtbGauge();
	if (gauge_w > 0) {
		// Full or not full bar
		Rect gauge_bar(full ? 64 : 48, gauge_y, 16, 16);
		Rect bar_rect(cx + 16, cy, gauge_w, 16);
		contents->StretchBlit(bar_rect, *system2, gauge_bar, alpha);
	}
}

void Window_Base::DrawActorHpValue(const Game_Battler& actor, int cx, int cy) const {
	contents->TextDraw(cx, cy, GetValueFontColor(actor.GetHp(), actor.GetMaxHp(), true), std::to_string(actor.GetHp()), Text::AlignRight);
}

void Window_Base::DrawActorSpValue(const Game_Battler& actor, int cx, int cy) const {
	contents->TextDraw(cx, cy, GetValueFontColor(actor.GetSp(), actor.GetMaxSp(), false), std::to_string(actor.GetSp()), Text::AlignRight);
}

int Window_Base::GetValueFontColor(int have, int max, bool can_knockout) const {
	if (can_knockout && have == 0) return Font::ColorKnockout;
	if (max > 0 && (have <= max / 4)) return Font::ColorCritical;
	return Font::ColorDefault;
}
