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

namespace Game_Message {
	PendingMessage pending_message;

	bool message_waiting;
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

int Game_Message::PendingMessage::PushLine(std::string msg) {
	assert(!HasChoices());
	assert(!HasNumberInput());
	texts.push_back(std::move(msg));
	return texts.size();
}

int Game_Message::PendingMessage::PushChoice(std::string msg, bool enabled) {
	assert(!HasNumberInput());
	if (!HasChoices()) {
		choice_start = NumLines();
	}
	choice_enabled[GetNumChoices()] = enabled;
	texts.push_back(std::move(msg));
	return texts.size();
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
	if (texts.empty()) {
		texts.push_back("");
	}
	texts.back().push_back('\f');
}

void Game_Message::PendingMessage::SetWordWrapped(bool value) {
	assert(texts.empty());
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


