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

#ifndef EP_WINDOW_BATTLECOMMAND_H
#define EP_WINDOW_BATTLECOMMAND_H

// Headers
#include <string>
#include <vector>
#include "window_base.h"
#include <lcf/rpg/battlecommand.h>
#include "font.h"

/**
 * Window_BattleCommand class.
 */
class Window_BattleCommand: public Window_Base {
public:
	/**
	 * Constructor.
	 */
	Window_BattleCommand(Scene* parent, int x, int y, int width, int height);

	/**
	 * Refreshes the window contents.
	 */
	void Refresh();

	/**
	 * Updates the window state.
	 */
	void Update() override;

	/**
	 * Enables or disables a command.
	 *
	 * @param index command index.
	 * @param enabled whether the command is enabled.
	 */
	void SetEnabled(int index, bool enabled);

	void SetActor(int actor_id);

	int GetIndex();
	void SetIndex(int index);
	void SetActive(bool active);
	void UpdateCursorRect();
	int GetSkillSubset();

protected:
	int actor_id;
	std::vector<std::string> commands;
	int index;
	int num_rows;
	int top_row;
	std::vector<bool> disabled;
	int cycle;

	void DrawItem(int index, Font::SystemColor color);
};

#endif
