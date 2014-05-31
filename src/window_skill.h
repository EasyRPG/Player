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

#ifndef _WINDOW_SKILL_H_
#define _WINDOW_SKILL_H_

// Headers
#include <vector>
#include "window_selectable.h"

/**
 * Window_Skill class.
 */
class Window_Skill : public Window_Selectable {

public:
	/**
	 * Constructor.
	 */
	Window_Skill(int ix, int iy, int iwidth, int iheight);

	/**
	 * Sets the actor whose skills are displayed.
	 *
	 * @param actor_id ID of the actor.
	 */
	void SetActor(int actor_id);

	/**
	 * Gets skill.
	 *
	 * @return current selected skill.
	 */
	const RPG::Skill* GetSkill() const;

	/**
	 * Refreshes the skill list.
	 */
	virtual void Refresh();

	/**
	 * Draws a skill together with the costs.
	 *
	 * @param index index of skill to draw.
	 */
	void DrawItem(int index);

	/**
	 * Updates the help window.
	 */
	void UpdateHelp();

	/**
	 * Checks if the skill should be in the list.
	 *
	 * @param skill_id skill to check.
	 */
	virtual bool CheckInclude(int skill_id);

	/**
	 * Checks if skill should be enabled.
	 *
	 * @param skill_id skill to check.
	 */
	virtual bool CheckEnable(int skill_id);

	/**
	 * Filters skill window by subset.
	 *
	 * @param subset Subset to filter for (0 for no subset)
	 */
	void SetSubsetFilter(int subset);

protected:
	std::vector<int> data;

	int actor_id;

	int subset;
};

#endif
