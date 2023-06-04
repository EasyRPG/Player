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
	void vUpdate() override;
	void TransitionIn(SceneType prev_scene) override;
	void TransitionOut(SceneType next_scene) override;

	/**
	 * Updates the range list window.
	 */
	void UpdateRangeListWindow();

	/**
	 * Resets the remembered indices
	 */
	static void ResetPrevIndices();

	enum Mode {
		eMain,
		eSave,
		eLoad,
		eSwitch,
		eVariable,
		eGold,
		eItem,
		eBattle,
		eMap,
		eFullHeal,
		eLevel,
		eCallCommonEvent,
		eCallMapEvent,
		eCallBattleEvent,
		eOpenMenu,
		eLastMainMenuOption,
	};

	enum UiMode {
		eUiMain,
		eUiRangeList,
		eUiVarList,
		eUiNumberInput
	};
private:
	Mode mode = eMain;

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

	int GetNumMainMenuItems() const;

	void DoSwitch();
	void DoVariable();
	void DoGold();
	void DoItem();
	void DoBattle();
	void DoMap();
	void DoFullHeal();
	void DoLevel();
	void DoCallCommonEvent();
	void DoCallMapEvent();
	void DoCallBattleEvent();
	void DoOpenMenu();

	/** Displays a range selection for mode. */
	std::unique_ptr<Window_Command> range_window;
	/** Displays the vars inside the current range. */
	std::unique_ptr<Window_VarList> var_window;
	/** Number Editor. */
	std::unique_ptr<Window_NumberInput> numberinput_window;

	struct StackFrame {
		UiMode uimode = eUiMain;
		int value = 0;
	};
	std::array<StackFrame,8> stack;
	int stack_index = 0;

	StackFrame& GetFrame(int n = 0);
	const StackFrame& GetFrame(int n = 0) const;

	int GetStackSize() const;

	void Push(UiMode ui);
	void Pop();

	void SetupUiRangeList();
	void PushUiRangeList();
	void PushUiVarList();
	void PushUiNumberInput(int init_value, int digits, bool show_operator);

	Window_VarList::Mode GetWindowMode() const;
	void UpdateFrameValueFromUi();

	bool IsValidMapId(int map_id) const;

	void UpdateArrows();
	int arrow_frame = 0;
};

#endif
