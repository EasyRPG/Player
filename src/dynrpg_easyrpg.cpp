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
#include "string_view.h"
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

bool DynRpg::EasyRpgPlugin::EasyCall(Game_DynRpg& dynrpg_instance, dyn_arg_list args, bool& do_yield, Game_Interpreter* interpreter) {
	auto func_name = std::get<0>(DynRpg::ParseArgs<std::string>("call", args));

	if (func_name.empty()) {
		// empty function name
		Output::Warning("call: Empty RPGSS function name");

		return true;
	}

	for (auto& plugin: dynrpg_instance.plugins) {
		if (plugin->Invoke(dynrpg_instance, func_name, args.subspan(1), do_yield, interpreter)) {
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

bool DynRpg::EasyRpgPlugin::EasyRaw(dyn_arg_list args, Game_Interpreter* interpreter) {
	if (!interpreter) {
		return true;
	}

	auto func = "easyrpg_raw";
	bool okay = false;

	lcf::rpg::EventCommand cmd;
	std::vector<int32_t> output_args;

	if (args.empty()) {
		Output::Warning("easyrpg_raw: Command too short");
		return true;
	}

	Constants Constants;
	std::string keyToPrint = "ENABLE";

	//Output::Warning("Key {}, value {}", keyToPrint, Constants.get("DestinyScript",keyToPrint));

	auto evt = args[0];
	if (evt.find("@") == 0) {
		evt = evt.substr(1);
		evt = Constants.get("EventCode", evt);

		cmd.code = stoi(evt);
		okay = bool(cmd.code);
	} else
		std::tie(cmd.code) = DynRpg::ParseArgs<int>(func, args, &okay);

	if (!okay) {
		Output::Warning("EasyRpgPlugin - Unknown Input: {}",args[0]);
		return true;
	}

	if (args.size() >= 2) {
		auto [string_arg] = DynRpg::ParseArgs<std::string>(func, args.subspan(1), &okay);
		cmd.string = lcf::DBString(string_arg);

		if (!okay) {
			return true;
		}

		for (size_t i = 2; i < args.size(); ++i) {
			auto [int_arg] = DynRpg::ParseArgs<int>(func, args.subspan(i), &okay);

			if (!okay) {
				return true;
			}
		}
	}

	cmd.parameters = lcf::DBArray<int32_t>(output_args.begin(), output_args.end());

	interpreter->Push({ cmd }, 0, false);

	return true;
}

bool DynRpg::EasyRpgPlugin::Invoke(Game_DynRpg& dynrpg_instance, StringView func, dyn_arg_list args, bool& do_yield, Game_Interpreter* interpreter) {
	if (func == "call") {
		return EasyCall(dynrpg_instance, args, do_yield, interpreter);
	} else if (func == "easyrpg_output") {
		return EasyOput(args);
	} else if (func == "easyrpg_add") {
		return EasyAdd(args);
	} else if (func == "easyrpg_raw") {
		return EasyRaw(args, interpreter);
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
