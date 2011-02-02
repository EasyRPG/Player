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

#ifndef _SCENE_MENU_H_
#define _SCENE_MENU_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "scene.h"
#include "window_command.h"
#include "window_gold.h"
#include "window_menustatus.h"

////////////////////////////////////////////////////////////
/// Scene Menu class
////////////////////////////////////////////////////////////
class Scene_Menu : public Scene {
public:
	////////////////////////////////////////////////////////
	/// Constructor.
	/// @param menu_index : Selected index in the menu
	////////////////////////////////////////////////////////
	Scene_Menu(int menu_index = 0);

	void Start();
	void Update();
	void Terminate();

	////////////////////////////////////////////////////////
	/// Creates the Window displaying the options.
	////////////////////////////////////////////////////////
	void CreateCommandWindow();

	////////////////////////////////////////////////////////
	/// Update Function if Command Window is Active
	////////////////////////////////////////////////////////
	void UpdateCommand();

	////////////////////////////////////////////////////////
	/// Update Function if Status Window is Active
	////////////////////////////////////////////////////////
	void UpdateActorSelection();

private:
	/// selected index on startup
	int menu_index;

	/// window displaying the commands
	Window_Command* command_window;

	/// window displaying the gold amount
	Window_Gold* gold_window;

	/// window displaying the heros and their status
	Window_MenuStatus* menustatus_window;
};

#endif
