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
#include "game_interpreter.h"
#include "output.h"
#include <cstdlib>
#include <lcf/inireader.h>
#include <lcf/rpg/eventcommand.h>
#include <lcf/rpg/saveeventexecframe.h>

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

	std::string ArgToString(dyn_arg_list args, int i) {
		return static_cast<size_t>(i) < args.size() ? args[i] : std::string();
	}
}

DynRpg::ParamsPlugin::ParamsPlugin(Game_DynRpg& instance) : DynRpgPlugin("DynParams", instance) {
	if (auto is = FileFinder::Game() ? FileFinder::Game().OpenFile("DynRPG.ini") : Filesystem_Stream::InputStream()) {
		lcf::INIReader ini(is);
		// DynParams.dll: any number but 0
		debug_log = ini.GetInteger("DynParams", "DebugLog", 0) != 0;
	}
}

bool DynRpg::ParamsPlugin::Invoke(std::string_view func, dyn_arg_list args, bool&, Game_Interpreter* interpreter) {
	if (func == "dynparams_start_record") {
		record = true;
		return true;
	}
	if (func == "dynparams_stop_record") {
		record = false;
		return true;
	}

	if (func == "dynparams_add_param") {
		if (args.size() == 1) {
			state.params[state.index] = ArgToInt(func, args, 0);
			++state.index;
		} else {
			state.params[ArgToInt(func, args, 0)] = ArgToInt(func, args, 1);
		}
		return true;
	}
	if (func == "dynparams_set_index") {
		state.index = ArgToInt(func, args, 0);
		return true;
	}
	if (func == "dynparams_append_string") {
		// Raw comment text after "@dynparams_append_string "
		std::string_view comment = instance.GetCurrentComment();
		constexpr size_t prefix = sizeof("@dynparams_append_string ") - 1;
		if (comment.size() > prefix) {
			state.text += std::string(comment.substr(prefix));
		}
		state.text_set = true;
		return true;
	}
	if (func == "dynparams_append_number") {
		state.text += std::to_string(ArgToInt(func, args, 0));
		state.text_set = true;
		return true;
	}
	if (func == "dynparams_message_line_append_string" || func == "dynparams_message_line_append_number") {
		int line = ArgToInt(func, args, 0);
		if (line < 1 || line > message_lines) {
			Output::Warning("{}: Message line {} is not between 1 and {}", func, line, message_lines);
			return true;
		}
		bool number = func == "dynparams_message_line_append_number";
		state.lines[line - 1] += number ? std::to_string(ArgToInt(func, args, 1)) : ArgToString(args, 1);
		state.lines_set = true;
		return true;
	}
	if (func == "dynparams_choice_case_append_string" || func == "dynparams_choice_case_append_number") {
		int choice = ArgToInt(func, args, 0);
		if (choice < 1) {
			Output::Warning("{}: Choice {} is not 1 or greater", func, choice);
			return true;
		}
		bool number = func == "dynparams_choice_case_append_number";
		state.choices[choice - 1] += number ? std::to_string(ArgToInt(func, args, 1)) : ArgToString(args, 1);
		return true;
	}
	if (func == "dynparams_overwrite_next") {
		state.overwrite = true;
		if (debug_log) {
			const auto* frame = interpreter && !interpreter->GetState().stack.empty() ? &interpreter->GetState().stack.back() : nullptr;
			Log(fmt::format("Setting overwrite next flag to true: event {}, line {}",
				frame ? frame->event_id : 0, frame ? frame->current_command : 0));
		}
		return true;
	}
	if (func == "dynparams_clear_params") {
		state = {};
		return true;
	}
	if (func == "dynparams_list_params") {
		List();
		return true;
	}
	return false;
}

