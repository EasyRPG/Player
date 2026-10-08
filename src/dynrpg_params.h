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

#ifndef EP_DYNRPG_PARAMS_H
#define EP_DYNRPG_PARAMS_H

#include "game_dynrpg.h"

namespace DynRpg {
	/**
	 * DynParams plugin: replaces parameters of the next event command.
	 *
	 *   @dynparams_add_param 2, V152    (parameter 2 := value of variable 152)
	 *   @dynparams_overwrite_next       (apply to the next event command)
	 *
	 * Parameter indices are 1-based.
	 */
	class ParamsPlugin : public DynRpgPlugin {
	public:
		ParamsPlugin(Game_DynRpg& instance) : DynRpgPlugin("DynParams", instance) {}

		bool Invoke(std::string_view func, dyn_arg_list args, bool& do_yield, Game_Interpreter* interpreter) override;
	};
}

#endif
