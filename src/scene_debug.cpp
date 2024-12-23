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

// Headers
#include <vector>
#include <sstream>
#include <cmath>
#include <iomanip>
#include "baseui.h"
#include "cache.h"
#include "input.h"
#include "game_variables.h"
#include "game_switches.h"
#include "game_strings.h"
#include "game_map.h"
#include "game_system.h"
#include "game_battle.h"
#include "scene_debug.h"
#include "scene_load.h"
#include "scene_menu.h"
#include "scene_save.h"
#include "scene_map.h"
#include "scene_battle.h"
#include "player.h"
#include "window_command.h"
#include "window_varlist.h"
#include "window_numberinput.h"
#include "bitmap.h"
#include "game_party.h"
#include "game_player.h"
#include <lcf/data.h>
#include "output.h"
#include "transition.h"
#include "lcf/reader_util.h"

namespace {
struct IndexSet {
	int range_index = 0;
	int range_page = 0;
	int range_page_index = 0;
};

std::array<IndexSet,Scene_Debug::eLastMainMenuOption> prev = {};
}

constexpr int arrow_animation_frames = 20;

Scene_Debug::Scene_Debug() {
	Scene::type = Scene::Debug;
}

void Scene_Debug::ResetPrevIndices() {
	prev = {};
}

void Scene_Debug::Start() {
	CreateRangeWindow();
	CreateVarListWindow();
	CreateNumberInputWindow();
	CreateChoicesWindow();
	CreateStringViewWindow();
	CreateInterpreterWindow();

	SetupUiRangeList();

	range_window->SetActive(true);
	var_window->SetActive(false);
	stringview_window->SetActive(false);
	interpreter_window->SetActive(false);

	UpdateRangeListWindow();
	RefreshDetailWindow();

	strings_cached = false;
	interpreter_states_cached = false;
}

Scene_Debug::StackFrame& Scene_Debug::GetFrame(int n) {
	auto i = stack_index - n;
	assert(i >= 0 && i < static_cast<int>(stack.size()));
	return stack[i];
}

const Scene_Debug::StackFrame& Scene_Debug::GetFrame(int n) const {
	auto i = stack_index - n;
	assert(i >= 0 && i < static_cast<int>(stack.size()));
	return stack[i];
}

int Scene_Debug::GetStackSize() const {
	return stack_index + 1;
}

Window_VarList::Mode Scene_Debug::GetWindowMode() const {
	switch (mode) {
		case eSwitch:
			return Window_VarList::eSwitch;
		case eVariable:
			return Window_VarList::eVariable;
		case eItem:
			return Window_VarList::eItem;
		case eBattle:
			return Window_VarList::eTroop;
		case eMap:
			return Window_VarList::eMap;
		case eFullHeal:
			return Window_VarList::eHeal;
		case eLevel:
			return Window_VarList::eLevel;
		case eCallCommonEvent:
			return Window_VarList::eCommonEvent;
		case eCallMapEvent:
			return Window_VarList::eMapEvent;
		case eString:
			return Window_VarList::eString;
		default:
			return Window_VarList::eNone;
	}
}

void Scene_Debug::UpdateFrameValueFromUi() {
	auto& frame = GetFrame();
	auto& idx = prev[mode];
	switch (frame.uimode) {
		case eUiMain:
			idx.range_index = range_index;
			idx.range_page = range_page;
			break;
		case eUiRangeList:
			idx.range_index = range_index;
			idx.range_page = range_page;
			frame.value = GetSelectedIndexFromRange() - 1;
			break;
		case eUiVarList:
			idx.range_page_index = var_window->GetIndex();
			frame.value = GetSelectedIndexFromRange() + var_window->GetIndex();
			break;
		case eUiNumberInput:
			frame.value = numberinput_window->GetNumber();
			break;
		case eUiChoices:
			frame.value = choices_window->GetIndex();
			break;
		case eUiStringView:
			frame.value = stringview_window->GetIndex();
			break;
		case eUiInterpreterView:
			idx.range_page_index = interpreter_window->GetIndex();
			frame.value = GetSelectedIndexFromRange() + interpreter_window->GetIndex();
			state_interpreter.selected_frame = interpreter_window->GetSelectedStackFrameLine();
			break;
	}
}

void Scene_Debug::Push(UiMode ui) {
	++stack_index;
	assert(stack_index < static_cast<int>(stack.size()));
	stack[stack_index] = { ui, 0 };

	range_window->SetActive(false);
	var_window->SetActive(false);
	interpreter_window->SetActive(false);
	interpreter_window->SetVisible(false);
	numberinput_window->SetActive(false);
	numberinput_window->SetVisible(false);
	stringview_window->SetActive(false);
	stringview_window->SetVisible(false);
}

int Scene_Debug::GetSelectedIndexFromRange() const {
	switch (mode) {
		case eInterpreter:
			return range_page * 10 + range_index + 1;
		default:
			return range_page * 100 + range_index * 10 + 1;
	}
}

void Scene_Debug::RestoreRangeSelectionFromSelectedValue(int value) {
	switch (mode) {
		case eInterpreter:
			range_index = value % 10;
			range_page = value / 10;
			break;
		default:
			range_index = (value % 100) / 10;
			range_page = value / 100;
			break;
	}
}

