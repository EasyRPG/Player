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

#ifndef EP_SCENE_DEBUG_H
#define EP_SCENE_DEBUG_H

// Headers
#include <vector>
#include "scene.h"
#include "window_command.h"
#include "window_numberinput.h"
#include "window_varlist.h"

/**
 * Scene Equip class.
 * Displays the equipment of a hero.
 */
class Scene_Debug : public Scene {

public:
	/**
	 * Constructor.
	 */
	Scene_Debug();

	void Start() override;
	void Update() override;

	/**
	 * Updates the range list window.
	 */
	void UpdateRangeListWindow();

	/**
	 * Updates the var list window.
	 */
	void UpdateVarListWindow();

	/**
	 * Updates the item window.
	 */
	void UpdateItemSelection();

	/**
	 * Gets an int with the current switch/variable selected.
	 */
	int GetIndex();

private:
	enum Mode {
		eMain,
		eSwitch,
		eVariable,
		eGold,
		eItem,
		eBattle
	};
	/** Current variables being displayed (Switches or Integers). */
	Mode mode = eMain;
	/** Current Page being displayed */
	int range_page = 0;
	/** Current range being displayed. */
	int range_index = 0;
	/** Last range_index used for switch */
	int prev_switch_range_index = 0;
	/** Last range page used for switch */
	int prev_switch_range_page = 0;
	/** Last range index used for variable */
	int prev_variable_range_index = 0;
	/** Last range page used for variable */
	int prev_variable_range_page = 0;
	/** Last range index used for item */
	int prev_item_range_index = 0;
	/** Last range page used for item */
	int prev_item_range_page = 0;
	/** Last range index used for troop */
	int prev_troop_range_index = 0;
	/** Last range page used for troop */
	int prev_troop_range_page = 0;

	/** Creates Range window. */
	void CreateRangeWindow();

	/** Creates variable list View window. */
	void CreateVarListWindow();

	/** Creates number input window. */
	void CreateNumberInputWindow();

	/** Get the last page for the current mode */
	int GetLastPage();

	/** Displays a range selection for mode. */
	std::unique_ptr<Window_Command> range_window;
	/** Displays the vars inside the current range. */
	std::unique_ptr<Window_VarList> var_window;
	/** Number Editor. */
	std::unique_ptr<Window_NumberInput> numberinput_window;
};

#endif
