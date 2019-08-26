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
#include "game_message.h"
#include "game_player.h"
#include "game_temp.h"
#include "main_data.h"
#include "font.h"
#include "player.h"
#include "game_variables.h"
#include "data.h"
#include "reader_util.h"
#include "output.h"

#include <cctype>

namespace Game_Message {
	PendingMessage pending_message;

	bool message_waiting;
	bool closing;
	bool visible;
}

RPG::SaveSystem& data = Main_Data::game_data.system;

void Game_Message::Init() {
	ClearFace();
	pending_message = {};
}

void Game_Message::ClearFace() {
	SetFaceName("");
	SetFaceIndex(0);
}

std::string Game_Message::GetFaceName() {
	return data.face_name;
}

void Game_Message::SetFaceName(const std::string& face) {
	data.face_name = face;
}

int Game_Message::GetFaceIndex() {
	return data.face_id;
}

void Game_Message::SetFaceIndex(int index) {
	data.face_id = index;
}

bool Game_Message::IsFaceFlipped() {
	return data.face_flip;
}

void Game_Message::SetFaceFlipped(bool flipped) {
	data.face_flip = flipped;
}

bool Game_Message::IsFaceRightPosition() {
	return data.face_right;
}

void Game_Message::SetFaceRightPosition(bool right) {
	data.face_right = right;
}

bool Game_Message::IsTransparent() {
	if (Player::IsRPG2k() && Game_Temp::battle_running) {
		return false;
	}

	return data.message_transparent != 0;
}

void Game_Message::SetTransparent(bool transparent) {
	data.message_transparent = transparent;
}

int Game_Message::GetPosition() {
	return data.message_position;
}

void Game_Message::SetPosition(int new_position) {
	data.message_position = new_position;
}

bool Game_Message::IsPositionFixed() {
	return !data.message_prevent_overlap;
}

void Game_Message::SetPositionFixed(bool fixed) {
	data.message_prevent_overlap = !fixed;
}

bool Game_Message::GetContinueEvents() {
	return !!data.message_continue_events;
}

void Game_Message::SetContinueEvents(bool continue_events) {
	data.message_continue_events = continue_events;
}

int Game_Message::GetRealPosition() {
	if (Game_Temp::battle_running) {
		if (Player::IsRPG2k()) {
			return 2;
		}
		else {
			return 0;
		}
	}

	if (Game_Message::IsPositionFixed()) {
		return Game_Message::GetPosition();
	}
	else {
		// Move Message Box to prevent player hiding
		int disp = Main_Data::game_player->GetScreenY();

		switch (Game_Message::GetPosition()) {
		case 0: // Up
			return disp > (16 * 7) ? 0 : 2;
		case 1: // Center
			if (disp <= 16 * 7) {
				return 2;
			}
			else if (disp >= 16 * 10) {
				return 0;
			}
			else {
				return 1;
			}
		default: // Down
			return disp >= (16 * 10) ? 0 : 2;
		};
	}
}

int Game_Message::WordWrap(const std::string& line, const int limit, const std::function<void(const std::string &line)> callback) {
	int start = 0;
	int line_count = 0;
	FontRef font = Font::Default();

	do {
		int next = start;
		do {
			auto found = line.find(" ", next);
			if (found == std::string::npos) {
				found = line.size();
			}

			auto wrapped = line.substr(start, found - start);
			auto width = font->GetSize(wrapped).width;
			if (width > limit) {
				if (next == start) {
					next = found + 1;
				}
				break;
			}

			next = found + 1;
		} while(next < line.size());

		if (start == (next - 1)) {
			start = next;
			continue;
		}

		auto wrapped = line.substr(start, (next - 1) - start);

		callback(wrapped);
		line_count++;

		start = next;
	} while (start < line.size());

	return line_count;
}

static void RemoveControlChars(std::string& s) {
	// RPG_RT ignores any control characters within messages.
	auto iter = std::remove_if(s.begin(), s.end(), [](const char c) { return std::iscntrl(c); });
	s.erase(iter, s.end());
}

static Game_Message::ParseParamResult ParseParamImpl(
		const char upper,
		const char lower,
		const char* iter,
		const char* end,
		uint32_t escape_char,
		bool skip_prefix,
		int max_recursion)
{
	if (!skip_prefix) {
		const auto begin = iter;
		if (iter == end) {
			return { 0, begin };
		}
		auto ret = Utils::UTF8Next(iter, end);
		// Invalid commands
		if (ret.ch != escape_char) {
			return { 0, begin };
		}
		iter = ret.iter;
		if (iter == end || (*iter != upper && *iter != lower)) {
			return { 0, begin };
		}
		++iter;
	}

	// If no bracket, RPG_RT will return 0.
	if (iter == end || *iter != '[') {
		return { 0, iter };
	}

	int value = 0;
	++iter;
	bool stop_parsing = false;
	bool did_variable_subst = 0;

	while (iter != end && *iter != ']') {
		if (stop_parsing) {
			++iter;
			continue;
		}

		// Fast inline isdigit()
		if (*iter >= '0' && *iter <= '9') {
			value *= 10;
			value += (*iter - '0');
			++iter;
			continue;
		}

		if (max_recursion > 0) {
			auto ret = Utils::UTF8Next(iter, end);
			auto ch = ret.ch;
			iter = ret.iter;

			// Recursive variable case.
			if (ch == escape_char) {
				if (iter != end && (*iter == 'V' || *iter == 'v')) {
					++iter;
					did_variable_subst = true;

					auto ret = ParseParamImpl('V', 'v', iter, end, escape_char, true, max_recursion - 1);
					iter = ret.iter;
					int var_val = Game_Variables.Get(ret.value);

					// RPG_RT concatenates the variable value.
					int m = 10;
					if (value != 0) {
						while (m < var_val) {
							m *= 10;
						}
					}
					value = value * m + var_val;
					continue;
				}
			}
		}

		// If we hit a non-digit, RPG_RT will stop parsing until the next closing bracket.
		stop_parsing = true;
	}

	if (iter != end) {
		++iter;
	}

	// RPG_RT will replace varible substitutions that result in 0
	// with 1 to avoid invalid actor crash.
	if (upper == 'N' && value == 0 && did_variable_subst) {
		value = 1;
	}

	return { value, iter };
}

