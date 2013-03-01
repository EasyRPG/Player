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

#ifndef _WINDOW_BATTLESKILL_H_
#define _WINDOW_BATTLESKILL_H_

// Headers
#include <vector>
#include "window_skill.h"

/**
 * Window_BattleSkill class.
 */
class Window_BattleSkill : public Window_Skill {

public:
	/**
	 * Constructor.
	 */
	Window_BattleSkill(int ix, int iy, int iwidth, int iheight);

	/**
	 * Sets the actor whose skills are displayed.
	 *
	 * @param id ID of the actor.
	 */
	void SetSubset(int id);

	/**
	 * Checks if the skill should be in the list.
	 *
	 * @param skill_id skill to check.
	 */
	bool CheckInclude(int skill_id);

	/**
	 * Chechs if skill should be enabled.
	 *
	 * @param skill_id skill to check.
	 */
	bool CheckEnable(int skill_id);

private:
	int subset;
};

#endif
