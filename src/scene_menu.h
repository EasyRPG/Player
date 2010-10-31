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
#include "window_status.h"

////////////////////////////////////////////////////////////
/// Scene Menu class
////////////////////////////////////////////////////////////
class Scene_Menu : public Scene {
public:
	Scene_Menu(int menu_index = 0);
	~Scene_Menu();

	void MainFunction();
	void Update();
	void UpdateCommand();
	void UpdateStatus();

private:
	int menu_index;
	Window_Command* command_window;
	Window_Gold* gold_window;
	Window_Status* status_window;
};

#endif
