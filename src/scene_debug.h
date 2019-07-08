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

	/**
	 * Resets the remembered indices
	 */
	static void ResetPrevIndices();

private:
	enum Mode {
		eMain,
		eSwitch,
		eSwitchSelect,
		eVariable,
		eVariableSelect,
		eVariableValue,
		eGold,
		eItem,
		eItemSelect,
		eItemValue,
		eBattle,
		eBattleSelect,
		eMap,
		eMapSelect,
		eMapX,
		eMapY,
		eFullHeal,
		eCallEvent,
		eCallEventSelect
	};
	/** Current variables being displayed (Switches or Integers). */
	Mode mode = eMain;

	struct PrevIndex;
	struct IndexSet;

	static PrevIndex prev;

	/** Current Page being displayed */
	int range_page = 0;
	/** Current range being displayed. */
	int range_index = 0;

	/** Creates Range window. */
	void CreateRangeWindow();

	/** Creates variable list View window. */
	void CreateVarListWindow();

	/** Creates number input window. */
	void CreateNumberInputWindow();

	/** Get the last page for the current mode */
	int GetLastPage();

	void SetupListOption(Mode mode, Window_VarList::Mode winmode, const IndexSet& idx);
	void UseRangeWindow();
	void UseVarWindow();
	void UseNumberWindow();

	void EnterFromMain();
	void EnterFromListOption(Mode m, const IndexSet& idx);
	void EnterFromListOptionToValue(Mode m, int init_value, int digits, bool show_operator);

	void EnterGold();
	void EnterMapSelectX();
	void EnterMapSelectY();
	void EnterFullHeal();

	void CancelListOption(IndexSet& idx, int from_idx);
	void CancelListOptionSelect(Mode m, IndexSet& idx);
	void CancelListOptionValue(Mode m);

	void CancelMapSelectY();

	void ReturnToMain(int from_idx);

	void DoSwitch();
	void DoVariable();
	void DoGold();
	void DoItem();
	void DoBattle();
	void DoMap();
	void DoFullHeal();
	void DoCallEvent();

	/** Displays a range selection for mode. */
	std::unique_ptr<Window_Command> range_window;
	/** Displays the vars inside the current range. */
	std::unique_ptr<Window_VarList> var_window;
	/** Number Editor. */
	std::unique_ptr<Window_NumberInput> numberinput_window;

	int pending_map_id = 0;
	int pending_map_x = 0;
	int pending_map_y = 0;

};

#endif
