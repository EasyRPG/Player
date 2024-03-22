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
#include "window_actortarget.h"
#include "game_actor.h"
#include "game_party.h"
#include "bitmap.h"
#include "player.h"

Window_ActorTarget::Window_ActorTarget(Scene* parent, int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(parent, ix, iy, iwidth, iheight) {

	SetContents(Bitmap::Create(width - 16, height - 16));

	Refresh();
}

void Window_ActorTarget::Refresh() {
	contents->Clear();

	item_max = Main_Data::game_party->GetActors().size();

	int y = 0;
	for (int i = 0; i < item_max; ++i) {
		const Game_Actor& actor = *(Main_Data::game_party->GetActors()[i]);

		DrawActorFace(actor, 0, i * 48 + y);
		DrawActorName(actor, 48 + 8, i * 48 + 2 + y);
		DrawActorLevel(actor, 48 + 8, i * 48 + 2 + 16 + y);
		DrawActorState(actor, 48 + 8, i * 48 + 2 + 16 + 16 + y);
		int digits = (actor.MaxHpValue() >= 1000 || actor.MaxSpValue() >= 1000) ? 4 : 3;
		int x_offset = 48 + 8 + 46 + (digits == 3 ? 12 : 0);
		DrawActorHp(actor, x_offset, i * 48 + 2 + 16 + y, digits);
		DrawActorSp(actor, x_offset, i * 48 + 2 + 16 + 16 + y, digits);

		y += 10;
	}
}

void Window_ActorTarget::UpdateCursorRect() {
	if (index < -10) { // Entire Party
		cursor_rect = { 48 + 4, 0, 120, item_max * (48 + 10) - 10 };
	} else if (index < 0) { // Fixed to one
		cursor_rect = { 48 + 4, (-index - 1) * (48 + 10), 120, 48 };
	} else {
		cursor_rect = { 48 + 4, index * (48 + 10), 120, 48 };
	}
}

Game_Actor* Window_ActorTarget::GetActor() {
	int ind = GetIndex();
	if (ind >= -10 && ind < 0) {
		ind = -ind - 1;
	}
	else if (ind == -100) {
		return nullptr;
	}

	return &(*Main_Data::game_party)[ind];
}
