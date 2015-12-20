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

#ifndef _SCENE_GAMEBROWSER_H_
#define _SCENE_GAMEBROWSER_H_

// Headers
#include "scene.h"
#include "window_about.h"
#include "window_command.h"
#include "window_help.h"
#include "window_gamelist.h"
#include <boost/scoped_ptr.hpp>

/**
 * Game browser class.
 */
class Scene_GameBrowser : public Scene {
public:
	/**
	 * Constructor.
	 *
	 * @param menu_index selected index in the menu.
	 */
	Scene_GameBrowser();

	void Start();
	void Continue();
	void Update();

	/**
	 * Creates the window displaying the options.
	 */
	void CreateWindows();

	/**
	 * Update function if command window is active.
	 */
	void UpdateCommand();

	/**
	 * Update function if status window is active.
	 */
	void UpdateGameListSelection();

	/**
	 * Starts the selected game.
	 */
	void BootGame();

	/** Options available in a Rpg2k3 menu. */
	enum CommandOptionType {
		GameList = 0,
		About,
		Quit
	};

private:
	/** Window displaying settings */
	boost::scoped_ptr<Window_Command> command_window;

	/** Window displaying the games. */
	boost::scoped_ptr<Window_GameList> gamelist_window;

	/** Window displaying help text. */
	boost::scoped_ptr<Window_Help> help_window;

	/** Window displaying the loading text */
	boost::scoped_ptr<Window_Help> load_window;

	/** Window dislaying about text */
	boost::scoped_ptr<Window_About> about_window;

	bool game_loading = false;

	int old_gamelist_index = 0;
};

#endif
