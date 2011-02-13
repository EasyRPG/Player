/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
// 
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "window_battleskill.h"
#include "game_actors.h"
#include "game_actor.h"

////////////////////////////////////////////////////////////
Window_BattleSkill::Window_BattleSkill(int ix, int iy, int iwidth, int iheight) :
	Window_Skill(ix, iy, iwidth, iheight) {
}

////////////////////////////////////////////////////////////
void Window_BattleSkill::SetSubset(int id) {
	subset = id;
	Refresh();
}

////////////////////////////////////////////////////////////
bool Window_BattleSkill::CheckInclude(int skill_id) {
	const RPG::Skill& skill = Data::skills[skill_id - 1];
	return (subset == RPG::Skill::Type_normal)
		? (skill.type < 4)
		: (subset == skill.type);
}

////////////////////////////////////////////////////////////
bool Window_BattleSkill::CheckEnable(int skill_id) {
	const RPG::Skill& skill = Data::skills[skill_id - 1];
	return (skill.type == RPG::Skill::Type_switch)
		? skill.occasion_battle
		: true;
}

