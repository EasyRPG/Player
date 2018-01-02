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
#include "window_skill.h"
#include "game_actor.h"
#include "game_actors.h"
#include "game_party.h"
#include "game_temp.h"
#include "bitmap.h"
#include "font.h"
#include "player.h"
#include "output.h"
#include "reader_util.h"

Window_Skill::Window_Skill(int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(ix, iy, iwidth, iheight), actor_id(-1), subset(0) {
	column_max = 2;
}

void Window_Skill::SetActor(int actor_id) {
	this->actor_id = actor_id;
	Refresh();
}

const RPG::Skill* Window_Skill::GetSkill() const {
	if (index < 0) {
		return nullptr;
	}

	return ReaderUtil::GetElement(Data::skills, data[index]);
}

void Window_Skill::Refresh() {
	data.clear();

	const std::vector<int16_t>& skills = Game_Actors::GetActor(actor_id)->GetSkills();
	for (size_t i = 0; i < skills.size(); ++i) {
		if (CheckInclude(skills[i]))
			data.push_back(skills[i]);
	}

	if (data.size() == 0) {
		data.push_back(0);
	}

	item_max = data.size();

	CreateContents();

	contents->Clear();

	for (int i = 0; i < item_max; ++i) {
		DrawItem(i);
	}
}

void Window_Skill::DrawItem(int index) {
	Rect rect = GetItemRect(index);
	contents->ClearRect(rect);

	int skill_id = data[index];

	if (skill_id > 0) {
		const Game_Actor* actor = Game_Actors::GetActor(actor_id);
		int costs = actor->CalculateSkillCost(skill_id);

		bool enabled = CheckEnable(skill_id);
		int color = !enabled ? Font::ColorDisabled : Font::ColorDefault;

		std::stringstream ss;
		ss << costs;
		contents->TextDraw(rect.x + rect.width - 28, rect.y, color, "-");
		contents->TextDraw(rect.x + rect.width - 6, rect.y, color, ss.str(), Text::AlignRight);

		// Skills are guaranteed to be valid
		DrawSkillName(*ReaderUtil::GetElement(Data::skills, skill_id), rect.x, rect.y, enabled);
	}
}

void Window_Skill::UpdateHelp() {
	help_window->SetText(GetSkill() == NULL ? "" :
		GetSkill()->description);
}

bool Window_Skill::CheckInclude(int skill_id) {
	if (!Game_Temp::battle_running) {
		return true;
	}

	if (Player::IsRPG2k()) {
		return true;
	}
	else {
		if (subset == 0) {
			return true;
		}

		const RPG::Skill* skill = ReaderUtil::GetElement(Data::skills, skill_id);
		if (skill) {
			return skill->type == subset;
		}

		Output::Warning("Window Skill: Invalid skill ID %d", skill_id);
		return false;
	}
}

bool Window_Skill::CheckEnable(int skill_id) {
	const Game_Actor* actor = Game_Actors::GetActor(actor_id);

	return actor->IsSkillLearned(skill_id) && Main_Data::game_party->IsSkillUsable(skill_id, actor);
}

void Window_Skill::SetSubsetFilter(int subset) {
	this->subset = subset;
}
