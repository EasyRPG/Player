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

#ifndef _SCENE_DEBUG_H_
#define _SCENE_DEBUG_H_

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

	enum VarType {
		TypeInt,
		TypeSwitch,
		TypeGeneral,
		TypeEnd
	};

private:
	/** Current variables being displayed (Switches or Integers). */
	int current_var_type;
	/** Current Page being displayed */
	int range_page;
	/** Current range being displayed. */
	int range_index;

	/** Creates Range window. */
	void CreateRangeWindow();

	/** Creates variable list View window. */
	void CreateVarListWindow();

	/** Creates number input window. */
	void CreateNumberInputWindow();

	/** Displays a range selection for current var type. */
	std::unique_ptr<Window_Command> range_window;
	/** Displays the vars inside the current range. */
	std::unique_ptr<Window_VarList> var_window;
	/** Number Editor. */
	std::unique_ptr<Window_NumberInput> numberinput_window;
};

#endif
