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
#include <algorithm>
#include "bitmap.h"
#include "cache.h"
#include "input.h"
#include "game_enemyparty.h"
#include "game_party.h"
#include "game_actor.h"
#include "game_system.h"
#include "game_battle.h"
#include "player.h"
#include "font.h"
#include "output.h"
#include "window_battlestatus.h"
#include "feature.h"


Window_BattleStatus::Window_BattleStatus(int ix, int iy, int iwidth, int iheight, bool enemy) :
	Window_Selectable(ix, iy, iwidth, iheight), mode(ChoiceMode_All), enemy(enemy) {

	SetBorderX(4);

	SetContents(Bitmap::Create(width - 8, height - 16));

	if (Player::IsRPG2k3() && lcf::Data::battlecommands.window_size == lcf::rpg::BattleCommands::WindowSize_small) {
		height = 68;
		menu_item_height = 14;
		actor_face_height = 17;
		SetBorderY(5);
		SetContents(Bitmap::Create(width - 8, height - 10));
	}

	index = -1;

	if (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_gauge) {
		// Simulate a borderless window
		// Doing it this way for gauge style makes the implementation on
		// scene-side easier
		border_x = 0;
		border_y = 0;
		SetContents(Bitmap::Create(width, height));
		SetOpacity(0);
	}
	isEmpty = true;
	Refresh();
}

void Window_BattleStatus::Refresh() {
	// If the battle system uses gauges, wait for system2 to load.
	if (!Feature::HasRpg2kBattleSystem() && !Cache::System2() ) {
		return;
	}
	// This avoids an issue where smaller face graphics (found in the game Ara Fell, as mentioned below in RefreshGauge)
	// will be stretched to fill the full face area, before the window is visible.  I am not sure why that happens.
	if (!IsVisible()) {
		return;
	}
	
	if (enemy) {
		item_max = Main_Data::game_enemyparty->GetBattlerCount();
	}
	else {
		item_max = Main_Data::game_party->GetBattlerCount();
	}

	item_max = std::min(item_max, 4);

	bool needs_redraw = false;
	// Check if any actor IDs have changed, and if so, redraw the whole window.
	for (int i = 0; i < item_max; i++) {
		const Game_Battler* actor = GetActorForItem(i);
		if (itemStates[i].actor_id != actor->GetId()) {
			needs_redraw = true;
			break;
		}
	}
	// Also redraw if we have any rows to remove
	for (int i = item_max; i < 4; i++) {
		if (itemStates[i].is_drawn) {
			needs_redraw = true;
			break;
		}
	}
	if (needs_redraw) {
		if (!isEmpty) {
			contents->Clear();
			isEmpty = true;
		}
		for (int i = item_max; i < 4; i++) {	
			itemStates[i].reset();
		}
	}

	for (int i = 0; i < item_max; i++) {
		// The party only contains valid battlers
		const Game_Battler* actor = GetActorForItem(i);
		itemStates[i].actor_id = actor->GetId();

		bool hp_changed = itemStates[i].hp != actor->GetHp();
		bool hp_max_changed = itemStates[i].max_hp != actor->GetMaxHp();
		bool sp_changed = itemStates[i].sp != actor->GetSp();
		bool sp_max_changed = itemStates[i].max_sp != actor->GetMaxSp();

		if (lcf::Data::battlecommands.battle_type != lcf::rpg::BattleCommands::BattleType_gauge || enemy) {
			
			
			int y = menu_item_height / 8 + i * menu_item_height; 

			const lcf::rpg::State* state = actor->GetSignificantState();
			bool state_changed = (state == NULL && itemStates[i].state_name.compare("") != 0 || itemStates[i].state_color != 0)
								 || (state != NULL && (itemStates[i].state_name.compare(ToString(state->name)) != 0 || itemStates[i].state_color != state->color));
			
								 
			int state_x;

			if (Feature::HasRpg2kBattleSystem() && (hp_changed || hp_max_changed || sp_changed || sp_max_changed || state_changed)) {
				int hpdigits = (actor->MaxHpValue() >= 1000) ? 4 : 3;
				int spdigits = (actor->MaxSpValue() >= 1000) ? 4 : 3;
				state_x = (hpdigits < 4 && spdigits < 4) ? 86 : 80;
				int sp_x = 220 - spdigits * 6;

				// Just clear and redraw the whole Status/HP/MP area for now when something changes
				if (!isEmpty) { contents->ClearRect(Rect(state_x, y, width - state_x, menu_item_height)); }
				DrawActorState(*actor, state_x, y);
				DrawActorHp(*actor, 178 - hpdigits * 6 - spdigits * 6, y, hpdigits, true);
				DrawActorSp(*actor, sp_x, y, spdigits, false);

			} else if (!Feature::HasRpg2kBattleSystem() && (hp_changed || hp_max_changed || state_changed)) {
				// BattleType_traditional/Type A
				if (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_traditional) {
					state_x = 84;
					int hp_x = 136 + 4 * 6;
					// State is left aligned and HP is right aligned, so redraw both of them due to the variable amount of space between them.
					if (!isEmpty) { contents->ClearRect(Rect(state_x, y, hp_x - state_x, menu_item_height)); }
					DrawActorState(*actor, state_x, y);
					DrawActorHpValue(*actor, hp_x, y);
				// BattleType_alternative/Type B
				} else {
					state_x = 80;
					if (state_changed) {
						// Clear up to the gauge area
						if (!isEmpty) { contents->ClearRect(Rect(state_x, y, 130 - state_x, menu_item_height)); }
						DrawActorState(*actor, state_x, y);
					}
				}
			}

			if (itemStates[i].actor_name.compare(actor->GetName())) {
				if (!isEmpty) { contents->ClearRect(Rect(4, y, state_x - 4, menu_item_height)); }
				DrawActorName(*actor, 4, y);
				itemStates[i].actor_name = actor->GetName();
			}
			itemStates[i].state_name = state ? ToString(state->name) : "";
			itemStates[i].state_color = state ? state->color : 0;
		}

		RefreshGauge(actor, i, hp_changed, hp_max_changed, sp_changed, sp_max_changed);

		itemStates[i].hp = actor->GetHp();
		itemStates[i].max_hp = actor->GetMaxHp();
		itemStates[i].sp = actor->GetSp();
		itemStates[i].max_sp = actor->GetMaxSp();

		itemStates[i].is_drawn = true;
	}
	isEmpty = false;
}

