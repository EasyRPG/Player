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

#ifndef _SCENE_EQUIP_H_
#define _SCENE_EQUIP_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <vector>
#include "scene.h"
#include "window_equipitem.h"
#include "window_equip.h"
#include "window_equipstatus.h"
#include "window_help.h"

////////////////////////////////////////////////////////////
/// Scene Equip class.
/// Displays the equipment of a hero.
////////////////////////////////////////////////////////////
class Scene_Equip : public Scene {

public:
	////////////////////////////////////////////////////////
	/// Constructor.
	/// @param actor_index : Actor in the party
	/// @param equip_index : Selected equipment
	////////////////////////////////////////////////////////
	Scene_Equip(int actor_index = 0, int equip_index = 0);

	void Start();
	void Update();
	void Terminate();

	////////////////////////////////////////////////////////
	/// Updates the Item Windows.
	////////////////////////////////////////////////////////
	void UpdateItemWindows();

	////////////////////////////////////////////////////////
	/// Updates the Equip Window.
	////////////////////////////////////////////////////////
	void UpdateEquipWindow();

	////////////////////////////////////////////////////////
	/// Updates the Status Window.
	////////////////////////////////////////////////////////
	void UpdateStatusWindow();

	////////////////////////////////////////////////////////
	/// Updates the Equip Window.
	////////////////////////////////////////////////////////
	void UpdateEquipSelection();
	
	////////////////////////////////////////////////////////
	/// Updates the Item Window.
	////////////////////////////////////////////////////////
	void UpdateItemSelection();

private:
	/// Actor in the party whose equipment is displayed
	int actor_index;
	/// Selected equipment on startup
	int equip_index;

	/// Displays available items in a category
	std::vector<Window_EquipItem*> item_windows;
	/// Current active item window
	Window_EquipItem* item_window;
	/// Displays stats of the hero/item
	Window_EquipStatus* equipstatus_window;
	/// Displays currently equipped items
	Window_Equip* equip_window;
	/// Displays description about the selected item
	Window_Help* help_window;
};

#endif
