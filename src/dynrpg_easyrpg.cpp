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

bool DynRpg::EasyRpgPlugin::Invoke(std::string_view func, dyn_arg_list args, bool& do_yield, Game_Interpreter* interpreter) {
	if (func == "call") {
		return EasyCall(args, do_yield, interpreter);
	} else if (func == "easyrpg_output") {
		return EasyOput(args);
	} else if (func == "easyrpg_add") {
		return EasyAdd(args);
	} else if (func == "easyrpg_set_event_frame") {
    auto func_name = "easyrpg_set_event_frame";
    bool okay = false;
    int char_id;
    std::string sequence_str;

    std::tie(char_id, sequence_str) = DynRpg::ParseArgs<int, std::string>(func_name, args, &okay);
    if (!okay) return true;

    Game_Character* target = interpreter->GetCharacter(char_id, func_name);
    if (target) {
        std::vector<int> seq;
        auto tokens = Utils::Tokenize(sequence_str, [](char32_t c) { return c == ','; });
        for (const auto& t : tokens) {
            try {
                seq.push_back(std::stoi(t));
            } catch (...) {
                Output::Warning("Invalid frame index in sequence: {}", t);
            }
        }
        target->SetCustomAnimationSequence(seq);
    }
    return true;
    } else if (func == "easyrpg_get_event_frame") {
    auto func_name = "easyrpg_get_event_frame";
    bool okay = false;
    int char_id, var_id;

    // Arguments: Event ID (0=this, -1=player, >0=ID), Variable ID to store result
    std::tie(char_id, var_id) = DynRpg::ParseArgs<int, int>(func_name, args, &okay);
    if (!okay) return true;

    Game_Character* target = interpreter->GetCharacter(char_id, func_name);
    if (target) {
        // Get the current logical animation frame
        int frame = target->GetAnimFrame();

        // Store it in the requested variable
        Main_Data::game_variables->Set(var_id, frame);

        // Ensure the map refreshes if any event depends on this variable
        Game_Map::SetNeedRefreshForVarChange(var_id);
    }
    return true;
    } else if (func == "easyrpg_event_anim_reset") {
    bool okay = false;
    int char_id;
    std::tie(char_id) = DynRpg::ParseArgs<int>("easyrpg_event_anim_reset", args, &okay);
    if (!okay) return true;

    Game_Character* target = interpreter->GetCharacter(char_id, "easyrpg_event_anim_reset");
    if (target) {
        target->ClearCustomAnimationSequence();
    }
    return true;
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
