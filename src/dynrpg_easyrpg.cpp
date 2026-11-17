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
#include <map>

#include "dynrpg_easyrpg.h"
#include "main_data.h"
#include "game_variables.h"
#include "utils.h"
#include "version.h"

static bool EasyOput(dyn_arg_list args) {
	auto func = "output";
	bool okay = false;
	std::string mode;
	std::tie(mode, std::ignore) = DynRpg::ParseArgs<std::string, std::string>(func, args, &okay);
	if (!okay)
		return true;
	mode = Utils::LowerCase(mode);

	auto msg = DynRpg::ParseVarArg(func, args, 1, okay);

	if (mode == "debug") {
		Output::DebugStr(msg);
	} else if (mode == "info") {
		Output::InfoStr(msg);
	} else if (mode == "warning") {
		Output::WarningStr(msg);
	} else if (mode == "error") {
		Output::ErrorStr(msg);
	}

	return true;
}

bool DynRpg::EasyRpgPlugin::EasyCall(dyn_arg_list args, bool& do_yield, Game_Interpreter* interpreter) {
	auto func_name = std::get<0>(DynRpg::ParseArgs<std::string>("call", args));

	if (func_name.empty()) {
		// empty function name
		Output::Warning("call: Empty RPGSS function name");

		return true;
	}

	for (auto& plugin: instance.plugins) {
		if (plugin->Invoke(func_name, args.subspan(1), do_yield, interpreter)) {
			return true;
		}
	}

	return false;
}

static bool EasyAdd(dyn_arg_list args) {
	auto func = "easyrpg_add";
	bool okay = false;

	int target_var;
	int val;
	std::tie(target_var, val) = DynRpg::ParseArgs<int, int>(func, args, &okay);
	if (!okay)
		return true;

	for (size_t i = 2; i < args.size(); ++i) {
		val += std::get<0>(DynRpg::ParseArgs<int>(func, args.subspan(i), &okay));
		if (!okay)
			return true;
	}

	Main_Data::game_variables->Set(target_var, val);

	return true;
}

// Helper to convert collision strings to our enum/int values
int ParseCollisionBias(std::string_view bias_str, bool is_x_axis) {
	std::string s = Utils::LowerCase(ToString(bias_str));

	// Check if it's a numeric string
	if (s == "0" || s == "center") return 0;

	if (is_x_axis) {
		if (s == "1" || s == "left") return 1;
		if (s == "2" || s == "right") return 2;
	} else {
		if (s == "1" || s == "up") return 1;
		if (s == "2" || s == "down") return 2;
	}
	return 0; // Default to center
}

static bool EasySetCollision(dyn_arg_list args, Game_Interpreter* interpreter) {
	const char* func = "easyrpg_set_collision";

	// We parse all arguments as strings first to handle the interchangeability
	if (args.size() < 5) return true;

	int event_id = atoi(args[0].c_str());
	int width    = atoi(args[1].c_str());
	int height   = atoi(args[2].c_str());

	int x_bias = ParseCollisionBias(args[3], true);
	int y_bias = ParseCollisionBias(args[4], false);

	Game_Character* chara = interpreter->GetCharacter(event_id, func);
	if (chara) {
		chara->SetCustomHitbox(width, height, x_bias, y_bias);
	}

	return true;
}

static bool EasyResetCollision(dyn_arg_list args, Game_Interpreter* interpreter) {
	int event_id = atoi(args[0].c_str());
	Game_Character* chara = interpreter->GetCharacter(event_id, "easyrpg_reset_collision");
	if (chara) {
		// Reset the hitbox values to 0, causing GetTileOffsets
		// to fall through to the filename tag parser next time it's called.
		chara->SetCustomHitbox(0, 0, 0, 0);
	}
	return true;
}

static bool EasyShowCollision(dyn_arg_list args, Game_Interpreter* interpreter) {
	int event_id = atoi(args[0].c_str());
	Game_Character* chara = interpreter->GetCharacter(event_id, "easyrpg_show_collision");
	if (chara) {
		chara->SetCollisionDebugEnabled(true);
	}
	return true;
}

static bool EasyHideCollision(dyn_arg_list args, Game_Interpreter* interpreter) {
	int event_id = atoi(args[0].c_str());
	Game_Character* chara = interpreter->GetCharacter(event_id, "easyrpg_hide_collision");
	if (chara) {
		chara->SetCollisionDebugEnabled(false);
	}
	return true;
}

bool DynRpg::EasyRpgPlugin::Invoke(std::string_view func, dyn_arg_list args, bool& do_yield, Game_Interpreter* interpreter) {
	if (func == "call") {
		return EasyCall(args, do_yield, interpreter);
	} else if (func == "easyrpg_output") {
		return EasyOput(args);
	} else if (func == "easyrpg_add") {
		return EasyAdd(args);
	} else if (func == "easyrpg_set_collision") {
		return EasySetCollision(args, interpreter);
	} else if (func == "easyrpg_reset_collision") {
		return EasyResetCollision(args, interpreter);
	} else if (func == "easyrpg_show_collision") {
		return EasyShowCollision(args, interpreter);
	} else if (func == "easyrpg_hide_collision") {
		return EasyHideCollision(args, interpreter);
	}
	return false;
}

void DynRpg::EasyRpgPlugin::Load(const std::vector<uint8_t>& buffer) {
	if (buffer.size() < 4) {
		Output::Warning("EasyRpgPlugin: Bad savegame data");
	} else {
		uint32_t ver;
		memcpy(&ver, buffer.data(), 4);
		Utils::SwapByteOrder(ver);
		Output::Debug("DynRpg Savegame version {}", ver);
	}
}

std::vector<uint8_t> DynRpg::EasyRpgPlugin::Save() {
	std::vector<uint8_t> save_data;
	save_data.resize(4);

	uint32_t version = PLAYER_SAVEGAME_VERSION;
	Utils::SwapByteOrder(version);
	memcpy(&save_data[0], reinterpret_cast<char*>(&version), 4);

	return save_data;
}
