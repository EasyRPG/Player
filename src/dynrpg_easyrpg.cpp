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

static bool EasyOput(const dyn_arg_list& args) {
	DYNRPG_FUNCTION("output")

	DYNRPG_CHECK_ARG_LENGTH(2);

	DYNRPG_GET_STR_ARG(0, mode);
	DYNRPG_GET_VAR_ARG(1, msg);

	if (mode == "Debug") {
		Output::DebugStr(msg);
	} else if (mode == "Info") {
		Output::InfoStr(msg);
	} else if (mode == "Warning") {
		Output::WarningStr(msg);
	} else if (mode == "Error") {
		Output::ErrorStr(msg);
	}

	return true;
}

static bool EasyCall(const dyn_arg_list& args) {
	DYNRPG_FUNCTION("call")

	DYNRPG_CHECK_ARG_LENGTH(1)

	DYNRPG_GET_STR_ARG(0, token)

	if (token.empty()) {
		// empty function name
		Output::Warning("call: Empty RPGSS function name");

		return true;
	}

	if (!DynRpg::HasFunction(token)) {
		// Not a supported function
		Output::Warning("Unsupported RPGSS function: {}", token);
		return true;
	}

	dyn_arg_list new_args(args.begin() + 1, args.end());

	return DynRpg::Invoke(token, new_args);
}

static bool EasyAdd(const dyn_arg_list& args) {
	DYNRPG_FUNCTION("easyrpg_add")

	DYNRPG_CHECK_ARG_LENGTH(2);

	DYNRPG_GET_INT_ARG(0, target_var);

	int res = 0;
	for (size_t i = 1; i < args.size(); ++i) {
		DYNRPG_GET_INT_ARG(i, val);
		res += val;
	}

	Main_Data::game_variables->Set(target_var, res);

	return true;
}

void DynRpg::EasyRpgPlugin::RegisterFunctions() {
	DynRpg::RegisterFunction("call", EasyCall);
	DynRpg::RegisterFunction("easyrpg_output", EasyOput);
	DynRpg::RegisterFunction("easyrpg_add", EasyAdd);
}

void DynRpg::EasyRpgPlugin::Load(std::vector<uint8_t>& buffer) {
	if (buffer.size() < 4) {
		Output::Warning("EasyRpgPlugin: Bad savegame data");
	} else {
		uint32_t ver = *reinterpret_cast<uint32_t*>(buffer.data());
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