void Scene_Debug::SetupUiRangeList() {
	auto& idx = prev[mode];
	auto vmode = GetWindowMode();

	range_index = idx.range_index;
	range_page = idx.range_page;

	//if (mode == eInterpreter && (state_interpreter.show_frame_switches || state_interpreter.show_frame_vars)) {
	//	var_window->SetMode(state_interpreter.show_frame_switches ? Window_VarList::eFrameSwitch : Window_VarList::eFrameVariable);
	//} else {
		var_window->SetMode(vmode, mode == eString ? strings.size() : 0);
	//}
	UpdateDetailWindow();

	range_window->SetIndex(range_index);
}

void Scene_Debug::PushUiRangeList() {
	Push(eUiRangeList);

	SetupUiRangeList();

	range_window->SetActive(true);

	UpdateRangeListWindow();
	RefreshDetailWindow();

	if (mode == eInterpreter) {
		interpreter_window->SetVisible(true);
	}
}

void Scene_Debug::PushUiVarList() {
	const bool was_range_list = (GetFrame().uimode == eUiRangeList);

	Push(eUiVarList);

	auto& idx = prev[mode];

	if (!was_range_list) {
		SetupUiRangeList();
	}

	var_window->SetActive(true);
	var_window->SetIndex(idx.range_page_index);

	UpdateRangeListWindow();
	var_window->Refresh();
}

void Scene_Debug::PushUiNumberInput(int init_value, int digits, bool show_operator) {
	Push(eUiNumberInput);

	numberinput_window->SetNumber(init_value);
	numberinput_window->SetShowOperator(show_operator);
	numberinput_window->SetVisible(true);
	numberinput_window->SetActive(true);
	numberinput_window->SetMaxDigits(digits);
	numberinput_window->Refresh();

	RefreshDetailWindow();
	UpdateRangeListWindow();
}

void Scene_Debug::PushUiChoices(std::vector<std::string> choices, std::vector<bool> choices_enabled) {
	Push(eUiChoices);

	choices_window->ReplaceCommands(choices);
	choices_window->SetY(Player::menu_offset_y + (choices.size() + 1) * 8 + 72);
	choices_window->SetHeight((choices.size() + 1) * 16);
	choices_window->SetVisible(true);
	choices_window->SetActive(true);
	choices_window->Refresh();

	for (int i = 0; i < static_cast<int>(choices_enabled.size()); i++) {
		choices_window->SetItemEnabled(i, choices_enabled[i]);
	}

	RefreshDetailWindow();
	UpdateRangeListWindow();
}

void Scene_Debug::PushUiStringView() {
	const auto str_id = GetFrame().value;

	auto value = ToString(Main_Data::game_strings->Get(str_id));

	if (value.empty()) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
		return;
	}
	Push(eUiStringView);

	var_window->SetActive(false);
	stringview_window->SetActive(true);
	stringview_window->SetVisible(true);

	Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
	stringview_window->SetDisplayData(value);
	stringview_window->SetIndex(0);
	stringview_window->Refresh();
}

void Scene_Debug::PushUiInterpreterView() {
	const bool was_range_list = (GetFrame().uimode == eUiRangeList);

	if (!interpreter_window->IsValid()) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
		return;
	}

	Push(eUiInterpreterView);

	if (!was_range_list) {
		SetupUiRangeList();
	}

	interpreter_window->SetVisible(true);
	var_window->SetVisible(false);

	interpreter_window->SetActive(true);
	interpreter_window->SetIndex(0);

	UpdateRangeListWindow();
	interpreter_window->Refresh();
}


void Scene_Debug::Pop() {
	range_window->SetActive(false);
	var_window->SetActive(false);
	numberinput_window->SetActive(false);
	numberinput_window->SetVisible(false);
	choices_window->SetActive(false);
	choices_window->SetVisible(false);
	stringview_window->SetActive(false);
	stringview_window->SetVisible(false);
	interpreter_window->SetActive(false);

	if (mode == eInterpreter /* && !(state_interpreter.show_frame_switches || state_interpreter.show_frame_vars) */) {
		interpreter_window->SetIndex(-1);
		interpreter_window->SetVisible(true);
		var_window->SetVisible(false);
	} else {
		var_window->SetIndex(-1);
		var_window->SetVisible(true);
		interpreter_window->SetVisible(false);
	}

	if (stack_index == 0) {
		Scene::Pop();
		return;
	}

	--stack_index;

	auto& frame = GetFrame();
	auto nui = frame.uimode;
	switch (nui) {
		case eUiMain:
			var_window->SetMode(Window_VarList::eNone);
			interpreter_window->SetVisible(false);
			range_index = (static_cast<int>(mode) - 1) % 10;
			range_page = (static_cast<int>(mode) - 1) / 10;
			range_window->SetActive(true);
			range_window->SetIndex(range_index);
			break;
		case eUiRangeList:
			range_window->SetActive(true);
			RestoreRangeSelectionFromSelectedValue(frame.value);
			range_window->SetIndex(range_index);
			break;
		case eUiVarList:
			var_window->SetActive(true);
			var_window->SetIndex((frame.value - 1) % 10);
			break;
		case eUiNumberInput:
			numberinput_window->SetNumber(frame.value);
			numberinput_window->SetActive(true);
			numberinput_window->SetVisible(true);
			break;
		case eUiChoices:
			choices_window->SetIndex(frame.value);
			choices_window->SetActive(true);
			choices_window->SetVisible(true);
			break;
		case eUiStringView:
			stringview_window->SetActive(true);
			stringview_window->SetVisible(true);
			break;
		case eUiInterpreterView:
			interpreter_window->SetActive(true);
			interpreter_window->SetIndex(frame.value - 1);
			var_window->SetVisible(false);
			interpreter_window->SetVisible(true);
			//state_interpreter.show_frame_switches = false;
			//state_interpreter.show_frame_vars = false;
			break;
	}

	if (stack_index == 0) {
		mode = eMain;
	}

	UpdateRangeListWindow();
	RefreshDetailWindow();
}

