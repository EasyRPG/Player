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
#include <sstream>
#include "window_targetstatus.h"
#include "game_party.h"
#include "bitmap.h"
#include "font.h"
#include <lcf/reader_util.h>

Window_TargetStatus::Window_TargetStatus(Scene* parent, int ix, int iy, int iwidth, int iheight) :
	Window_Base(parent, ix, iy, iwidth, iheight), id(-1), use_item(false) {

	SetContents(Bitmap::Create(width - 16, height - 16));
}

void Window_TargetStatus::Refresh() {
	contents->Clear();

	if (id < 0) {
		return;
	}

	if (use_item) {
		contents->TextDraw(0, 2, 1, lcf::Data::terms.possessed_items);
	} else {
		contents->TextDraw(0, 2, 1, lcf::Data::terms.sp_cost);
	}

	// Scene_ActorTarget validates items and skills
	std::string str;
	if (use_item) {
		str = std::to_string(Main_Data::game_party->GetItemCount(id));
	} else {
		str = std::to_string((*Main_Data::game_party)[actor_index].CalculateSkillCost(id));
	}

	FontRef font = Font::Default();
	contents->TextDraw(contents->GetWidth(), 2, Font::ColorDefault, str, Text::AlignRight);
}

void Window_TargetStatus::SetData(int id, bool is_item, int actor_index) {
	this->id = id;
	use_item = is_item;
	this->actor_index = actor_index;

	Refresh();
}
