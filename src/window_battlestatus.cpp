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
#include "window_battlestatus.h"

Window_BattleStatus::Window_BattleStatus(int ix, int iy, int iwidth, int iheight, bool enemy) :
	Window_Selectable(ix, iy, iwidth, iheight), mode(ChoiceMode_All), enemy(enemy) {

	SetBorderX(4);

	SetContents(Bitmap::Create(width - 8, height - 16));

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

	Refresh();
}

void Window_BattleStatus::Refresh() {
	contents->Clear();

	if (enemy) {
		item_max = Main_Data::game_enemyparty->GetBattlerCount();
	}
	else {
		item_max = Main_Data::game_party->GetBattlerCount();
	}

	item_max = std::min(item_max, 4);

	for (int i = 0; i < item_max; i++) {
		// The party only contains valid battlers
		const Game_Battler* actor;
		if (enemy) {
			actor = &(*Main_Data::game_enemyparty)[i];
		}
		else {
			actor = &(*Main_Data::game_party)[i];
		}

		if (!enemy && lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_gauge) {
			DrawActorFace(*static_cast<const Game_Actor*>(actor), 80 * i, 24);
		}
		else {
			int y = 2 + i * 16;

			DrawActorName(*actor, 4, y);
			DrawActorState(*actor, 84, y);
			if (Player::IsRPG2k3() && lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_traditional) {
				contents->TextDraw(126 + 42 + 4 * 6, y, Font::ColorDefault, std::to_string(actor->GetHp()), Text::AlignRight);
			} else {
				int digits = Player::IsRPG2k() ? 3 : 4;
				DrawActorHp(*actor, 126, y, digits, true);
				DrawActorSp(*actor, 198, y, 3, false);
			}
		}
	}

	RefreshGauge();
}

void Window_BattleStatus::RefreshGauge() {
	if (Player::IsRPG2k3()) {
		if (lcf::Data::battlecommands.battle_type != lcf::rpg::BattleCommands::BattleType_gauge) {
			contents->ClearRect(Rect(198, 0, 25 + 16, 15 * item_max));
		}

		for (int i = 0; i < item_max; ++i) {
			// The party always contains valid battlers
			Game_Battler* actor;
			if (enemy) {
				actor = &(*Main_Data::game_enemyparty)[i];
			}
			else {
				actor = &(*Main_Data::game_party)[i];
			}

			if (!enemy && lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_gauge) {
				BitmapRef system2 = Cache::System2();
				if (system2) {
					// Clear number and gauge drawing area
					contents->ClearRect(Rect(40 + 80 * i, 24, 8 * 4, 48));

					// Number clearing removed part of the face, but both, clear and redraw
					// are needed because some games don't have face graphics that are huge enough
					// to clear the number area (e.g. Ara Fell)
					DrawActorFace(*static_cast<const Game_Actor*>(actor), 80 * i, 24);

					int x = 32 + i * 80;
					int y = 24;

					// Left Gauge
					contents->Blit(x, y, *system2, Rect(0, 32, 16, 48), Opacity::Opaque());
					x += 16;

					// Center
					const auto fill_x = x;
					contents->StretchBlit(Rect(x, y, 25, 48), *system2, Rect(16, 32, 16, 48), Opacity::Opaque());
					x += 25;

					// Right
					contents->Blit(x, y, *system2, Rect(32, 32, 16, 48), Opacity::Opaque());

					// HP
					DrawGaugeSystem2(fill_x, y, actor->GetHp(), actor->GetMaxHp(), 0);
					// SP
					DrawGaugeSystem2(fill_x, y + 16, actor->GetSp(), actor->GetMaxSp(), 1);
					// Gauge
					DrawGaugeSystem2(fill_x, y + 16 * 2, actor->GetAtbGauge(), actor->GetMaxAtbGauge(), 2);

					// Numbers
					x = 40 + 80 * i;
					DrawNumberSystem2(x, y, actor->GetHp());
					DrawNumberSystem2(x, y + 12 + 4, actor->GetSp());
				}
			}
			else {
				int y = 2 + i * 16;

				DrawGauge(*actor, 198 - 10, y - 2);
				if (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_alternative) {
					DrawActorSp(*actor, 198, y, 3, false);
				}
			}
		}
	}
}

void Window_BattleStatus::DrawGaugeSystem2(int x, int y, int cur_value, int max_value, int which) {
	BitmapRef system2 = Cache::System2();
	assert(system2);

	int gauge_x;
	if (cur_value == max_value) {
		gauge_x = 16;
	}
	else {
		gauge_x = 0;
	}

	int gauge_width = 25;

	if (max_value > 0) {
		gauge_width = 25 * cur_value / max_value;
	}

	contents->StretchBlit(Rect(x, y, gauge_width, 16), *system2, Rect(48 + gauge_x, 32 + 16 * which, 16, 16), Opacity::Opaque());
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

	if (item_max != old_item_max) {
		Refresh();
	} else if (Player::IsRPG2k3()) {
		RefreshGauge();
	}

	if (active && index >= 0) {
		if (Input::IsRepeated(Input::DOWN) || Input::IsTriggered(Input::SCROLL_DOWN)) {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cursor));
			for (int i = 1; i < item_max; i++) {
				int new_index = (index + i) % item_max;
				if (IsChoiceValid((*Main_Data::game_party)[new_index])) {
					index = new_index;
					break;
				}
			}
		}
		if (Input::IsRepeated(Input::UP) || Input::IsTriggered(Input::SCROLL_UP)) {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cursor));
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
	if (lcf::Data::battlecommands.battle_type != lcf::rpg::BattleCommands::BattleType_traditional) {
		SetCursorRect(Rect());
		return;
	}

	if (index < 0)
		SetCursorRect(Rect());
	else
		SetCursorRect(Rect(0, index * 16, contents->GetWidth(), 16));
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

