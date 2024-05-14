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
#include "window_stringview.h"
#include "window_interpreter.h"

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
		eMoveSpeed,
		eCallCommonEvent,
		eCallMapEvent,
		eCallBattleEvent,
		eString,
		eInterpreter,
		eOpenMenu,
		eLastMainMenuOption,
	};

	enum UiMode {
		eUiMain,
		eUiRangeList,
		eUiVarList,
		eUiNumberInput,
		eUiStringView,
		eUiChoices,
		eUiInterpreterView
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

	/** Creates choices window. */
	void CreateChoicesWindow();

	/** Creates string view window. */
	void CreateStringViewWindow();

	/** Creates interpreter window. */
	void CreateInterpreterWindow();


	/** Get the last page for the current mode */
	int GetLastPage();

	/** Get the first item number for the selected range */
	int GetSelectedIndexFromRange() const;

	void RestoreRangeSelectionFromSelectedValue(int value);

	int GetNumMainMenuItems() const;

	void DoSwitch();
	void DoVariable();
	void DoGold();
	void DoItem();
	void DoBattle();
	void DoMap();
	void DoFullHeal();
	void DoLevel();
	void DoMoveSpeed();
	void DoCallCommonEvent();
	void DoCallMapEvent();
	void DoCallBattleEvent();
	void DoOpenMenu();

	const int choice_window_width = 120;

	/** Displays a range selection for mode. */
	std::unique_ptr<Window_Command> range_window;
	/** Displays the vars inside the current range. */
	std::unique_ptr<Window_VarList> var_window;
	/** Number Editor. */
	std::unique_ptr<Window_NumberInput> numberinput_window;
	/** Choices window. */
	std::unique_ptr<Window_Command> choices_window;
	/** Windows for displaying multiline strings. */
	std::unique_ptr<Window_StringView> stringview_window;
	/** Displays the currently running inteprreters. */
	std::unique_ptr<Window_Interpreter> interpreter_window;

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
	void PushUiChoices(std::vector<std::string> choices, std::vector<bool> choices_enabled);
	void PushUiStringView();
	void PushUiInterpreterView();

	Window_VarList::Mode GetWindowMode() const;
	void UpdateFrameValueFromUi();
	void UpdateDetailWindow();
	void RefreshDetailWindow();

	bool IsValidMapId(int map_id) const;

	void UpdateArrows();
	int arrow_frame = 0;

	bool strings_cached = false;
	std::vector<lcf::DBString> strings;

	bool interpreter_states_cached = false;

	void UpdateInterpreterWindow(int index);
	lcf::rpg::SaveEventExecFrame& GetSelectedInterpreterFrameFromUiState() const;
	void CacheBackgroundInterpreterStates();
	struct {
		std::vector<int> ev;
		std::vector<int> ce;
		std::vector<lcf::rpg::SaveEventExecState> state_ev;
		std::vector<lcf::rpg::SaveEventExecState> state_ce;

		// Frame-scoped data types introduced in 'ScopedVars' branch
		// bool show_frame_switches = false;
		// bool show_frame_vars = false;
		int selected_state = -1;
		int selected_frame = -1;
	} state_interpreter;
};

#endif