void Scene_Debug::vUpdate() {
	range_window->Update();
	if (range_index != range_window->GetIndex()){
		range_index = range_window->GetIndex();
		UpdateDetailWindow();
		RefreshDetailWindow();
	}
	var_window->Update();

	if (stringview_window->GetActive())
		stringview_window->Update();

	if (numberinput_window->GetActive())
		numberinput_window->Update();

	if (choices_window->GetActive())
		choices_window->Update();

	if (interpreter_window->GetActive())
		interpreter_window->Update();

	if (Input::IsTriggered(Input::CANCEL)) {
		UpdateFrameValueFromUi();
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
		Pop();

		const auto sz = GetStackSize();
		if ((mode == eInterpreter) && sz == 4) {
			Pop(); //skip over choices window
		}
	} else if (Input::IsTriggered(Input::DECISION)) {
		UpdateFrameValueFromUi();
		if (mode == eMain) {
			auto next_mode = static_cast<Mode>(range_window->GetIndex() + range_page * 10 + 1);
			if (next_mode > eMain && next_mode <= GetNumMainMenuItems()) {
				if (!range_window->IsItemEnabled(range_window->GetIndex())) {
					Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
				} else {
					Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
					mode = next_mode;
				}
			}
		}

		const auto sz = GetStackSize();
		const auto& frame = GetFrame();
		switch (mode) {
			case eMain:
			case eLastMainMenuOption:
				break;
			case eSave:
				Scene::PopUntil(Scene::Map);
				Scene::Push(std::make_shared<Scene_Save>());
				break;
			case eLoad:
				Scene::Push(std::make_shared<Scene_Load>());
				mode = eMain;
				break;
			case eSwitch:
				if (sz > 2) {
					DoSwitch();
				} else if (sz > 1) {
					PushUiVarList();
				} else if (sz > 0) {
					PushUiRangeList();
				}
				break;
			case eVariable:
				if (sz > 3) {
					DoVariable();
				} else if (sz > 2) {
					if (Main_Data::game_variables->IsValid(frame.value)) {
						PushUiNumberInput(Main_Data::game_variables->Get(frame.value), Main_Data::game_variables->GetMaxDigits(), true);
					}
				} else if (sz > 1) {
					PushUiVarList();
				} else {
					PushUiRangeList();
				}
				break;
			case eGold:
				if (sz > 1) {
					DoGold();
				} else {
					PushUiNumberInput(Main_Data::game_party->GetGold(), 6, false);
					range_index = 0;
					range_window->SetIndex(range_index);
				}
				break;
			case eItem:
				if (sz > 3) {
					DoItem();
				} else if (sz > 2) {
					if (frame.value <= static_cast<int>(lcf::Data::items.size())) {
						PushUiNumberInput(Main_Data::game_party->GetItemCount(frame.value), Main_Data::game_party->GetMaxItemCount(frame.value) >= 100 ? 3 : 2, false);
					}
				} else if (sz > 1) {
					PushUiVarList();
				} else {
					PushUiRangeList();
				}
				break;
			case eBattle:
				if (sz > 2) {
					DoBattle();
				} else if (sz > 1) {
					PushUiVarList();
				} else {
					PushUiRangeList();
				}
				break;
			case eMap:
				if (sz > 4) {
					DoMap();
				} else if (sz > 3) {
					// FIXME: Remember previous y
					PushUiNumberInput(GetFrame(-1).value, 4, false);
				} else if (sz > 2) {
					const auto map_id = GetFrame().value;
					if (IsValidMapId(map_id)) {
						// Reset x and y values
						GetFrame(-1).value = 0;
						GetFrame(-2).value = 0;
						PushUiNumberInput(GetFrame(-1).value, 4, false);
					}
				} else if (sz > 1) {
					PushUiVarList();
				} else {
					PushUiRangeList();
				}
				break;
			case eFullHeal:
				if (sz > 1) {
					DoFullHeal();
				} else {
					PushUiVarList();
				}
				break;
			case eLevel:
				if (sz > 2) {
					DoLevel();
				} else if (sz > 1) {
					if (frame.value <= static_cast<int>(Main_Data::game_party->GetActors().size())) {
						auto* actor = Main_Data::game_party->GetActors()[frame.value - 1];
						PushUiNumberInput(actor->GetLevel(), actor->GetMaxLevel() >= 100 ? 3 : 2, false);
					}
				} else {
					PushUiVarList();
				}
				break;
			case eMoveSpeed:
				if (sz > 1) {
					DoMoveSpeed();
				} else {
					PushUiNumberInput(Main_Data::game_player->GetMoveSpeed(), 1, false);
					range_index = 0;
					range_window->SetIndex(range_index);
				}
				break;
			case eCallCommonEvent:
				if (sz > 2) {
					DoCallCommonEvent();
				} else if (sz > 1) {
					PushUiVarList();
				} else {
					PushUiRangeList();
				}
				break;
			case eCallMapEvent:
				if (sz > 3) {
					DoCallMapEvent();
				} else if (sz > 2) {
					auto* event = Game_Map::GetEvent(GetFrame().value);
					if (event) {
						const auto num_digits = static_cast<int>(std::log10(event->GetNumPages()) + 1);
						PushUiNumberInput(1, num_digits, false);
					}
				} else if (sz > 1) {
					PushUiVarList();
				} else {
					PushUiRangeList();
				}
				break;
			case eCallBattleEvent:
				if (sz > 1) {
					DoCallBattleEvent();
				} else {
					auto* troop = Game_Battle::GetActiveTroop();
					if (troop) {
						const auto num_digits = static_cast<int>(std::log10(troop->pages.size()) + 1);
						PushUiNumberInput(0, num_digits, false);
					}
				}
				break;
			case eString:
				if (sz == 3) {
					PushUiStringView();
				} else if (sz == 2) {
					PushUiVarList();
				} else if (sz == 1) {
					if (!strings_cached) {
						strings = Main_Data::game_strings->GetLcfData();
						strings_cached = true;
					}
					PushUiRangeList();
				}
				break;
			case eInterpreter:
				if (sz == 3) {
					if (state_interpreter.selected_frame >= 0) {
						/*std::vector<std::string> choices;
						std::vector<bool> choices_enabled;

						choices.push_back("FrameSwitches");
						choices.push_back("FrameVariables");

						choices_enabled.push_back(GetSelectedInterpreterFrameFromUiState().easyrpg_frame_switches.size() > 0);
						choices_enabled.push_back(GetSelectedInterpreterFrameFromUiState().easyrpg_frame_variables.size() > 0);

						PushUiChoices(choices, choices_enabled);*/
					} else {
						Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
					}
				} else if (sz == 2) {
					PushUiInterpreterView();
				} else if (sz == 1) {
					if (!interpreter_states_cached) {
						CacheBackgroundInterpreterStates();
						interpreter_states_cached = true;
					}
					PushUiRangeList();
					interpreter_window->SetVisible(true);
					UpdateInterpreterWindow(GetSelectedIndexFromRange());
				}
				break;
			case eOpenMenu:
				DoOpenMenu();
				break;
		}
		Game_Map::SetNeedRefresh(true);
	} else if (range_window->GetActive() && Input::IsRepeated(Input::RIGHT)) {
		int range_page_prev = range_page;
		if (range_page < GetLastPage()) {
			++range_page;
		} else {
			range_page = 0;
		}
		if (range_page != range_page_prev) {
			UpdateDetailWindow();
			RefreshDetailWindow();
			UpdateRangeListWindow();
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
		}
	} else if (range_window->GetActive() && Input::IsRepeated(Input::LEFT)) {
		int range_page_prev = range_page;
		if (range_page > 0) {
			--range_page;
		} else {
			range_page = GetLastPage();
		}
		if (range_page != range_page_prev) {
			UpdateDetailWindow();
			RefreshDetailWindow();
			UpdateRangeListWindow();
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
		}
	}

	UpdateArrows();
}