const Game_Battler* Window_BattleStatus::GetActorForItem(int i_actor) {
	if (enemy) {
		return &(*Main_Data::game_enemyparty)[i_actor];
	}
	else {
		return &(*Main_Data::game_party)[i_actor];
	}
}

void Window_BattleStatus::RefreshGauge(const Game_Battler* actor, int i_item, bool hp_changed, bool hp_max_changed, bool sp_changed, bool sp_max_changed) {

	if (!Feature::HasRpg2k3BattleSystem()) {
		return;
	}

	bool atb_bar_full = actor->IsAtbGaugeFull();
	int atb_bar_w;

	// There are multiple gauges to draw for BattleType_gauge/Type C
	if (!enemy && lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_gauge) {
		BitmapRef system2 = Cache::System2();
		assert(system2);

		atb_bar_w = GaugeWidthSystem2(actor->GetAtbGauge(), actor->GetMaxAtbGauge());
		const int x_start = 32 + i_item * 80;
		const int fill_x = x_start + 16;
		int y = actor_face_height;
			
		const Game_Actor* game_actor = static_cast<const Game_Actor*>(actor);
		bool face_changed = game_actor->GetFaceName().compare(itemStates[i_item].face_name) != 0 || game_actor->GetFaceIndex() != itemStates[i_item].face_index; 
		// If the HP or SP have changed, 4-digit numbers can overlap the face graphic.  The face graphic 
		// is also overlapped by the gauge graphics, so just redraw everything.  (also do this if the ATB gauge resets.) 
		// Could be fine-tuned, but performance seems ok for now.
		if (hp_changed || hp_max_changed || sp_changed || sp_max_changed || face_changed || atb_bar_w < itemStates[i_item].atb_bar_width) {
			
			if (!isEmpty && face_changed) {
				// If the face graphic changed, clear the entire available area for this actor item. 
				contents->ClearRect(Rect(80 * i_item, 0, 80, height)); 
			}
			else if (!isEmpty) {
				// Otherwise, just clear the part of the face that could be overlapped by the numbers.
				// Note that both clear and redraw are needed because some games don't have 
				// face graphics that are huge enough to clear the left side of the number area (e.g. Ara Fell)
				contents->ClearRect(Rect(40 + 80 * i_item, actor_face_height, 8 * 4, 48));
			}
			
			DrawActorFace(*static_cast<const Game_Actor*>(actor), 80 * i_item, actor_face_height);
			
			int x = x_start; 
			
			// Left Gauge Segments
			contents->Blit(x, y, *system2, Rect(0, 32, 16, 48), Opacity::Opaque());
			x += 16;

			// Center
			contents->StretchBlit(Rect(x, y, 25, 48), *system2, Rect(16, 32, 16, 48), Opacity::Opaque());
			x += 25;

			// Right
			contents->Blit(x, y, *system2, Rect(32, 32, 16, 48), Opacity::Opaque());

			// HP
			DrawGaugeSystem2(fill_x, y, actor->GetHp(), actor->GetMaxHp(), 0);
			// SP
			DrawGaugeSystem2(fill_x, y + 16, actor->GetSp(), actor->GetMaxSp(), 1);
			// ATB
			DrawGaugeSystem2(fill_x, y + 16 * 2, actor->GetAtbGauge(), actor->GetMaxAtbGauge(), 2);

			// Numbers
			x = 40 + 80 * i_item;
			DrawNumberSystem2(x, y, actor->GetHp());
			DrawNumberSystem2(x, y + 12 + 4, actor->GetSp());

			itemStates[i_item].face_name = game_actor->GetFaceName();
			itemStates[i_item].face_index = game_actor->GetFaceIndex();
		}
		// If only the ATB gauge has only updated (common), we can get away with only drawing the gauge bar
		else if (atb_bar_w > itemStates[i_item].atb_bar_width || atb_bar_full != itemStates[i_item].atb_bar_full) {
			DrawGaugeSystem2(fill_x, y + 16 * 2, actor->GetAtbGauge(), actor->GetMaxAtbGauge(), 2);
		}
		itemStates[i_item].atb_bar_width = atb_bar_w;
		itemStates[i_item].atb_bar_full = atb_bar_full;
	}
	else {
		int y = menu_item_height / 8 + i_item * menu_item_height;
		atb_bar_w = GaugeWidth(actor->GetAtbGauge(), actor->GetMaxAtbGauge());

		// BattleType_alternative/Type B has a lot of overlapping content, and is the trickiest one to optimize.
		if (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_alternative) {
			int spdigits = (actor->MaxSpValue() >= 1000) ? 4 : 3;
			bool has_opaque_gauge = lcf::Data::battlecommands.transparency == lcf::rpg::BattleCommands::Transparency_opaque || (menu_item_height / 8 + index * menu_item_height != y);
			bool didUpdate = false;
			// If any of these have changed, then full clear and redraw since there's so much overlap.
			if (hp_changed || hp_max_changed || sp_changed || sp_max_changed || atb_bar_w < itemStates[i_item].atb_bar_width || has_opaque_gauge != itemStates[i_item].has_opaque_gauge) {
				if (!isEmpty) {
					contents->ClearRect(Rect(130, y, width - 130, menu_item_height));
				}
				// RPG_RT Bug (?): Gauge hidden when selected due to transparency (wrong color when rendering)
				if (has_opaque_gauge) {
					DrawGauge(202 - 10, y - 2, actor->GetAtbGauge(), actor->GetMaxAtbGauge(), actor->IsAtbGaugeFull(), true,
							lcf::Data::battlecommands.transparency == lcf::rpg::BattleCommands::Transparency_opaque ? 96 : 255);
				}
				int hpdigits = (actor->MaxHpValue() >= 1000) ? 4 : 3;
				DrawActorHp(*actor, 178 - hpdigits * 6 - spdigits * 6, y, hpdigits, true);
				didUpdate = true;
			}
			// Only the ATB gauge has increased
			else if (atb_bar_w > itemStates[i_item].atb_bar_width || atb_bar_full != itemStates[i_item].atb_bar_full) {
				if (has_opaque_gauge) {
					DrawGauge(202 - 10, y - 2, actor->GetAtbGauge(), actor->GetMaxAtbGauge(), actor->IsAtbGaugeFull(), false,
								lcf::Data::battlecommands.transparency == lcf::rpg::BattleCommands::Transparency_opaque ? 96 : 255);
				}
				didUpdate = true;
			}
			// The SP number overlaps the gauge bar, so it needs to be redrawn if any drawing was done above.
			if (didUpdate) {
				DrawActorSp(*actor, 220 - spdigits * 6, y, spdigits, false);
				itemStates[i_item].atb_bar_width = atb_bar_w;
				itemStates[i_item].atb_bar_full = atb_bar_full;
			}
			itemStates[i_item].has_opaque_gauge = has_opaque_gauge;
		} else {
			// BattleType_traditional/Type A
			if (atb_bar_w != itemStates[i_item].atb_bar_width || atb_bar_full != itemStates[i_item].atb_bar_full) {
				bool shouldDrawGraphic = atb_bar_w < itemStates[i_item].atb_bar_width;
				DrawGauge(156, y - 2, actor->GetAtbGauge(), actor->GetMaxAtbGauge(), actor->IsAtbGaugeFull(), shouldDrawGraphic);
			}
			itemStates[i_item].atb_bar_width = atb_bar_w;
			itemStates[i_item].atb_bar_full = atb_bar_full;
		}
		
	}
}

