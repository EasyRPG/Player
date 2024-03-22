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
#include <iomanip>
#include <sstream>
#include "window_actorinfo.h"
#include "game_actors.h"
#include "game_party.h"
#include "bitmap.h"
#include "font.h"
#include "feature.h"

Window_ActorInfo::Window_ActorInfo(Scene* parent, int ix, int iy, int iwidth, int iheight, int actor_id) :
	Window_Base(parent, ix, iy, iwidth, iheight),
	actor_id(actor_id) {

	SetContents(Bitmap::Create(width - 16, height - 16));

	Refresh();
}

void Window_ActorInfo::Refresh() {
	contents->Clear();

	DrawInfo();
}

void Window_ActorInfo::DrawInfo() {
	if (Feature::HasRow()) {
		// Draw Row formation.
		std::string battle_row = Main_Data::game_actors->GetActor(actor_id)->GetBattleRow() == Game_Actor::RowType::RowType_back ? lcf::rpg::Terms::TermOrDefault(lcf::Data::terms.easyrpg_status_scene_back, "Back") : lcf::rpg::Terms::TermOrDefault(lcf::Data::terms.easyrpg_status_scene_front, "Front");
		contents->TextDraw(contents->GetWidth(), 2, Font::ColorDefault, battle_row, Text::AlignRight);
	}

	const Game_Actor& actor = *Main_Data::game_actors->GetActor(actor_id);

	// Draw Face
	DrawActorFace(actor, 0, 0);

	// Draw Name
	contents->TextDraw(0, 50, 1, lcf::rpg::Terms::TermOrDefault(lcf::Data::terms.easyrpg_status_scene_name, "Name"));
	DrawActorName(actor, 36, 66);

	// Draw Profession
	contents->TextDraw(0, 82, 1, lcf::rpg::Terms::TermOrDefault(lcf::Data::terms.easyrpg_status_scene_class, "Class"));
	DrawActorClass(actor, 36, 98);

	// Draw Rank
	contents->TextDraw(0, 114, 1, lcf::rpg::Terms::TermOrDefault(lcf::Data::terms.easyrpg_status_scene_title, "Title"));
	DrawActorTitle(actor, 36, 130);

	// Draw Status
	contents->TextDraw(0, 146, 1, lcf::rpg::Terms::TermOrDefault(lcf::Data::terms.easyrpg_status_scene_condition, "State"));
	DrawActorState(actor, 36, 162);

	//Draw Level
	contents->TextDraw(0, 178, 1, lcf::Data::terms.level);
	contents->TextDraw(78, 178, Font::ColorDefault, std::to_string(actor.GetLevel()), Text::AlignRight);
}