void Scene_Debug::CreateRangeWindow() {

	std::vector<std::string> ranges;
	for (int i = 0; i < 10; i++)
		ranges.push_back("");
	range_window.reset(new Window_Command(ranges, 96));

	int height = 176;
	range_window->SetHeight(height);
	range_window->SetX(Player::menu_offset_x);
	range_window->SetY(Player::menu_offset_y + ((MENU_HEIGHT - height) / 2));
}

void Scene_Debug::UpdateRangeListWindow() {
	int idx = 0;
	const bool is_battle = Game_Battle::IsBattleRunning();

	auto addItem = [&](const auto& name, bool enabled = true) {
		range_window->SetItemText(idx, name);
		range_window->SetItemEnabled(idx, enabled);
		++idx;
	};

	auto fillRange = [&](const auto& prefix) {
		for (int i = 0; i < 10; i++){
			const auto st = range_page * 100 + i * 10 + 1;
			addItem(fmt::format("{}[{:04d}-{:04d}]", prefix, st, st + 9));
		}
	};

	switch (mode) {
		case eMain:
			if (range_page == 0) {
				addItem("Save", !is_battle);
				addItem("Load");
				addItem("Switches");
				addItem("Variables");
				addItem(lcf::Data::terms.gold.c_str());
				addItem("Items");
				addItem("Battle", !is_battle);
				addItem("Goto Map", !is_battle);
				addItem("Full Heal");
				addItem("Level");
			} else {
				addItem("Move Speed", !is_battle);
				addItem("Call ComEvent");
				addItem("Call MapEvent", Scene::Find(Scene::Map) != nullptr);
				addItem("Call BtlEvent", is_battle);
				addItem("Strings", Player::IsPatchManiac());
				addItem("Interpreter");
				addItem("Open Menu", !is_battle);
			}
			break;
		case eSwitch:
			fillRange("Sw");
			break;
		case eVariable:
			fillRange("Vr");
			break;
		case eItem:
			fillRange("It");
			break;
		case eBattle:
			fillRange("Bt");
			break;
		case eMap:
			if (GetStackSize() > 3) {
				if (GetStackSize() > 4) {
					addItem("Map: " + std::to_string(GetFrame(2).value));
					addItem("X: " + std::to_string(GetFrame(1).value));
					addItem("Y: ");
				} else {
					addItem("Map: " + std::to_string(GetFrame(1).value));
					addItem("X: ");
				}
			} else {
				fillRange("Mp");
			}
			break;
		case eCallCommonEvent:
			fillRange("Ce");
			break;
		case eCallMapEvent:
			if (GetStackSize() > 3) {
				auto* event = Game_Map::GetEvent(GetFrame(1).value);
				if (event) {
					addItem(fmt::format("{:04d}: {}", event->GetId(), event->GetName()));
					addItem(fmt::format("NumPages: {}", event->GetNumPages()));
					const auto* page = event->GetActivePage();
					const auto page_id = page ? page->ID : 0;
					addItem(fmt::format("ActvPage: {}", page_id));
					addItem(fmt::format("Enabled: {}", event->IsActive() ? 'Y' : 'N'));
					addItem(fmt::format("X: {}", event->GetX()));
					addItem(fmt::format("Y: {}", event->GetY()));
				}
			} else {
				fillRange("Me");
			}
			break;
		case eGold:
			addItem(lcf::Data::terms.gold);
			for (int i = 1; i < 10; i++){
				range_window->SetItemText(i, "");
			}
			break;
		case eFullHeal:
			addItem("Full Heal");
			break;
		case eLevel:
			addItem("Level");
			break;
		case eMoveSpeed:
			addItem("Move Speed");
			addItem("Range: 1-7");
			break;
		case eCallBattleEvent:
			if (is_battle) {
				auto* troop = Game_Battle::GetActiveTroop();
				if (troop) {
					addItem(troop->name);
					addItem(fmt::format("TroopId: {}", troop->ID));
					addItem(fmt::format("NumEnemies: {}", troop->members.size()));
					addItem(fmt::format("NumPages: {}", troop->pages.size()));
				}
			}
			break;
		case eString:
			fillRange("St");
			break;
		case eInterpreter:
		{
			//if (state_interpreter.show_frame_switches || state_interpreter.show_frame_vars) {
			//	for (int i = 0; i < 10; i++) {
			//		const auto st = range_page * 100 + i * 10 + 1;
			//		addItem(fmt::format("{}[{:03d}-{:03d}]", state_interpreter.show_frame_switches ? "FSw" : "FVr", st, st + 9));
			//	}
			//} else {
				int skip_items = range_page * 10;
				int count_items = 0;
				if (range_page == 0) {
					addItem(fmt::format("{}Main", Game_Interpreter::GetForegroundInterpreter().GetState().wait_movement ? "(W) " : ""));
					skip_items = 1;
					count_items = 1;
				}
				for (int i = 0; i < static_cast<int>(state_interpreter.ev.size()) && count_items < 10; i++) {
					if (skip_items > 0) {
						skip_items--;
						continue;
					}
					int evt_id = state_interpreter.ev[i];
					addItem(fmt::format("{}EV{:04d}: {}", state_interpreter.state_ev[i].wait_movement ? "(W) " : "", evt_id, Game_Map::GetEvent(evt_id)->GetName()));
					count_items++;
				}
				for (int i = 0; i < static_cast<int>(state_interpreter.ce.size()) && count_items < 10; i++) {
					if (skip_items > 0) {
						skip_items--;
						continue;
					}
					int ce_id = state_interpreter.ce[i];
					auto* ce = lcf::ReaderUtil::GetElement(lcf::Data::commonevents, ce_id);
					addItem(fmt::format("{}CE{:04d}: {}", state_interpreter.state_ce[i].wait_movement ? "(W) " : "", ce_id, ce->name));
					count_items++;
				}
			//}
		}
		break;
		default:
			break;
	}

	while (idx < 10) {
		addItem("", true);
	}
}