Game_Message::ParseParamResult Game_Message::ParseVariable(const char* iter, const char* end, uint32_t escape_char, bool skip_prefix, int max_recursion) {
	return ParseParamImpl('V', 'v', iter, end, escape_char, skip_prefix, max_recursion);
}

Game_Message::ParseParamResult Game_Message::ParseColor(const char* iter, const char* end, uint32_t escape_char, bool skip_prefix, int max_recursion) {
	return ParseParamImpl('C', 'c', iter, end, escape_char, skip_prefix, max_recursion);
}

Game_Message::ParseParamResult Game_Message::ParseSpeed(const char* iter, const char* end, uint32_t escape_char, bool skip_prefix, int max_recursion) {
	return ParseParamImpl('S', 's', iter, end, escape_char, skip_prefix, max_recursion);
}

Game_Message::ParseParamResult Game_Message::ParseActor(const char* iter, const char* end, uint32_t escape_char, bool skip_prefix, int max_recursion) {
	return ParseParamImpl('N', 'n', iter, end, escape_char, skip_prefix, max_recursion);
}

void Game_Message::ApplyTextInsertingCommands(std::string& output, const std::string& input, uint32_t escape_char) {
	if (input.empty()) {
		return;
	}

	auto iter = input.data();
	const auto end = input.data() + input.size();

	auto start_copy = iter;
	while (iter != end) {
		auto ret = Utils::UTF8Next(iter, end);
		if (ret.ch != escape_char) {
			iter = ret.iter;
			continue;
		}

		// utf8 parsing failed
		if (ret.ch == 0) {
			break;
		}

		output.append(start_copy, iter);
		start_copy = iter;

		iter = ret.iter;
		if (iter == end) {
			break;
		}

		const auto ch = *iter;
		++iter;
		if (ch == 'N' || ch == 'n' || ch == 'V' || ch == 'v') {
			auto parse_ret = ParseActor(iter, end, escape_char, true);
			iter = parse_ret.iter;
			int value = parse_ret.value;

			if (ch == 'N' || ch == 'n') {
				const auto* actor = ReaderUtil::GetElement(Data::actors, value);
				if (!actor) {
					Output::Error("Invalid Actor Id %d in message text", value);
				} else{
					output.append(actor->name);
				}
			}

			if (ch == 'V' || ch == 'v') {
				int variable_value = Game_Variables.Get(value);
				output.append(std::to_string(variable_value));
			}

			start_copy = iter;
		}
	}

	output.append(start_copy, end);
}

int Game_Message::PendingMessage::PushLineImpl(std::string msg) {
	// Current limitation.
	assert(!IsWordWrapped() || NumLines() == 0);

	RemoveControlChars(msg);
	ApplyTextInsertingCommands(text, msg, Player::escape_char);
	text.push_back('\n');
	++num_lines;
	return num_lines;
}

int Game_Message::PendingMessage::PushLine(std::string msg) {
	assert(!HasChoices());
	assert(!HasNumberInput());
	return PushLineImpl(std::move(msg));
}

int Game_Message::PendingMessage::PushChoice(std::string msg, bool enabled) {
	assert(!HasNumberInput());
	if (!HasChoices()) {
		choice_start = NumLines();
	}
	choice_enabled[GetNumChoices()] = enabled;
	return PushLineImpl(std::move(msg));
}

int Game_Message::PendingMessage::PushNumInput(int variable_id, int num_digits) {
	assert(!HasChoices());
	assert(!HasNumberInput());
	num_input_variable = variable_id;
	num_input_digits = num_digits;
	return NumLines();
}

void Game_Message::PendingMessage::PushPageEnd() {
	assert(!HasChoices());
	assert(!HasNumberInput());
	text.push_back('\f');
}

void Game_Message::PendingMessage::SetWordWrapped(bool value) {
	assert(text.empty());
	word_wrapped = true;
}

void Game_Message::PendingMessage::SetChoiceCancelType(int value) {
	choice_cancel_type = value;
}

void Game_Message::SetPendingMessage(PendingMessage&& pm) {
	pending_message = std::move(pm);
	message_waiting = true;
}

const Game_Message::PendingMessage& Game_Message::GetPendingMessage() {
	return pending_message;
}

void Game_Message::PendingMessage::SetChoiceContinuation(ChoiceContinuation f) {
	choice_continuation = std::move(f);
}

void Game_Message::ResetPendingMessage() {
	pending_message = {};
	message_waiting = false;
}

bool Game_Message::CanShowMessage(bool foreground) {
	// If there's a text already, return immediately
	if (Game_Message::message_waiting)
		return false;

	// Forground interpreters: If the message box already started animating we wait for it to finish.
	if (foreground && Game_Message::visible && Game_Message::closing)
		return false;

	// Parallel interpreters must wait until the message window is closed
	if (!foreground && Game_Message::visible)
		return false;

	return true;
}



