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
#include "input.h"
#include "game_variables.h"
#include "game_switches.h"
#include "game_system.h"
#include "scene_debug.h"

Scene_Debug::Scene_Debug() {
	Scene::type = Scene::Debug;
}

void Scene_Debug::Start() {
	current_var_type = TypeSwitch;
	range_index = 0;
	CreateRangeWindow();
	CreateVarListWindow();

	range_window->SetActive(true);
	var_window->SetActive(false);
	var_window->Refresh();
}

void Scene_Debug::Update() {
	range_window->Update();
	if (range_index != range_window->GetIndex()){
		var_window->UpdateList(range_window->GetIndex());
		range_index = range_window->GetIndex();
		var_window->Refresh();
	}
	var_window->Update();

	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Main_Data::game_data.system.cancel_se);
		if (range_window->GetActive())
			Scene::Pop();
		else {
			var_window->SetActive(false);
			range_window->SetActive(true);
			var_window->Refresh();
		}
	} else if (Input::IsTriggered(Input::DECISION)) {
		var_window->Refresh();
		if (range_window->GetActive()){
			range_window->SetActive(false);
			var_window->SetActive(true);
		}
	} else if (range_window->GetActive() && (Input::IsTriggered(Input::LEFT) || Input::IsTriggered(Input::RIGHT))) {
		var_window->Refresh();
		if (current_var_type == TypeSwitch) {
			current_var_type = TypeInt;
			var_window->SetShowSwitch(false);
		} else{
			current_var_type = TypeSwitch;
			var_window->SetShowSwitch(true);
		}
		UpdateRangeListWindow();
	}
}

void Scene_Debug::CreateRangeWindow() {
	
	std::vector<std::string> ranges;
	int range_size = Game_Switches.size() / 10;
	for (int i = 0; i < Game_Switches.size() / 10; i++)
		ranges.push_back("");
	range_window.reset(new Window_Command(ranges, 96));

	range_window->SetHeight(176);
	range_window->SetY(32);
	UpdateRangeListWindow();
}
	
void Scene_Debug::UpdateRangeListWindow() {
	std::stringstream ss;
	int size = current_var_type == TypeSwitch ? Game_Switches.size() : Game_Variables.size();
	for (int i = 0; i <= size / 10; i++){
		ss.str("");
		ss  << ((current_var_type == TypeSwitch) ? "Sw[" : "Vr[")
			<< std::setfill('0') << std::setw(4) << (i * 10 + 1)
			<< "-"
			<< std::setw(4) << (1, i*10+10 < Game_Variables.size() ? i*10+10 : Game_Variables.size()) << "]";
		range_window->SetItemText(i, ss.str());
		if (i*10+10 >= Game_Variables.size())
			break;
	}
}

void Scene_Debug::CreateVarListWindow() {
	
	std::vector<std::string> vars;
	for (int i = 0; i < 10; i++)
		vars.push_back("");
	var_window.reset(new Window_VarList(vars));
	var_window->SetX(range_window->GetWidth());
	var_window->SetY(range_window->GetY());
	var_window->UpdateList(range_window->GetIndex());
}
