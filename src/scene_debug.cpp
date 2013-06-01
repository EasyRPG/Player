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
#include "input.h"
#include "game_variables.h"
#include "game_system.h"
#include "scene_debug.h"

Scene_Debug::Scene_Debug() {
	Scene::type = Scene::Debug;
}

void Scene_Debug::Start() {
	current_var_type = TypeInt;
	CreateRangeWindow();

	range_window->SetActive(true);
	
}

void Scene_Debug::Update() {
	range_window->Update();
	var_window->Update();
	var_index = range_window->GetIndex() * 10 + var_window->GetIndex();

	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Main_Data::game_data.system.cancel_se);
		if (range_window->GetActive())
			Scene::Pop();
		else {
			var_window->SetActive(false);
			range_window->SetActive(true);
		}
	} else if (Input::IsTriggered(Input::DECISION)) {
		if (range_window->GetActive()){
			range_window->SetActive(false);
			range_window->SetActive(true);
		}
	}
}

void Scene_Debug::CreateRangeWindow() {
	
	std::vector<std::string> ranges;
	for (int i = 0; i <= Game_Variables.size() / 10; i++)
		ranges.push_back(std::string("I [ %4d-%4d ]", i * 10 + 1, i*10+11 < Game_Variables.size() ? i*10+11 : Game_Variables.size()));
	range_window.reset(new Window_Command(ranges));

	range_window->SetWidth(96);
	range_window->SetHeight(176);
	range_window->SetY(32);
}

void Scene_Debug::CreateVarListWindow() {
	
	std::vector<std::string> vars;
	for (int i = range_index; i <= Game_Variables.size() / 10; i++)
		vars.push_back(std::string("%4d:", i));

	var_window.reset(new Window_Command(vars));

	var_window->SetWidth(224);
	var_window->SetHeight(176);
	var_window->SetX(96);
	var_window->SetY(32);
}
