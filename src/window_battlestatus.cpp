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
#include "game_party.h"
#include "game_actor.h"
#include "game_system.h"
#include "game_battle.h"
#include "player.h"
#include "window_battlestatus.h"

Window_BattleStatus::Window_BattleStatus(int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(ix, iy, iwidth, iheight) {

	SetBorderX(4);

	SetContents(Bitmap::Create(width - 8, height - 16));

	item_max = Main_Data::game_party->GetBattlerCount();

	index = -1;

	Refresh();
}

void Window_BattleStatus::Refresh() {
	contents->Clear();

	for (int i = 0; i < item_max && i < 4; i++) {
		int y = 2 + i * 16;
		Game_Actor* actor = Main_Data::game_party->GetActors()[i];
		DrawActorName(actor, 4, y);
		DrawActorState(actor, 84, y);
		DrawActorHp(actor, 138, y, true);
		DrawActorSp(actor, 198, y, false);
	}
}

void Window_BattleStatus::RefreshGauge() {
	contents->ClearRect(Rect(198, 0, 25 + 16, 15 * item_max));

	for (int i = 0; i < item_max; ++i) {
		Game_Actor* actor = Main_Data::game_party->GetActors()[i];
		int y = 2 + i * 16;
		DrawGauge(actor, 198 - 10, y - 2);
		DrawActorSp(actor, 198, y, false);
	}
}

void Window_BattleStatus::DrawGauge(Game_Actor* actor, int cx, int cy) {
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

void Window_BattleStatus::SetActiveCharacter(int _index) {
	index = _index;
	Refresh();
}

int Window_BattleStatus::GetActiveCharacter() {
	return index;
}

void Window_BattleStatus::ChooseActiveCharacter() {
	int num_actors = Game_Battle::allies.size();
	int old_index = index < 0 ? 0 : index;
	index = -1;
	for (int i = 0; i < num_actors; i++) {
		int new_index = (old_index + i) % num_actors;
		if (Game_Battle::GetAlly(new_index).IsReady()) {
			index = new_index;
			break;
		}
	}

	if (index != old_index)
		UpdateCursorRect();
}

void Window_BattleStatus::Update() {
	Window_Selectable::Update();

	if (Player::engine == Player::EngineRpg2k3) {
		RefreshGauge();

		if (active && index >= 0) {
			if (Input::IsRepeated(Input::DOWN)) {
				Game_System::SePlay(Main_Data::game_data.system.cursor_se);
				for (int i = 1; i < item_max; i++) {
					int new_index = (index + i) % item_max;
					if (Game_Battle::GetAlly(new_index).IsReady()) {
						index = new_index;
						break;
					}
				}
			}
			if (Input::IsRepeated(Input::UP)) {
				Game_System::SePlay(Main_Data::game_data.system.cursor_se);
				for (int i = item_max - 1; i > 0; i--) {
					int new_index = (index + i) % item_max;
					if (Game_Battle::GetAlly(new_index).IsReady()) {
						index = new_index;
						break;
					}
				}
			}
		}

		ChooseActiveCharacter();

		UpdateCursorRect();
	}
}

void Window_BattleStatus::UpdateCursorRect() {
	if (index < 0)
		SetCursorRect(Rect());
	else
		SetCursorRect(Rect(0, index * 15, contents->GetWidth(), 16));
}