void Scene_Debug::UpdateDetailWindow() {
	if (mode == eInterpreter) {
		//if (state_interpreter.show_frame_switches || state_interpreter.show_frame_vars) {
		//	auto & interpreter_frame = GetSelectedInterpreterFrameFromUiState();
		//	var_window->SetInterpreterFrame(&interpreter_frame);
		//	var_window->UpdateList(GetSelectedIndexFromRange());
		//
		//	interpreter_window->SetVisible(false);
		//	interpreter_window->SetActive(false);
		//} else {
			UpdateInterpreterWindow(GetSelectedIndexFromRange());
		//}
	} else {
		var_window->UpdateList(GetSelectedIndexFromRange());
	}
}

void Scene_Debug::RefreshDetailWindow() {
	if (mode == eInterpreter) {
		//if (state_interpreter.show_frame_switches || state_interpreter.show_frame_vars) {
		//	var_window->Refresh();
		//} else {
			//var_window->SetInterpreterFrame(nullptr);
			interpreter_window->Refresh();
		//}
	} else {
		//var_window->SetInterpreterFrame(nullptr);
		var_window->Refresh();
	}
}

void Scene_Debug::CreateVarListWindow() {
	std::vector<std::string> vars;
	for (int i = 0; i < 10; i++)
		vars.push_back("");
	var_window.reset(new Window_VarList(vars));
	var_window->SetX(Player::menu_offset_x + range_window->GetWidth());
	var_window->SetY(range_window->GetY());
	var_window->SetVisible(false);
	var_window->SetIndex(-1);

	var_window->UpdateList(GetSelectedIndexFromRange());
}

