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
#include "player.h"

Window_SkillStatus::Window_SkillStatus(Scene* parent, int ix, int iy, int iwidth, int iheight) :
	Window_Base(parent, ix, iy, iwidth, iheight), actor_id(-1) {

	SetContents(Bitmap::Create(width - 16, height - 16));
}

void Window_SkillStatus::SetActor(int actor_id) {
	this->actor_id = actor_id;
	Refresh();
}

void Window_SkillStatus::Refresh() {
	contents->ClearRect(Rect(0, 0, contents->GetWidth(), 16));

	// Actors are guaranteed to be valid
	const Game_Actor& actor = *Main_Data::game_actors->GetActor(actor_id);

	int x = 0;
	int y = 2;
	DrawActorName(actor, x, y);
	x += 80;
	DrawActorLevel(actor, x, y);
	x += 44;
	DrawActorState(actor, x, y);
	int hpdigits = (actor.MaxHpValue() >= 1000) ? 4 : 3;
	int spdigits = (actor.MaxSpValue() >= 1000) ? 4 : 3;
	x += (96 - hpdigits * 6 - spdigits * 6);
	DrawActorHp(actor, x, y, hpdigits);
	x += (66 + hpdigits * 6 - spdigits * 6);
	DrawActorSp(actor, x, y, spdigits);
}
