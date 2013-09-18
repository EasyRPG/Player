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
#include <boost/scoped_ptr.hpp>
#include "scene.h"

class Window_Command;
class Window_VarList;
class Window_NumberInput;

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

	void Start();
	void Update();

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
		TypeSwitch
	};

private:
	/** Current variables being displayed (Switches or Integers). */
	VarType current_var_type;
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
	boost::scoped_ptr<Window_Command> range_window;
	/** Displays the vars inside the current range. */
	boost::scoped_ptr<Window_VarList> var_window;
	/** Number Editor. */
	boost::scoped_ptr<Window_NumberInput> numberinput_window;
};

#endif