void Scene_Debug::CreateNumberInputWindow() {
	numberinput_window.reset(new Window_NumberInput(Player::menu_offset_x + 160 - (Main_Data::game_variables->GetMaxDigits() + 1) * 6 - 8, Player::menu_offset_y + 104,
		(Main_Data::game_variables->GetMaxDigits() + 1) * 12 + 16, 32));
	numberinput_window->SetVisible(false);
	numberinput_window->SetOpacity(255);
	numberinput_window->SetShowOperator(true);
}

void Scene_Debug::CreateChoicesWindow() {
	std::vector<std::string> vars;
	for (int i = 0; i < 2; i++)
		vars.push_back("");
	choices_window.reset(new Window_Command(vars));
	choices_window->SetX(Player::menu_offset_x + 160 - choice_window_width / 2);
	choices_window->SetY(Player::menu_offset_y + 104);
	choices_window->SetWidth(choice_window_width);
	choices_window->SetHeight(64);
	choices_window->SetVisible(false);
	choices_window->SetOpacity(255);
}

void Scene_Debug::CreateStringViewWindow() {
	stringview_window.reset(new Window_StringView(Player::menu_offset_x + 15, Player::menu_offset_y + 16, 288, 208));
	stringview_window->SetVisible(false);
	stringview_window->SetIndex(-1);
}

void Scene_Debug::CreateInterpreterWindow() {
	interpreter_window.reset(new Window_Interpreter(Player::menu_offset_x + range_window->GetWidth(), range_window->GetY(), 224, 176));
	interpreter_window->SetVisible(false);
	interpreter_window->SetIndex(-1);
}

int Scene_Debug::GetNumMainMenuItems() const {
	return static_cast<int>(eLastMainMenuOption) - 1;
}

int Scene_Debug::GetLastPage() {
	size_t num_elements = 0;
	switch (mode) {
		case eMain:
			return GetNumMainMenuItems() / 10;
		case eSwitch:
			num_elements = Main_Data::game_switches->GetSizeWithLimit();
			break;
		case eVariable:
			num_elements = Main_Data::game_variables->GetSizeWithLimit();
			break;
		case eItem:
			num_elements = lcf::Data::items.size();
			break;
		case eBattle:
			num_elements = lcf::Data::troops.size();
			break;
		case eMap:
			num_elements = lcf::Data::treemap.maps.size() > 0 ? lcf::Data::treemap.maps.back().ID : 0;
			break;
		case eFullHeal:
			num_elements = Main_Data::game_party->GetBattlerCount() + 1;
			break;
		case eLevel:
			num_elements = Main_Data::game_party->GetBattlerCount();
			break;
		case eCallCommonEvent:
			num_elements = lcf::Data::commonevents.size();
			break;
		case eCallMapEvent:
			num_elements = Game_Map::GetHighestEventId();
			break;
		case eString:
			num_elements = strings.size();
			break;
		case eInterpreter:
			//if (state_interpreter.show_frame_switches) {
			//	auto & interpreter_frame = GetSelectedInterpreterFrameFromUiState();
			//	return interpreter_frame.easyrpg_frame_switches.size();
			//} else if (state_interpreter.show_frame_vars) {
			//		auto & interpreter_frame = GetSelectedInterpreterFrameFromUiState();
			//		return interpreter_frame.easyrpg_frame_variables.size();
			//} else {
				num_elements = 1 + state_interpreter.ev.size() + state_interpreter.ce.size();
				return (static_cast<int>(num_elements) - 1) / 10;
			//}
		default:
			break;
	}

	if (num_elements > 0) {
		return (static_cast<int>(num_elements) - 1) / 100;
	}
	return 0;
}

bool Scene_Debug::IsValidMapId(int map_id) const {
	auto iter = std::lower_bound(lcf::Data::treemap.maps.begin(), lcf::Data::treemap.maps.end(), map_id,
			[](const lcf::rpg::MapInfo& l, int r) { return l.ID < r; });
	return (iter != lcf::Data::treemap.maps.end()
			&& iter->ID == map_id
			&& iter->type == lcf::rpg::TreeMap::MapType_map);
}

void Scene_Debug::DoSwitch() {
	const auto sw_id = GetFrame().value;
	if (Main_Data::game_switches->IsValid(sw_id)) {
		Main_Data::game_switches->Flip(sw_id);
		Game_Map::SetNeedRefresh(true);

		RefreshDetailWindow();
	}
}

