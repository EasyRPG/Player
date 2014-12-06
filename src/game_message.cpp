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
#include "main_data.h"

namespace Game_Message {
	std::vector<std::string> texts;

	unsigned int owner_id;

	int choice_start;
	int num_input_start;

	int choice_max;
	std::bitset<8> choice_disabled;

	int choice_cancel_type;

	int num_input_variable_id;
	int num_input_digits_max;

	bool message_waiting;
	bool visible;
	bool closing;

	int choice_result;
}

RPG::SaveSystem& data = Main_Data::game_data.system;

void Game_Message::Init() {
	FullClear();
}

void Game_Message::SemiClear() {
	texts.clear();
	owner_id = 0;
	choice_disabled.reset();
	choice_start = 99;
	choice_max = 0;
	choice_cancel_type = 0;
	num_input_start = -1;
	num_input_variable_id = 0;
	num_input_digits_max = 0;
}

void Game_Message::FullClear() {
	SemiClear();
	SetFaceName("");
	SetFaceIndex(0);

	/*background = true;
	position = 2;
	fixed_position = false;
	dont_halt = false;*/
}

bool Game_Message::Busy() {
	return texts.size() > 0;
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
	return data.message_continue_events != 0;
}

void Game_Message::SetContinueEvents(bool continue_events) {
	data.message_continue_events = continue_events;
}