// This draws the ATB gauge for BattleType_Traditional (Type A) and BattleType_alternative (Type B)
void Window_BattleStatus::DrawGauge(int cx, int cy, int cur_value, int max_value, bool is_full, bool draw_graphic, int alpha) {
	BitmapRef system2 = Cache::System2();
	assert(system2);
	int gauge_w = GaugeWidth(cur_value, max_value);

	// Use the Y position for the gauge at index 2
	int gauge_y = 32 + 2 * 16;

	if (draw_graphic) {
		// Three components of the gauge
		Rect gauge_left(0, gauge_y, 16, 16);
		Rect gauge_center(16, gauge_y, 16, 16);
		Rect gauge_right(32, gauge_y, 16, 16);

		Rect dst_rect(cx + 16, cy, 25, 16);

		contents->Blit(cx + 0, cy, *system2, gauge_left, alpha);
		contents->Blit(cx + 16 + 25, cy, *system2, gauge_right, alpha);
		contents->StretchBlit(dst_rect, *system2, gauge_center, alpha);
	}

	// Draw the bar
	if (gauge_w > 0) {
		// Full or not full bar
		Rect gauge_bar(is_full ? 64 : 48, gauge_y, 16, 16);
		Rect bar_rect(cx + 16, cy, gauge_w, 16);
		contents->StretchBlit(bar_rect, *system2, gauge_bar, alpha);
	}
}

