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
#include "baseui.h"
#include "cache.h"
#include "game_party.h"
#include "bitmap.h"

Window_ActorTarget::Window_ActorTarget(int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(ix, iy, iwidth, iheight) {

	SetContents(Bitmap::Create(width - 16, height - 16));
	contents->SetTransparentColor(windowskin->GetTransparentColor());

	Refresh();
}

void Window_ActorTarget::Refresh() {
	contents->Clear();

	DisplayUi->SetBackcolor(Cache::system_info.bg_color);

	item_max = Game_Party::GetActors().size();

	int y = 0;
	for (int i = 0; i < item_max; ++i) {
		DrawActorFace(Game_Party::GetActors()[i], 0, i * 48 + y);
		DrawActorName(Game_Party::GetActors()[i], 48 + 8, i * 48 + 2 + y);
		DrawActorLevel(Game_Party::GetActors()[i], 48 + 8, i * 48 + 2 + 16 + y);
		DrawActorState(Game_Party::GetActors()[i], 48 + 8, i * 48 + 2 + 16 + 16 + y);
		DrawActorHp(Game_Party::GetActors()[i], 48 + 8 + 58, i * 48 + 2 + 16 + y);
		DrawActorSp(Game_Party::GetActors()[i], 48 + 8 + 58, i * 48 + 2 + 16 + 16 + y);

		y += 10;
	}
}

void Window_ActorTarget::UpdateCursorRect() {
	if (index < -10) { // Entire Party
		cursor_rect.Set(48 + 4, 0, 120, item_max * (48 + 10) - 10);
	} else if (index < 0) { // Fixed to one
		cursor_rect.Set(48 + 4, -index * (48 + 10), 120, 48);
	} else {
		cursor_rect.Set(48 + 4, index * (48 + 10), 120, 48);
	}
}
