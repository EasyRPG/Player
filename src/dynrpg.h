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

#ifndef EP_DYNRPG_H
#define EP_DYNRPG_H

#include <vector>
#include <sstream>
#include <string>
#include "output.h"
#include "utils.h"

// Headers
namespace lcf {
namespace rpg {
	class EventCommand;
}
}

typedef std::vector<std::string> dyn_arg_list;
typedef bool(*dynfunc)(const dyn_arg_list&);

// Macros

#define DYNRPG_FUNCTION(var) \
	std::string func_name = var;

#define DYNRPG_CHECK_ARG_LENGTH(len) \
	if (args.size() < len) { \
		Output::Warning("{}: Got {} args (needs {} or more)", func_name, args.size(), len); \
		return true; \
	}

#define DYNRPG_GET_FLOAT_ARG(i, var) \
	float var; \
	bool valid_float##var; \
	var = DynRpg::GetFloat(args[i], &valid_float##var); \
	if (!valid_float##var) { \
	Output::Warning("{}: Arg {} ({}) is not numeric", func_name, i, args[i]); \
	return true; \
	}

#define DYNRPG_GET_INT_ARG(i, var) \
	DYNRPG_GET_FLOAT_ARG(i, var##_float_arg) \
	int var = (int)var##_float_arg;

#define DYNRPG_GET_STR_ARG(i, var) \
	const std::string& var = args[i];

#define DYNRPG_GET_VAR_ARG(i, var) \
	std::string var = DynRpg::ParseVarArg(args, i); \
	if (var.empty()) { \
	Output::Warning("{}: Vararg {} out of range", func_name, i); \
	}

/**
 * DynRPG namespace
 */
namespace DynRpg {
	void RegisterFunction(const std::string& name, dynfunc function);
	float GetFloat(const std::string& str, bool* valid = nullptr);
	std::string ParseVarArg(const dyn_arg_list &args, int index);
	bool Invoke(const std::string& command);
	void Update();
	void Reset();
}

#endif
