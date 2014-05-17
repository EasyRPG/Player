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
#include "bitmap.h"
#include "cache.h"
#include "input.h"
#include "game_enemyparty.h"
#include "game_party.h"
#include "game_actor.h"
#include "game_system.h"
#include "game_battle.h"
#include "player.h"
#include "window_battlestatus.h"

Window_BattleStatus::Window_BattleStatus(int ix, int iy, int iwidth, int iheight, bool enemy) :
	Window_Selectable(ix, iy, iwidth, iheight), mode(ChoiceMode_All), enemy(enemy) {

	SetBorderX(4);

	SetContents(Bitmap::Create(width - 8, height - 16));

	if (enemy) {
		item_max = Main_Data::game_enemyparty->GetBattlerCount();
	}
	else {
		item_max = Main_Data::game_party->GetBattlerCount();
	}

	item_max = std::min(item_max, 4);

	index = -1;

	Refresh();
}

void Window_BattleStatus::Refresh() {
	contents->Clear();

	for (int i = 0; i < item_max; i++) {
		int y = 2 + i * 16;

		Game_Battler* actor;
		if (enemy) {
			actor = &(*Main_Data::game_enemyparty)[i];
		}
		else {
			actor = &(*Main_Data::game_party)[i];
		}

		DrawActorName(actor, 4, y);
		DrawActorState(actor, 84, y);
		DrawActorHp(actor, 138, y, true);
		DrawActorSp(actor, 198, y, false);
	}

	RefreshGauge();
}

void Window_BattleStatus::RefreshGauge() {
	if (Player::engine == Player::EngineRpg2k3) {
		contents->ClearRect(Rect(198, 0, 25 + 16, 15 * item_max));

		for (int i = 0; i < item_max; ++i) {
			Game_Battler* actor;
			if (enemy) {
				actor = &(*Main_Data::game_enemyparty)[i];
			}
			else {
				actor = &(*Main_Data::game_party)[i];
			}

			int y = 2 + i * 16;
			DrawGauge(actor, 198 - 10, y - 2);
			DrawActorSp(actor, 198, y, false);
		}
	}
}

int Window_BattleStatus::ChooseActiveCharacter() {
	int old_index = index < 0 ? 0 : index;
	index = -1;
	for (int i = 0; i < item_max; i++) {
		int new_index = (old_index + i) % item_max;
		if ((*Main_Data::game_party)[new_index].IsGaugeFull()) {
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

	if (Player::engine == Player::EngineRpg2k3) {
		RefreshGauge();
	}

	if (active && index >= 0) {
		if (Input::IsRepeated(Input::DOWN)) {
			Game_System::SePlay(Main_Data::game_data.system.cursor_se);
			for (int i = 1; i < item_max; i++) {
				int new_index = (index + i) % item_max;
				if (IsChoiceValid((*Main_Data::game_party)[new_index])) {
					index = new_index;
					break;
				}
			}
		}
		if (Input::IsRepeated(Input::UP)) {
			Game_System::SePlay(Main_Data::game_data.system.cursor_se);
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
	if (index < 0)
		SetCursorRect(Rect());
	else
		SetCursorRect(Rect(0, index * 15, contents->GetWidth(), 16));
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
			return battler.IsGaugeFull();
		case ChoiceMode_None:
			return false;
		default:
			assert(false && "Invalid Choice");
			return false;
	}
}
