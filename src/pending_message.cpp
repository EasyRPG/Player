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

#include "pending_message.h"
#include "game_variables.h"
#include "game_strings.h"
#include "game_actors.h"
#include "game_message.h"
#include "game_switches.h"
#include <lcf/data.h>
#include "output.h"
#include "text.h"
#include "utils.h"
#include "player.h"
#include "main_data.h"
#include <cassert>
#include <cctype>
#include <algorithm>
#include <utility>

static void RemoveControlChars(std::string& s) {
	// RPG_RT ignores any control characters within messages.
	auto iter = std::remove_if(s.begin(), s.end(), [](const char c) { return Utils::IsControlCharacter(c); });
	s.erase(iter, s.end());
}

PendingMessage::PendingMessage(PendingMessage::CommandInserter cmd_fn) :
	command_inserter(std::move(cmd_fn)) {
	// no-op
};

int PendingMessage::PushLineImpl(std::string msg) {
	RemoveControlChars(msg);
	msg = ApplyTextInsertingCommands(std::move(msg), Player::escape_char, command_inserter);
	texts.push_back(std::move(msg));
	runs.push_back(Text::Bidi(texts.back(), Text::Direction::LTR));
	return texts.size();
}

int PendingMessage::PushLine(std::string msg) {
	assert(!HasChoices());
	assert(!HasNumberInput());
	return PushLineImpl(std::move(msg));
}

int PendingMessage::PushChoice(std::string msg, bool enabled) {
	assert(!HasNumberInput());
	if (!HasChoices()) {
		choice_start = NumLines();
	}
	choice_enabled[GetNumChoices()] = enabled;
	return PushLineImpl(std::move(msg));
}

int PendingMessage::PushNumInput(int variable_id, int num_digits) {
	assert(!HasChoices());
	assert(!HasNumberInput());
	num_input_variable = variable_id;
	num_input_digits = num_digits;
	return NumLines();
}

void PendingMessage::PushPageEnd() {
	assert(!HasChoices());
	assert(!HasNumberInput());
	// FIXME: Runs
	if (texts.empty()) {
		texts.push_back("");
	}
	texts.back().push_back('\f');
}

void PendingMessage::SetWordWrapped(bool value) {
	assert(texts.empty());
	word_wrapped = value;
}

void PendingMessage::SetChoiceCancelType(int value) {
	choice_cancel_type = value;
}

void PendingMessage::SetChoiceResetColors(bool value) {
	choice_reset_color = value;
}

std::string PendingMessage::ApplyTextInsertingCommands(std::string input, uint32_t escape_char, const CommandInserter& cmd_fn) {
	if (input.empty()) {
		return input;
	}

	std::string output;

	const char* iter = input.data();
	const auto end = input.data() + input.size();

	const char* start_copy = iter;
	while (iter != end) {
		auto ret = Utils::UTF8Next(iter, end);
		if (ret.ch != escape_char) {
			iter = ret.next;
			continue;
		}

		// utf8 parsing failed
		if (ret.ch == 0) {
			break;
		}

		output.append(start_copy, iter - start_copy);
		start_copy = iter;

		iter = ret.next;
		if (iter == end) {
			break;
		}

		const auto ch = *iter;
		++iter;

		auto fn_res = cmd_fn(ch, &iter, end, escape_char);
		if (fn_res) {
			output.append(*fn_res);
			start_copy = iter;
		}
	}

	if (start_copy == input.data()) {
		// Fast path - no substitutions occured, so just move the input into the return value.
		output = std::move(input);
	} else {
		output.append(start_copy, end - start_copy);
	}

	return output;
}

std::optional<std::string> PendingMessage::DefaultCommandInserter(char ch, const char** iter, const char* end, uint32_t escape_char) {
	if (ch == 'N' || ch == 'n') {
		auto parse_ret = Game_Message::ParseActor(*iter, end, escape_char, true);
		*iter = parse_ret.next;
		int value = parse_ret.value;

		const auto* actor = Main_Data::game_actors->GetActor(value);
		if (!actor) {
			Output::Warning("Invalid Actor Id {} in message text", value);
			return "";
		} else {
			return ToString(actor->GetName());
		}
	} else if (ch == 'V' || ch == 'v') {
		auto parse_ret = Game_Message::ParseVariable(*iter, end, escape_char, true);
		*iter = parse_ret.next;
		int value = parse_ret.value;

		int variable_value = Main_Data::game_variables->Get(value);
		return std::to_string(variable_value);
	}

	return std::nullopt;
}
