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
#include "window_menustatus.h"
#include "cache.h"
#include "game_party.h"
#include "player.h"
#include "bitmap.h"
#include "feature.h"

Window_MenuStatus::Window_MenuStatus(int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(ix, iy, iwidth, iheight) {

	if (Player::IsRPG2k3()) {
		SetContents(Bitmap::Create(width - 12, height - 16));
		SetBorderX(4);
		text_offset = 4;
	} else {
		SetContents(Bitmap::Create(width - 16, height - 16));
	}

	Refresh();
}

void Window_MenuStatus::Refresh() {
	contents->Clear();

	item_max = Main_Data::game_party->GetActors().size();

	int y = 0;
	for (int i = 0; i < item_max; ++i) {
		// The party always contains valid battlers
		const Game_Actor& actor = *(Main_Data::game_party->GetActors()[i]);

		int face_x = 0;
		if (Player::IsRPG2k3()) {
			if (!Feature::HasRow()) {
				face_x = 4;
			} else {
				face_x = actor.GetBattleRow() == Game_Actor::RowType::RowType_back ? 8 : 0;
			}
		}
		DrawActorFace(actor, face_x, i*48 + y);

		DrawActorName(actor, 48 + 8 + text_offset, i*48 + 2 + y);
		DrawActorTitle(actor, 48 + 8 + 88 + text_offset, i*48 + 2 + y);
		DrawActorLevel(actor, 48 + 8 + text_offset, i*48 + 2 + 16 + y);
		DrawActorState(actor, 48 + 8 + 42 + text_offset, i*48 + 2 + 16 + y);
		DrawActorExp(actor, 48 + 8 + text_offset, i*48 + 2 + 16 + 16 + y);
		int digits = (actor.MaxHpValue() >= 1000 || actor.MaxSpValue() >= 1000) ? 4 : 3;
		DrawActorHp(actor, 48 + 8 + 106 + text_offset - (digits == 3 ? 0 : 12), i * 48 + 2 + 16 + y, digits);
		DrawActorSp(actor, 48 + 8 + 106 + text_offset - (digits == 3 ? 0 : 12), i * 48 + 2 + 16 + 16 + y, digits);

		y += 10;
	}
}

void Window_MenuStatus::UpdateCursorRect()
{
	if (index < 0) {
		cursor_rect = { 0, 0, 0, 0 };
	} else {
		cursor_rect = { 48 + 4 + text_offset, index * (48 + 10), 168, 48 };
	}
}

Game_Actor* Window_MenuStatus::GetActor() const {
	return &(*Main_Data::game_party)[GetIndex()];
}