int Window_BattleStatus::GaugeWidth(int cur_value, int max_value) {
	return 25 * cur_value / max_value;
}

void Window_BattleStatus::DrawGaugeSystem2(int x, int y, int cur_value, int max_value, int which) {
	BitmapRef system2 = Cache::System2();
	assert(system2);

	if (max_value == 0) {
		return;
	}

	int gauge_x;
	if (cur_value == max_value) {
		gauge_x = 16;
	}
	else {
		gauge_x = 0;
	}

	int gauge_width = GaugeWidthSystem2(cur_value, max_value);

	contents->StretchBlit(Rect(x, y, gauge_width, 16), *system2, Rect(48 + gauge_x, 32 + 16 * which, 16, 16), Opacity::Opaque());
}

int Window_BattleStatus::GaugeWidthSystem2(int cur_value, int max_value) {
	if (max_value > 0) {
		return 25 * cur_value / max_value;
	}
	return 25;
}

void Window_BattleStatus::DrawNumberSystem2(int x, int y, int value) {
	BitmapRef system2 = Cache::System2();
	assert(system2);

	bool handle_zero = false;

	if (value >= 1000) {
		contents->Blit(x, y, *system2, Rect((value / 1000) * 8, 80, 8, 16), Opacity::Opaque());
		value %= 1000;
		if (value < 100) {
			handle_zero = true;
		}
	}
	if (handle_zero || value >= 100) {
		handle_zero = false;
		contents->Blit(x + 8, y, *system2, Rect((value / 100) * 8, 80, 8, 16), Opacity::Opaque());
		value %= 100;
		if (value < 10) {
			handle_zero = true;
		}
	}
	if (handle_zero || value >= 10) {
		contents->Blit(x + 8 * 2, y, *system2, Rect((value / 10) * 8, 80, 8, 16), Opacity::Opaque());
		value %= 10;
	}

	contents->Blit(x + 8 * 3, y, *system2, Rect(value * 8, 80, 8, 16), Opacity::Opaque());
}

