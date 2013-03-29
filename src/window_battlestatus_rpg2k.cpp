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
#include "window_battlestatus_rpg2k.h"

Window_BattleStatus_Rpg2k::Window_BattleStatus_Rpg2k(int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(ix, iy, iwidth, iheight) {

	SetContents(Bitmap::Create(width - 16, height - 16));
	contents->SetTransparentColor(windowskin->GetTransparentColor());

	item_max = Game_Party::GetActors().size();

	Refresh();
}

Window_BattleStatus_Rpg2k::~Window_BattleStatus_Rpg2k() {
}

void Window_BattleStatus_Rpg2k::Refresh() {
	contents->Clear();

	for (size_t i = 0; i < Game_Party::GetActors().size() && i < 4; i++) {
		int y = 2 + i * 16;
		Game_Actor* actor = Game_Party::GetActors()[i];
		DrawActorName(actor, 0, y);
		DrawActorState(actor, 84, y);
		DrawActorHp(actor, 138, y, true);
		DrawActorSp(actor, 198, y, false);
	}
}

void Window_BattleStatus_Rpg2k::UpdateCursorRect() {
	if (index < 0)
		SetCursorRect(Rect());
	else
		SetCursorRect(Rect(0, index * 16, contents->GetWidth(), 16));
}

void Window_BattleStatus_Rpg2k::ChooseActiveCharacter() {
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
}

void Window_BattleStatus_Rpg2k::Update()
{
	Window_Selectable::Update();
}
