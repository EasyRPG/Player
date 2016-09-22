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
#include "baseui.h"
#include "window_menustatus.h"
#include "graphics.h"
#include "cache.h"
#include "game_party.h"
#include "player.h"
#include "bitmap.h"

Window_MenuStatus::Window_MenuStatus(int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(ix, iy, iwidth, iheight) {

	SetContents(Bitmap::Create(width - 16, height - 16));

	Refresh();
}

void Window_MenuStatus::Refresh() {
	contents->Clear();

	item_max = Main_Data::game_party->GetActors().size();

	int y = 0;
	for (int i = 0; i < item_max; ++i)
	{
		Game_Actor* actor = Main_Data::game_party->GetActors()[i];

		int face_x = 0;
		if (Player::IsRPG2k3()) {
			face_x = actor->GetBattleRow() == 1 ? 5 : 0;
		}
		DrawActorFace(actor, face_x, i*48 + y);

		DrawActorName(actor, 48 + 8, i*48 + 2 + y);
		DrawActorTitle(actor, 48 + 8 + 88, i*48 + 2 + y);
		DrawActorLevel(actor, 48 + 8, i*48 + 2 + 16 + y);
		DrawActorState(actor, 48 + 8 + 42, i*48 + 2 + 16 + y);
		DrawActorExp(actor, 48 + 8, i*48 + 2 + 16 + 16 + y);
		DrawActorHp(actor, 48 + 8 + 106 - (Player::IsRPG2k() ? 0 : 12), i * 48 + 2 + 16 + y);
		DrawActorSp(actor, 48 + 8 + 106 - (Player::IsRPG2k() ? 0 : 12), i * 48 + 2 + 16 + 16 + y);

		y += 10;
	}
}

void Window_MenuStatus::UpdateCursorRect()
{
	if (index < 0) {
		cursor_rect.Set(0, 0, 0, 0);
	} else {
		cursor_rect.Set(48 + 4, index * (48 + 10), 168, 48);
	}
}

Game_Actor* Window_MenuStatus::GetActor() const {
	return &(*Main_Data::game_party)[GetIndex()];
}