void Scene_Debug::DoVariable() {
	const auto var_id = GetFrame(1).value;
	const auto value = GetFrame(0).value;
	Main_Data::game_variables->Set(var_id, value);
	Game_Map::SetNeedRefresh(true);

	Pop();
}

void Scene_Debug::DoGold() {
	const auto delta = GetFrame().value - Main_Data::game_party->GetGold();
	Main_Data::game_party->GainGold(delta);

	Pop();
}

void Scene_Debug::DoItem() {
	const auto item_id = GetFrame(1).value;
	auto delta = GetFrame().value - Main_Data::game_party->GetItemCount(item_id);

	Main_Data::game_party->AddItem(item_id, delta);

	Game_Map::SetNeedRefresh(true);

	Pop();
}

void Scene_Debug::DoBattle() {
	auto troop_id = GetFrame(0).value;
	if (troop_id > static_cast<int>(lcf::Data::troops.size())) {
		return;
	}

	Scene::PopUntil(Scene::Map);
	if (!Scene::instance) {
		return;
	}

	BattleArgs args;
	args.troop_id = troop_id;
	args.first_strike = false;
	args.allow_escape = true;

	Output::Debug("Debug Scene starting battle {}.", troop_id);

	Game_Map::SetupBattle(args);

	Scene::Push(Scene_Battle::Create(std::move(args)));
}

void Scene_Debug::DoMap() {
	auto y = GetFrame(0).value;
	auto x = GetFrame(1).value;
	auto map_id = GetFrame(2).value;

	Scene::PopUntil(Scene::Map);
	if (Scene::instance) {
		Main_Data::game_player->ReserveTeleport(map_id, x, y, -1, TeleportTarget::eSkillTeleport);
	}
}

void Scene_Debug::DoFullHeal() {
	const auto id = GetFrame(0).value;

	Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_UseItem));
	auto actors = Main_Data::game_party->GetActors();
	if (id <= 1) {
		for (auto& actor: actors) {
			actor->FullHeal();
		}
	} else {
		int idx = id - 2;
		if (idx < static_cast<int>(actors.size())) {
			actors[idx]->FullHeal();
		}
	}
	var_window->UpdateList(1);
	var_window->Refresh();
}

void Scene_Debug::DoLevel() {
	const auto id = GetFrame(1).value;
	const auto level = GetFrame(0).value;

	auto actors = Main_Data::game_party->GetActors();
	int idx = id - 1;
	if (idx < static_cast<int>(actors.size())) {
		if (actors[idx]->GetLevel() != level) {
			actors[idx]->ChangeLevel(level, nullptr);
		}
	}

	Pop();
}

void Scene_Debug::DoMoveSpeed() {
	Main_Data::game_player->SetMoveSpeed(Utils::Clamp<int>(GetFrame().value, 1, 7));

	Pop();
}

void Scene_Debug::DoCallCommonEvent() {
	const auto ceid = GetFrame(0).value;

	if (ceid > static_cast<int>(lcf::Data::commonevents.size())) {
		return;
	}

	auto& ce = Game_Map::GetCommonEvents()[ceid - 1];

	if (Game_Battle::IsBattleRunning()) {
		Game_Battle::GetInterpreter().Push(&ce);
		Scene::PopUntil(Scene::Battle);
		Output::Debug("Debug Scene Forced execution of common event {} on the battle foreground interpreter.", ce.GetIndex());
	} else {
		Game_Map::GetInterpreter().Push(&ce);
		Scene::PopUntil(Scene::Map);
		Output::Debug("Debug Scene Forced execution of common event {} on the map foreground interpreter.", ce.GetIndex());
	}
}

void Scene_Debug::DoCallMapEvent() {
	if (!Scene::Find(Scene::Map)) {
		return;
	}

	const auto me_id = GetFrame(1).value;
	const auto page_id = GetFrame(0).value;

	auto* me = Game_Map::GetEvent(me_id);
	if (!me) {
		return;
	}

	auto* page = me->GetPage(page_id);
	if (!page) {
		return;
	}

	if (Game_Battle::IsBattleRunning()) {
		Game_Battle::GetInterpreter().Push(me, page, false);
		Scene::PopUntil(Scene::Battle);
		Output::Debug("Debug Scene Forced execution of map event {} page {} on the battle foreground interpreter.", me->GetId(), page->ID);
	} else {
		Game_Map::GetInterpreter().Push(me, page, false);
		Scene::PopUntil(Scene::Map);
		Output::Debug("Debug Scene Forced execution of map event {} page {} on the map foreground interpreter.", me->GetId(), page->ID);
	}
}

void Scene_Debug::DoCallBattleEvent() {
	if (!Game_Battle::IsBattleRunning()) {
		return;
	}

	auto* troop = Game_Battle::GetActiveTroop();
	if (!troop) {
		return;
	}

	const auto page_idx = GetFrame(0).value - 1;

	if (page_idx < 0 || page_idx >= static_cast<int>(troop->pages.size())) {
		return;
	}

	auto& page = troop->pages[page_idx];

	Game_Battle::GetInterpreter().Push(page.event_commands, 0, false);
	Scene::PopUntil(Scene::Battle);
	Output::Debug("Debug Scene Forced execution of battle troop {} event page {} on the map foreground interpreter.", troop->ID, page.ID);
}

