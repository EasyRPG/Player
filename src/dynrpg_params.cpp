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
#include "dynrpg_params.h"
#include "output.h"

bool DynRpg::ParamsPlugin::Invoke(std::string_view func, dyn_arg_list args, bool&, Game_Interpreter*) {
	if (func == "dynparams_add_param") {
		bool okay = false;
		auto [index, value] = DynRpg::ParseArgs<int, int>(func, args, &okay);
		if (!okay) {
			return true;
		}
		if (index < 1) {
			Output::Warning("{}: Invalid parameter index {}", func, index);
			return true;
		}
		instance.pending_params.emplace_back(index, value);
		return true;
	}

	if (func == "dynparams_overwrite_next") {
		instance.next_command_params = std::move(instance.pending_params);
		instance.pending_params.clear();
		return true;
	}

	if (func == "dynparams_clear") {
		instance.pending_params.clear();
		instance.next_command_params.clear();
		return true;
	}

	return false;
}