std::optional<lcf::rpg::EventCommand> DynRpg::ParamsPlugin::OnEventCommand(const Game_Interpreter&, lcf::rpg::SaveEventExecFrame& frame) {
	const auto& com = frame.commands[frame.current_command];
	std::optional<lcf::rpg::EventCommand> rewritten;

	if (state.overwrite && !IsSkipped(com.code)) {
		rewritten = com;
		Apply(*rewritten, frame);
		// Applying resets everything
		state = {};
	}

	if (record) {
		Record(rewritten ? *rewritten : com);
	}
	return rewritten;
}

void DynRpg::ParamsPlugin::Apply(lcf::rpg::EventCommand& com, lcf::rpg::SaveEventExecFrame& frame) {
	auto& commands = frame.commands;
	const size_t current = frame.current_command;
	Log(fmt::format("Overwriting event line: command {}, event {}, line {}", com.code, frame.event_id, current));

	if (state.text_set) {
		Log(fmt::format("Overwriting main string parameter with: '{}'", state.text));
		com.string = lcf::DBString(state.text);
	}

	if (state.lines_set) {
		// The first line, then the message lines that follow (not restored after execution)
		Log("Overwriting message line string parameters");
		com.string = lcf::DBString(state.lines[0]);
		for (int line = 1; line < message_lines; ++line) {
			size_t i = current + line;
			if (i >= commands.size() || commands[i].code != static_cast<int>(Cmd::ShowMessage_2)) {
				break;
			}
			Log(fmt::format("Line {}: '{}'", line, state.lines[line]));
			commands[i].string = lcf::DBString(state.lines[line]);
		}
	}

	if (!state.choices.empty()) {
		Log("Overwriting choice case string parameters");
		// The choices of this Show Choices, or of the one right after this message
		size_t i = current;
		if (com.code == static_cast<int>(Cmd::ShowMessage)) {
			++i;
			while (i < commands.size() && commands[i].code == static_cast<int>(Cmd::ShowMessage_2)) {
				++i;
			}
		}
		if (i < commands.size() && commands[i].code == static_cast<int>(Cmd::ShowChoice)) {
			int nesting = 0;
			int choice = 0;
			for (++i; i < commands.size(); ++i) {
				auto& line = commands[i];
				if (line.code == static_cast<int>(Cmd::ShowChoice)) {
					++nesting;
				} else if (line.code == static_cast<int>(Cmd::ShowChoiceEnd)) {
					if (nesting == 0) {
						break;
					}
					--nesting;
				} else if (line.code == static_cast<int>(Cmd::ShowChoiceOption) && nesting == 0) {
					auto text = state.choices.find(choice);
					if (text != state.choices.end()) {
						Log(fmt::format("Choice case {}: '{}'", choice + 1, text->second));
						line.string = lcf::DBString(text->second);
					}
					++choice;
				}
			}
		}
	}

	if (!state.params.empty()) {
		Log("Overwriting number parameters");
	}
	for (const auto& [i, value] : state.params) {
		// DynParams.dll writes out of bounds (RPG_RT crashes or corrupts memory)
		if (i < 1 || static_cast<size_t>(i) > com.parameters.size()) {
			Output::Warning("DynParams: Command {} has no parameter {} (it has {})", com.code, i, com.parameters.size());
			continue;
		}
		Log(fmt::format("Number parameter {}: '{}'", i, value));
		com.parameters[i - 1] = value;
	}
	Log("Overwriting of event line completed");
}

void DynRpg::ParamsPlugin::List() const {
	// DynParams.dll shows this in a message box
	for (const auto& [i, value] : state.params) {
		Output::Info("DynParams: Integer parameter {}: {}", i, value);
	}
	Output::Info("DynParams: String parameter: \"{}\"", state.text);
	for (int line = 0; line < message_lines; ++line) {
		Output::Info("DynParams: Message line {} parameter: \"{}\"", line + 1, state.lines[line]);
	}
	for (const auto& [choice, text] : state.choices) {
		Output::Info("DynParams: Choice case {} parameter: \"{}\"", choice + 1, text);
	}
}

void DynRpg::ParamsPlugin::Log(std::string_view text) const {
	if (debug_log) {
		Output::Debug("DynParams: {}", text);
	}
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