int Window_BattleStatus::ChooseActiveCharacter() {
	int old_index = index < 0 ? 0 : index;
	index = -1;
	for (int i = 0; i < item_max; i++) {
		int new_index = (old_index + i) % item_max;
		if ((*Main_Data::game_party)[new_index].IsAtbGaugeFull()) {
			index = new_index;
			return index;
		}
	}

	if (index != old_index)
		UpdateCursorRect();

	return index;
}

void Window_BattleStatus::SetChoiceMode(ChoiceMode new_mode) {
	mode = new_mode;
}

void Window_BattleStatus::Update() {
	// Window Selectable update logic skipped on purpose
	// (breaks up/down-logic)
	Window_Base::Update();

	int old_item_max = item_max;
	if (enemy) {
		item_max = Main_Data::game_enemyparty->GetBattlerCount();
	} else {
		item_max = Main_Data::game_party->GetBattlerCount();
	}

	if (item_max != old_item_max || Feature::HasRpg2k3BattleSystem()) {
		Refresh();
	}

	if (active && index >= 0) {
		if (Input::IsRepeated(Input::DOWN) || Input::IsRepeated(Input::RIGHT) || Input::IsTriggered(Input::SCROLL_DOWN)) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
			for (int i = 1; i < item_max; i++) {
				int new_index = (index + i) % item_max;
				if (IsChoiceValid((*Main_Data::game_party)[new_index])) {
					index = new_index;
					break;
				}
			}
		}
		if (Input::IsRepeated(Input::UP) || Input::IsRepeated(Input::LEFT) || Input::IsTriggered(Input::SCROLL_UP)) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
			for (int i = item_max - 1; i > 0; i--) {
				int new_index = (index + i) % item_max;
				if (IsChoiceValid((*Main_Data::game_party)[new_index])) {
					index = new_index;
					break;
				}
			}
		}
	}

	UpdateCursorRect();
}

void Window_BattleStatus::UpdateCursorRect() {
	if (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_gauge) {
		SetCursorRect(Rect());
		return;
	}

	if (index < 0)
		SetCursorRect(Rect());
	else
		SetCursorRect(Rect(0, index * menu_item_height, contents->GetWidth(), menu_item_height));
}

bool Window_BattleStatus::IsChoiceValid(const Game_Battler& battler) const {
	switch (mode) {
		case ChoiceMode_All:
			return true;
		case ChoiceMode_Alive:
			return !battler.IsDead();
		case ChoiceMode_Dead:
			return battler.IsDead();
		case ChoiceMode_Ready:
			return battler.IsAtbGaugeFull();
		case ChoiceMode_None:
			return false;
		default:
			assert(false && "Invalid Choice");
			return false;
	}
}

void Window_BattleStatus::RefreshActiveFromValid() {
	std::vector<Game_Battler*> battlers;
	if (enemy) {
		Main_Data::game_enemyparty->GetBattlers(battlers);
	} else {
		Main_Data::game_party->GetBattlers(battlers);
	}

	for (size_t i = 0; i < battlers.size(); ++i) {
		auto* battler = battlers[i];
		if (IsChoiceValid(*battler)) {
			if (!GetActive() || GetIndex() < 0) {
				SetIndex(i);
				SetActive(true);
			}
			return;
		}
		SetIndex(-1);
		SetActive(false);
	}
	UpdateCursorRect();
}
