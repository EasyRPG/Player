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

#ifndef EP_WINDOW_SKILL_H
#define EP_WINDOW_SKILL_H

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
	Window_Skill(Scene* parent, int ix, int iy, int iwidth, int iheight);

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
	const lcf::rpg::Skill* GetSkill() const;

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
	void UpdateHelp() override;

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

/**
 * Window_Skill class.
 */
class Window_BattleSkill : public Window_Skill {
	public:
		using Window_Skill::Window_Skill;

		/**
		 * Saves the value of GetIndex() for actor.
		 *
		 * @param which_actor which actor to save
		 **/
		void SaveActorIndex(int which_actor);

		/**
		 * Restores the index value for actor.
		 *
		 * @param which_actor which actor to restore
		 **/
		void RestoreActorIndex(int which_actor);

	private:
		std::array<int,4> actor_index = {{}};
};


inline void Window_BattleSkill::SaveActorIndex(int which_actor) {
	actor_index[which_actor] = GetIndex();
}

inline void Window_BattleSkill::RestoreActorIndex(int which_actor) {
	SetIndex(actor_index[which_actor]);
}

#endif
