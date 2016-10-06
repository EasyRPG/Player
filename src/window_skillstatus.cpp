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
#include "window_skillstatus.h"
#include "game_actor.h"
#include "game_actors.h"
#include "bitmap.h"
#include "font.h"

Window_SkillStatus::Window_SkillStatus(int ix, int iy, int iwidth, int iheight) :
	Window_Base(ix, iy, iwidth, iheight), actor_id(-1) {

	SetContents(Bitmap::Create(width - 16, height - 16));
}

void Window_SkillStatus::SetActor(int actor_id) {
	this->actor_id = actor_id;
	Refresh();
}

void Window_SkillStatus::Refresh() {
	contents->ClearRect(Rect(0, 0, contents->GetWidth(), 16));

	Game_Actor* actor = Game_Actors::GetActor(actor_id);

	DrawActorName(actor, 0, 0);
	DrawActorLevel(actor, 80, 0);
	DrawActorState(actor, 80 + 44, 0);
	DrawActorHp(actor, 80 + 44 + 54, 0);
	DrawActorSp(actor, 80 + 44 + 54 + 72, 0);
}
