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

DynRpg::ParamsPlugin::ScriptKey DynRpg::ParamsPlugin::KeyOf(const Game_Interpreter* interpreter) {
	if (!interpreter) {
		return {nullptr, 0};
	}
	return {interpreter, static_cast<int>(interpreter->GetState().stack.size()) - 1};
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

	auto& script = scripts[KeyOf(interpreter)];

	if (func == "dynparams_add_param") {
		if (args.size() == 1) {
			script.params.emplace_back(script.index, ArgToInt(func, args, 0));
			++script.index;
		} else {
			script.params.emplace_back(ArgToInt(func, args, 0), ArgToInt(func, args, 1));
		}
		return true;
	}
	if (func == "dynparams_set_index") {
		script.index = ArgToInt(func, args, 0);
		return true;
	}
	if (func == "dynparams_append_string") {
		// Raw comment text after "@dynparams_append_string "
		std::string_view comment = instance.GetCurrentComment();
		constexpr size_t prefix = sizeof("@dynparams_append_string ") - 1;
		if (comment.size() > prefix) {
			script.text += std::string(comment.substr(prefix));
		}
		script.text_set = true;
		return true;
	}
	if (func == "dynparams_append_number") {
		script.text += std::to_string(ArgToInt(func, args, 0));
		script.text_set = true;
		return true;
	}
	if (func == "dynparams_message_line_append_string" || func == "dynparams_message_line_append_number") {
		int line = ArgToInt(func, args, 0);
		if (line < 1 || line > message_lines) {
			Output::Warning("{}: Message line {} is not between 1 and {}", func, line, message_lines);
			return true;
		}
		bool number = func == "dynparams_message_line_append_number";
		script.lines[line - 1] += number ? std::to_string(ArgToInt(func, args, 1)) : ArgToString(args, 1);
		script.lines_set = true;
		return true;
	}
	if (func == "dynparams_choice_case_append_string" || func == "dynparams_choice_case_append_number") {
		int choice = ArgToInt(func, args, 0);
		if (choice < 1) {
			Output::Warning("{}: Choice {} is not 1 or greater", func, choice);
			return true;
		}
		bool number = func == "dynparams_choice_case_append_number";
		script.choices[choice - 1] += number ? std::to_string(ArgToInt(func, args, 1)) : ArgToString(args, 1);
		return true;
	}
	if (func == "dynparams_overwrite_next") {
		script.overwrite = true;
		return true;
	}
	if (func == "dynparams_clear_params") {
		script = {};
		return true;
	}
	if (func == "dynparams_list_params") {
		List(script);
		return true;
	}
	return false;
}

std::optional<lcf::rpg::EventCommand> DynRpg::ParamsPlugin::OnEventCommand(const Game_Interpreter& interpreter, lcf::rpg::SaveEventExecFrame& frame) {
	const auto& com = frame.commands[frame.current_command];
	std::optional<lcf::rpg::EventCommand> rewritten;

	auto it = scripts.find(KeyOf(&interpreter));
	if (it != scripts.end() && it->second.overwrite && !IsSkipped(com.code)) {
		rewritten = com;
		Apply(it->second, *rewritten, frame);
		// Applying resets everything
		scripts.erase(it);
	}

	if (record) {
		Record(rewritten ? *rewritten : com);
	}
	return rewritten;
}

void DynRpg::ParamsPlugin::OnFramesReset(const Game_Interpreter* interpreter, int first_frame) {
	auto it = scripts.lower_bound({interpreter, first_frame});
	while (it != scripts.end() && it->first.first == interpreter) {
		it = scripts.erase(it);
	}
}

void DynRpg::ParamsPlugin::Apply(Script& script, lcf::rpg::EventCommand& com, lcf::rpg::SaveEventExecFrame& frame) {
	auto& commands = frame.commands;
	const size_t current = frame.current_command;

	if (script.text_set) {
		com.string = lcf::DBString(script.text);
	}

	if (script.lines_set) {
		// The first line, then the message lines that follow (not restored after execution)
		com.string = lcf::DBString(script.lines[0]);
		for (int line = 1; line < message_lines; ++line) {
			size_t i = current + line;
			if (i >= commands.size() || commands[i].code != static_cast<int>(Cmd::ShowMessage_2)) {
				break;
			}
			commands[i].string = lcf::DBString(script.lines[line]);
		}
	}

	if (!script.choices.empty()) {
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
					auto text = script.choices.find(choice);
					if (text != script.choices.end()) {
						line.string = lcf::DBString(text->second);
					}
					++choice;
				}
			}
		}
	}

	for (const auto& [i, value] : script.params) {
		// DynParams.dll writes out of bounds (RPG_RT crashes or corrupts memory)
		if (i < 1 || static_cast<size_t>(i) > com.parameters.size()) {
			Output::Warning("DynParams: Command {} has no parameter {} (it has {})", com.code, i, com.parameters.size());
			continue;
		}
		com.parameters[i - 1] = value;
	}
}

void DynRpg::ParamsPlugin::List(const Script& script) const {
	// DynParams.dll shows this in a message box
	for (const auto& [i, value] : script.params) {
		Output::Info("DynParams: Integer parameter {}: {}", i, value);
	}
	Output::Info("DynParams: String parameter: \"{}\"", script.text);
	for (int line = 0; line < message_lines; ++line) {
		Output::Info("DynParams: Message line {} parameter: \"{}\"", line + 1, script.lines[line]);
	}
	for (const auto& [choice, text] : script.choices) {
		Output::Info("DynParams: Choice case {} parameter: \"{}\"", choice + 1, text);
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
