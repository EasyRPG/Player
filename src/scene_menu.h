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

#ifndef _SCENE_MENU_H_
#define _SCENE_MENU_H_

// Headers
#include "scene.h"
#include "window_command.h"
#include "window_gold.h"
#include "window_menustatus.h"

/**
 * Scene Menu class.
 */
class Scene_Menu : public Scene {
public:
	/**
	 * Constructor.
	 *
	 * @param menu_index selected index in the menu.
	 */
	Scene_Menu(int menu_index = 0);

	void Start() override;
	void Continue() override;
	void Update() override;

	/**
	 * Creates the window displaying the options.
	 */
	void CreateCommandWindow();

	/**
	 * Update function if command window is active.
	 */
	void UpdateCommand();

	/**
	 * Update function if status window is active.
	 */
	void UpdateActorSelection();

	/** Options available in a Rpg2k3 menu. */
	enum CommandOptionType {
		Item = 1,
		Skill,
		Equipment,
		Save,
		Status,
		Row,
		Order,
		Wait,
		Quit,
		// EasyRPG extra
		Debug = 100
	};

private:
	/** Selected index on startup. */
	int menu_index;

	/** Window displaying the commands. */
	std::unique_ptr<Window_Command> command_window;

	/** Window displaying the gold amount. */
	std::unique_ptr<Window_Gold> gold_window;

	/** Window displaying the heros and their status. */
	std::unique_ptr<Window_MenuStatus> menustatus_window;

	/** Options available in the menu. */
	std::vector<CommandOptionType> command_options;
};

#endif
