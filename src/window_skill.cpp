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
#include "bitmap.h"
#include "font.h"
#include "player.h"
#include "output.h"
#include <lcf/reader_util.h>
#include "game_battle.h"

Window_Skill::Window_Skill(Scene* parent, int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(parent, ix, iy, iwidth, iheight), actor_id(-1), subset(0) {
	column_max = 2;
}

void Window_Skill::SetActor(int actor_id) {
	this->actor_id = actor_id;
	Refresh();
}

const lcf::rpg::Skill* Window_Skill::GetSkill() const {
	if (index < 0) {
		return nullptr;
	}

	return lcf::ReaderUtil::GetElement(lcf::Data::skills, data[index]);
}

void Window_Skill::Refresh() {
	data.clear();

	const std::vector<int16_t>& skills = Main_Data::game_actors->GetActor(actor_id)->GetSkills();
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
		const Game_Actor* actor = Main_Data::game_actors->GetActor(actor_id);
		int costs = actor->CalculateSkillCost(skill_id);

		bool enabled = CheckEnable(skill_id);
		int color = !enabled ? Font::ColorDisabled : Font::ColorDefault;

		contents->TextDraw(rect.x + rect.width - 24, rect.y, color, fmt::format("{}{:3d}", lcf::rpg::Terms::TermOrDefault(lcf::Data::terms.easyrpg_skill_cost_separator, "-"), costs));

		// Skills are guaranteed to be valid
		DrawSkillName(*lcf::ReaderUtil::GetElement(lcf::Data::skills, skill_id), rect.x, rect.y, enabled);
	}
}

void Window_Skill::UpdateHelp() {
	help_window->SetText(GetSkill() == nullptr ? "" : ToString(GetSkill()->description));
}

bool Window_Skill::CheckInclude(int skill_id) {
	if (!Game_Battle::IsBattleRunning()) {
		return true;
	}

	if (Player::IsRPG2k()) {
		return true;
	}
	else {
		if (subset == 0) {
			return true;
		}

		const lcf::rpg::Skill* skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, skill_id);
		if (skill) {
			return skill->type == subset;
		}

		Output::Warning("Window Skill: Invalid skill ID {}", skill_id);
		return false;
	}
}

bool Window_Skill::CheckEnable(int skill_id) {
	const Game_Actor* actor = Main_Data::game_actors->GetActor(actor_id);

	return actor->IsSkillLearned(skill_id) && actor->IsSkillUsable(skill_id);
}

void Window_Skill::SetSubsetFilter(int subset) {
	this->subset = subset;
}