void Scene_Debug::DoOpenMenu() {
	if (Scene::Find(Scene::Menu)) {
		Scene::PopUntil(Scene::Menu);
	} else {
		Scene::Push(std::make_shared<Scene_Menu>(), true);
	}
}

void Scene_Debug::TransitionIn(SceneType /* prev_scene */) {
	Transition::instance().InitShow(Transition::TransitionCutIn, this);
}

void Scene_Debug::TransitionOut(SceneType /* next_scene */) {
	Transition::instance().InitErase(Transition::TransitionCutOut, this);
}

void Scene_Debug::UpdateArrows() {
	bool show_left_arrow = (range_page > 0);
	bool show_right_arrow = (range_page < GetLastPage());

	if (show_left_arrow || show_right_arrow) {
		arrow_frame = (arrow_frame + 1) % (arrow_animation_frames * 2);
	}
	bool arrow_visible = (arrow_frame < arrow_animation_frames);
	range_window->SetLeftArrow(show_left_arrow && arrow_visible);
	range_window->SetRightArrow(show_right_arrow && arrow_visible);
}

void Scene_Debug::CacheBackgroundInterpreterStates() {
	state_interpreter.ev.clear();
	state_interpreter.ce.clear();
	state_interpreter.state_ev.clear();
	state_interpreter.state_ce.clear();

	if (Game_Map::GetMapId() > 0) {
		for (auto& ev : Game_Map::GetEvents()) {
			if (ev.GetTrigger() != lcf::rpg::EventPage::Trigger_parallel || !ev.interpreter)
				continue;
			state_interpreter.ev.emplace_back(ev.GetId());
			state_interpreter.state_ev.emplace_back(ev.interpreter->GetState());
		}
		for (auto& ce : Game_Map::GetCommonEvents()) {
			if (ce.IsWaitingBackgroundExecution(false)) {
				state_interpreter.ce.emplace_back(ce.common_event_id);
				state_interpreter.state_ce.emplace_back(ce.interpreter->GetState());
			}
		}
	} else if (Game_Battle::IsBattleRunning() && Player::IsPatchManiac()) {
		//Not implemented: battle common events
	}
}

void Scene_Debug::UpdateInterpreterWindow(int index) {
	lcf::rpg::SaveEventExecState state;
	std::string first_line = "";
	bool valid = false;
	int evt_id = 0;

	if (index == 1) {
		state = Game_Interpreter::GetForegroundInterpreter().GetState();
		first_line = Game_Battle::IsBattleRunning() ? "Foreground (Battle)" : "Foreground (Map)";
		valid = true;
	} else if (index <= static_cast<int>(state_interpreter.ev.size())) {
		evt_id = state_interpreter.ev[index - 1];
		state = state_interpreter.state_ev[index - 1];
		first_line = fmt::format("EV{:04d}: {}", evt_id, Game_Map::GetEvent(evt_id)->GetName());
		valid = true;
	} else if ((index - state_interpreter.ev.size()) <= state_interpreter.ce.size()) {
		int ce_id = state_interpreter.ce[index - state_interpreter.ev.size() - 1];
		state = state_interpreter.state_ce[index - state_interpreter.ev.size() - 1];
		for (auto& ce : Game_Map::GetCommonEvents()) {
			if (ce.common_event_id == ce_id) {
				first_line = fmt::format("CE{:04d}: {}", ce_id, ce.GetName());
				evt_id = ce_id;
				valid = true;
				break;
			}
		}
	}

	if (valid) {
		state_interpreter.selected_state = index;
		interpreter_window->SetStackState(index > static_cast<int>(state_interpreter.ev.size()), evt_id, first_line, state);
	} else {
		state_interpreter.selected_state = -1;
		interpreter_window->SetStackState(0, 0, "", {});
	}
}

lcf::rpg::SaveEventExecFrame& Scene_Debug::GetSelectedInterpreterFrameFromUiState() const {
	static lcf::rpg::SaveEventExecFrame empty;

	if (state_interpreter.selected_state <= 0 || state_interpreter.selected_frame < 0) {
		return empty;
	}

	int index = state_interpreter.selected_state;

	if (index == 1) {
		auto& state = Game_Interpreter::GetForegroundInterpreter()._state;
		return state.stack[state_interpreter.selected_frame];
	} else if (index <= static_cast<int>(state_interpreter.ev.size())) {
		int evt_id = state_interpreter.ev[index - 1];
		auto ev = Game_Map::GetEvent(evt_id);

		auto& state = ev->interpreter->_state;
		return state.stack[state_interpreter.selected_frame];
	} else if ((index - state_interpreter.ev.size()) <= state_interpreter.ce.size()) {
		int ce_id = state_interpreter.ce[index - state_interpreter.ev.size() - 1];
		for (auto& ce : Game_Map::GetCommonEvents()) {
			if (ce.common_event_id == ce_id) {
				auto& state = ce.interpreter->_state;
				return state.stack[state_interpreter.selected_frame];
			}
		}
	}
	return empty;
}
