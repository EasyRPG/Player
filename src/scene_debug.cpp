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
#include <iomanip>
#include "baseui.h"
#include "cache.h"
#include "input.h"
#include "game_variables.h"
#include "game_switches.h"
#include "game_map.h"
#include "game_system.h"
#include "scene_debug.h"
#include "player.h"
#include "window_command.h"
#include "window_varlist.h"
#include "window_numberinput.h"
#include "bitmap.h"

Scene_Debug::Scene_Debug() {
	Scene::type = Scene::Debug;
}

void Scene_Debug::Start() {
	DisplayUi->SetBackcolor(Cache::System()->GetBackgroundColor());

	current_var_type = TypeSwitch;
	range_index = 0;
	range_page = 0;
	CreateRangeWindow();
	CreateVarListWindow();
	CreateNumberInputWindow();

	range_window->SetActive(true);
	var_window->SetActive(false);
	var_window->Refresh();
}

void Scene_Debug::Update() {
	range_window->Update();
	if (range_index != range_window->GetIndex()){
		range_index = range_window->GetIndex();
		var_window->UpdateList(range_page * 100 + range_index * 10 + 1);
		var_window->Refresh();
	}
	var_window->Update();

	if (numberinput_window->GetActive())
		numberinput_window->Update();

	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Main_Data::game_data.system.cancel_se);
		if (range_window->GetActive())
			Scene::Pop();
		else if (var_window->GetActive()) {
			var_window->SetActive(false);
			range_window->SetActive(true);
			var_window->Refresh();
		} else if (numberinput_window->GetActive()) {
			numberinput_window->SetVisible(false);
			numberinput_window->SetActive(false);
			var_window->SetActive(true);
		}
	} else if (Input::IsTriggered(Input::DECISION)) {
		var_window->Refresh();
		if (range_window->GetActive()) {
			range_window->SetActive(false);
			var_window->SetActive(true);
		} else if (var_window->GetActive()) {
			if (current_var_type == TypeSwitch && Game_Switches.isValidSwitch(GetIndex()))
				Game_Switches[GetIndex()] = !Game_Switches[GetIndex()];
			else if (current_var_type == TypeInt && Game_Variables.isValidVar(GetIndex())) {
				var_window->SetActive(false);
				numberinput_window->SetNumber(Game_Variables[GetIndex()]);
				numberinput_window->SetVisible(true);
				numberinput_window->SetActive(true);
			}
			var_window->Refresh();
		} else if (numberinput_window->GetActive()) {
			Game_Variables[GetIndex()] = numberinput_window->GetNumber();
			numberinput_window->SetActive(false);
			numberinput_window->SetVisible(false);
			var_window->SetActive(true);
			var_window->Refresh();
		}
		Game_Map::SetNeedRefresh(true);
	} else if (range_window->GetActive() &&  Input::IsTriggered(Input::RIGHT)) {
		range_page++;
		if (current_var_type == TypeSwitch && !Game_Switches.isValidSwitch(range_page*100+1)) {
			range_page = 0;
			current_var_type = TypeInt;
			var_window->SetShowSwitch(false);
		} else if (current_var_type == TypeInt && !Game_Variables.isValidVar(range_page*100+1)) {
			range_page = 0;
			current_var_type = TypeSwitch;
			var_window->SetShowSwitch(true);
		}
		var_window->UpdateList(range_page * 100 + range_index * 10 + 1);
		UpdateRangeListWindow();
		var_window->Refresh();
	} else if (range_window->GetActive() && Input::IsTriggered(Input::LEFT)) {
		range_page--;
		if (current_var_type == TypeSwitch && range_page < 0) {
			range_page = 0;
			for (;;)
				if (Game_Variables.isValidVar(range_page*100 + 101))
					range_page++;
				else
					break;
			current_var_type = TypeInt;
			var_window->SetShowSwitch(false);
		} else if (current_var_type == TypeInt && range_page < 0) {
			range_page = 0;
			for (;;)
				if (Game_Switches.isValidSwitch(range_page*100 + 101))
					range_page++;
				else
					break;
			current_var_type = TypeSwitch;
			var_window->SetShowSwitch(true);
		}
		var_window->UpdateList(range_page * 100 + range_index * 10 + 1);
		UpdateRangeListWindow();
		var_window->Refresh();
	}
}

void Scene_Debug::CreateRangeWindow() {
	
	std::vector<std::string> ranges;
	for (int i = 0; i < 10; i++)
		ranges.push_back("");
	range_window.reset(new Window_Command(ranges, 96));

	range_window->SetHeight(176);
	range_window->SetY(32);
	UpdateRangeListWindow();
}
	
void Scene_Debug::UpdateRangeListWindow() {
	std::stringstream ss;
	for (int i = 0; i < 10; i++){
		ss.str("");
		ss  << ((current_var_type == TypeSwitch) ? "Sw[" : "Vr[")
			<< std::setfill('0')
			<< std::setw(4)
			<< (range_page * 100 + i * 10 + 1)
			<< "-"
			<< std::setw(4)
			<< (range_page * 100 + i * 10 + 10) <<
			"]";
		range_window->SetItemText(i, ss.str());
	}
}

void Scene_Debug::CreateVarListWindow() {
	
	std::vector<std::string> vars;
	for (int i = 0; i < 10; i++)
		vars.push_back("");
	var_window.reset(new Window_VarList(vars));
	var_window->SetX(range_window->GetWidth());
	var_window->SetY(range_window->GetY());
	var_window->UpdateList(range_page * 100 + range_index * 10 + 1);
}

void Scene_Debug::CreateNumberInputWindow() {
	numberinput_window.reset(new Window_NumberInput(105, 104,
		Player::engine == Player::EngineRpg2k ? 12*7 + 16 : 12*8 + 16, 32));
	numberinput_window->SetVisible(false);
	numberinput_window->SetOpacity(255);
	numberinput_window->SetShowOperator(true);
}

int Scene_Debug::GetIndex() {
	return (range_page * 100 + range_index * 10 + var_window->GetIndex() + 1);
}

