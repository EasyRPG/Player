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
#include <algorithm>
#include <unordered_map>
#include <lcf/inireader.h>

#include "dynrpg_keyboard_mouse_input.h"
#include "filefinder.h"
#include "game_dynrpg.h"
#include "game_map.h"
#include "game_runtime_patches.h"
#include "game_switches.h"
#include "game_variables.h"
#include "input.h"
#include "main_data.h"

DynRpg::KeyboardMouseInput::KeyboardMouseInput(Game_DynRpg& instance) :
	DynRpgPlugin("keyboard_mouse_input", instance)
{
	LoadConfig();
}

void DynRpg::KeyboardMouseInput::RebuildUniqueSwitches() {
	active_bindings.clear();
	unique_switches.clear();

	for (int i = 0; i < 256; ++i) {
		int sw_id = key_switches[i];
		if (sw_id > 0) {
			active_bindings.emplace_back(i, sw_id);
			unique_switches.push_back(sw_id);
		}
	}

	std::sort(unique_switches.begin(), unique_switches.end());
	unique_switches.erase(std::unique(unique_switches.begin(), unique_switches.end()), unique_switches.end());
}

void DynRpg::KeyboardMouseInput::LoadConfig() {
	key_switches.fill(0);
	mouse_x_var = 0;
	mouse_y_var = 0;

	std::string ini_file = FileFinder::Game().FindFile("DynRPG.ini");
	if (ini_file.empty()) {
		return;
	}

	auto ini_stream = FileFinder::Game().OpenInputStream(ini_file, std::ios_base::in);
	if (!ini_stream) {
		return;
	}

	lcf::INIReader ini(ini_stream);
	if (ini.ParseError() == -1) {
		return;
	}

	const std::string section = "keyboard_mouse_input";

	for (int i = 0; i < 256; ++i) {
		std::string key_name = fmt::format("Key{}Switch", i);
		int switch_id = ini.GetInteger(section, key_name, 0);
		if (switch_id > 0) {
			key_switches[i] = switch_id;
		}
	}

	RebuildUniqueSwitches();

	mouse_x_var = std::max<int>(0, ini.GetInteger(section, "MouseXVar", 0));
	mouse_y_var = std::max<int>(0, ini.GetInteger(section, "MouseYVar", 0));
}

bool DynRpg::KeyboardMouseInput::IsVirtualKeyPressed(int key_id) const {
	if (key_id <= 0 || key_id >= 256) {
		return false;
	}

	Input::Keys::InputKey key = Input::Keys::NONE;
	switch (key_id) {
		case 1:
			key = Input::Keys::MOUSE_LEFT;
			break;
		case 2:
			key = Input::Keys::MOUSE_RIGHT;
			break;
		case 4:
			key = Input::Keys::MOUSE_MIDDLE;
			break;
		default:
			key = RuntimePatches::VirtualKeys::VirtualKeyToInputKey(key_id);
			break;
	}

	if (key == Input::Keys::NONE) {
		return false;
	}

	return Input::IsRawKeyPressed(key);
}

void DynRpg::KeyboardMouseInput::Update() {
	if (!unique_switches.empty()) {
		std::unordered_map<int, bool> desired_states;
		desired_states.reserve(unique_switches.size());

		for (int sw_id : unique_switches) {
			desired_states[sw_id] = false;
		}

		for (const auto& [vk, sw_id] : active_bindings) {
			if (IsVirtualKeyPressed(vk)) {
				desired_states[sw_id] = true;
			}
		}

		for (int sw_id : unique_switches) {
			bool next_val = desired_states[sw_id];
			if (Main_Data::game_switches->Get(sw_id) != next_val) {
				Main_Data::game_switches->Set(sw_id, next_val);
				Game_Map::SetNeedRefreshForSwitchChange(sw_id);
			}
		}
	}

	if (mouse_x_var > 0 || mouse_y_var > 0) {
		Point mouse_pos = Input::GetMousePosition();

		if (mouse_x_var > 0 && Main_Data::game_variables->Get(mouse_x_var) != mouse_pos.x) {
			Main_Data::game_variables->Set(mouse_x_var, mouse_pos.x);
			Game_Map::SetNeedRefreshForVarChange(mouse_x_var);
		}

		if (mouse_y_var > 0 && Main_Data::game_variables->Get(mouse_y_var) != mouse_pos.y) {
			Main_Data::game_variables->Set(mouse_y_var, mouse_pos.y);
			Game_Map::SetNeedRefreshForVarChange(mouse_y_var);
		}
	}
}

bool DynRpg::KeyboardMouseInput::CheckKey(dyn_arg_list args) {
	auto func = "checkkey";
	bool okay = false;
	int key_id, switch_id;

	std::tie(key_id, switch_id) = DynRpg::ParseArgs<int, int>(func, args, &okay);
	if (!okay) {
		return true;
	}

	if (switch_id > 0) {
		bool pressed = IsVirtualKeyPressed(key_id);
		if (Main_Data::game_switches->Get(switch_id) != pressed) {
			Main_Data::game_switches->Set(switch_id, pressed);
			Game_Map::SetNeedRefreshForSwitchChange(switch_id);
		}
	}

	return true;
}

bool DynRpg::KeyboardMouseInput::CheckAllKeys(dyn_arg_list args) {
	auto func = "checkallkeys";
	bool okay = false;
	int variable_id;

	std::tie(variable_id) = DynRpg::ParseArgs<int>(func, args, &okay);
	if (!okay) {
		return true;
	}

	if (variable_id > 0) {
		int pressed_key = 0;
		for (int i = 8; i < 256; ++i) {
			if (IsVirtualKeyPressed(i)) {
				pressed_key = i;
				break;
			}
		}

		if (Main_Data::game_variables->Get(variable_id) != pressed_key) {
			Main_Data::game_variables->Set(variable_id, pressed_key);
			Game_Map::SetNeedRefreshForVarChange(variable_id);
		}
	}

	return true;
}

bool DynRpg::KeyboardMouseInput::SetKeySwitch(dyn_arg_list args) {
	auto func = "setkeyswitch";
	bool okay = false;
	int key_id, switch_id;

	std::tie(key_id, switch_id) = DynRpg::ParseArgs<int, int>(func, args, &okay);
	if (!okay) {
		return true;
	}

	if (key_id >= 0 && key_id < 256) {
		key_switches[key_id] = std::max(0, switch_id);
		RebuildUniqueSwitches();
	}

	return true;
}

bool DynRpg::KeyboardMouseInput::SetMouseVar(dyn_arg_list args) {
	auto func = "setmousevar";
	bool okay = false;
	int x_var, y_var;

	std::tie(x_var, y_var) = DynRpg::ParseArgs<int, int>(func, args, &okay);
	if (!okay) {
		return true;
	}

	mouse_x_var = std::max(0, x_var);
	mouse_y_var = std::max(0, y_var);

	return true;
}

bool DynRpg::KeyboardMouseInput::Invoke(std::string_view func, dyn_arg_list args, bool&, Game_Interpreter*) {
	if (func == "checkkey") {
		return CheckKey(args);
	} else if (func == "checkallkeys") {
		return CheckAllKeys(args);
	} else if (func == "setkeyswitch") {
		return SetKeySwitch(args);
	} else if (func == "setmousevar") {
		return SetMouseVar(args);
	}

	return false;
}
