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
#include "filefinder.h"
#include "output.h"
#include <cstdlib>
#include <lcf/rpg/eventcommand.h>

namespace {
	using Cmd = lcf::rpg::EventCommand::Code;

	/** Commands DynParams never rewrites: the overwrite waits for the next one. */
	bool IsSkipped(int code) {
		switch (static_cast<Cmd>(code)) {
			case Cmd::END:
			case Cmd::ShowChoiceEnd:
			case Cmd::EndBattle:
			case Cmd::EndShop:
			case Cmd::EndInn:
			case Cmd::EndBranch:
			case Cmd::EndLoop:
			case Cmd::EndBranch_B:
				return true;
			default:
				return code == 0;
		}
	}

	int ArgToInt(std::string_view func, dyn_arg_list args, int i) {
		// DynRPG numbers are doubles; the plugin truncates them to int
		if (static_cast<size_t>(i) >= args.size() || args[i].empty()) {
			return 0;
		}
		char* end = nullptr;
		double value = std::strtod(args[i].c_str(), &end);
		if (end == args[i].c_str()) {
			Output::Warning("{}: Arg {} ({}) is not numeric", func, i, args[i]);
			return 0;
		}
		return static_cast<int>(value);
	}
}

bool DynRpg::ParamsPlugin::Invoke(std::string_view func, dyn_arg_list args, bool&, Game_Interpreter*) {
	if (func == "dynparams_add_param") {
		if (args.size() == 1) {
			params.emplace_back(index, ArgToInt(func, args, 0));
			++index;
		} else {
			params.emplace_back(ArgToInt(func, args, 0), ArgToInt(func, args, 1));
		}
		return true;
	}
	if (func == "dynparams_set_index") {
		index = ArgToInt(func, args, 0);
		return true;
	}
	if (func == "dynparams_append_string") {
		// Raw comment text after "@dynparams_append_string "
		std::string_view comment = instance.GetCurrentComment();
		constexpr size_t prefix = sizeof("@dynparams_append_string ") - 1;
		if (comment.size() > prefix) {
			text += std::string(comment.substr(prefix));
		}
		text_set = true;
		return true;
	}
	if (func == "dynparams_append_number") {
		text += std::to_string(ArgToInt(func, args, 0));
		text_set = true;
		return true;
	}
	if (func == "dynparams_overwrite_next") {
		overwrite = true;
		return true;
	}
	if (func == "dynparams_clear_params") {
		Clear();
		return true;
	}
	if (func == "dynparams_start_record") {
		record = true;
		return true;
	}
	if (func == "dynparams_stop_record") {
		record = false;
		return true;
	}
	return false;
}

void DynRpg::ParamsPlugin::OnEventCommand(lcf::rpg::EventCommand& com) {
	if (overwrite && !IsSkipped(com.code)) {
		if (text_set) {
			com.string = lcf::DBString(text);
		}
		std::vector<int32_t> values(com.parameters.begin(), com.parameters.end());
		for (const auto& [i, value] : params) {
			if (i < 1) {
				Output::Warning("DynParams: Invalid parameter index {}", i);
				continue;
			}
			if (static_cast<size_t>(i) > values.size()) {
				values.resize(i, 0);
			}
			values[i - 1] = value;
		}
		com.parameters = lcf::DBArray<int32_t>(values.begin(), values.end());
		Clear();
	}

	if (record) {
		Record(com);
	}
}

void DynRpg::ParamsPlugin::Clear() {
	params.clear();
	text.clear();
	text_set = false;
	overwrite = false;
	index = default_index;
}

void DynRpg::ParamsPlugin::Record(const lcf::rpg::EventCommand& com) {
	if (!record_file) {
		// Truncated on the first write of the session, then appended
		FileFinder::Save().MakeDirectory("DynPlugins", false);
		record_file = FileFinder::Save().OpenOutputStream("DynPlugins/DynParamsRecord.txt",
			std::ios_base::out | std::ios_base::trunc);
		if (!record_file) {
			Output::Warning("DynParams: Cannot write DynPlugins/DynParamsRecord.txt");
			record = false;
			return;
		}
	}

	record_file << "EventCommand type: " << com.code << "\n";
	record_file << "String parameter: \"" << ToString(com.string) << "\"\n";
	record_file << "Number parameters:\n";
	for (size_t i = 0; i < com.parameters.size(); ++i) {
		record_file << (i + 1) << ": " << com.parameters[i] << "\n";
	}
	record_file << "\n";
	record_file.flush();
}
