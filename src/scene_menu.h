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
#include <boost/scoped_ptr.hpp>

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

	void Start();
	void Continue();
	void Update();

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
		Quit
	};

private:
	/** Selected index on startup. */
	int menu_index;

	/** Window displaying the commands. */
	boost::scoped_ptr<Window_Command> command_window;

	/** Window displaying the gold amount. */
	boost::scoped_ptr<Window_Gold> gold_window;

	/** Window displaying the heros and their status. */
	boost::scoped_ptr<Window_MenuStatus> menustatus_window;

	/** Options available in the menu. */
	std::vector<CommandOptionType> command_options;
};

#endif
