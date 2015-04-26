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

#ifndef _WINDOW_BASE_H_
#define _WINDOW_BASE_H_

// Headers
#include <string>
#include "window.h"
#include "game_actor.h"
#include "main_data.h"

/**
 * Window Base class.
 */
class Window_Base : public Window {
public:
	/**
	 * Constructor.
	 *
	 * @param x window x position.
	 * @param y window y position.
	 * @param width window width.
	 * @param height window height.
	 */
	Window_Base(int x, int y, int width, int height);

	/**
	 * Updates the window.
	 */
	virtual void Update();

	virtual void Refresh();

	/**
	 * Draw helpers.
	 */
	/** @{ */
	void DrawFace(std::string face_name, int face_index, int cx, int cy, bool flip = false);
	void DrawActorFace(Game_Actor* actor, int cx, int cy);
	void DrawActorName(Game_Battler* actor, int cx, int cy);
	void DrawActorTitle(Game_Actor* actor, int cx, int cy);
	void DrawActorClass(Game_Actor* actor, int cx, int cy);
	void DrawActorLevel(Game_Actor* actor, int cx, int cy);
	void DrawActorState(Game_Battler* actor, int cx, int cy);
	void DrawActorExp(Game_Actor* actor, int cx, int cy);
	void DrawActorHp(Game_Battler* actor, int cx, int cy, bool draw_max = true);
	void DrawActorSp(Game_Battler* actor, int cx, int cy, bool draw_max = true);
	void DrawActorParameter(Game_Battler* actor, int cx, int cy, int type);
	void DrawEquipmentType(Game_Actor* actor, int cx, int cy, int type);
	void DrawItemName(RPG::Item* item, int cx, int cy, bool enabled = true);
	void DrawSkillName(RPG::Skill* skill, int cx, int cy, bool enabled = true);
	void DrawCurrencyValue(int money, int cx, int cy);
	void DrawGauge(Game_Battler* actor, int cx, int cy);
	/** @} */

protected:
	std::string windowskin_name;
};

#endif
