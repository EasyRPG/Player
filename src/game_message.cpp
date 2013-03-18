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

namespace Game_Message {
	std::vector<std::string> texts;

	std::string face_name;
	int face_index;
	bool face_flipped;
	bool face_left_position;

	bool background;

	int choice_start;
	int num_input_start;

	int choice_max;
	std::bitset<8> choice_disabled;

	int choice_cancel_type;

	int num_input_variable_id;
	int num_input_digits_max;

	int position;
	bool fixed_position;
	bool dont_halt;
	bool message_waiting;

	bool visible;

	int choice_result;
}

void Game_Message::Init() {
	background = true;
	position = 2;

	FullClear();
}

void Game_Message::SemiClear() {
	texts.clear();
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
	face_name.clear();
	face_index = 0;
	/*background = true;
	position = 2;
	fixed_position = false;
	dont_halt = false;*/
}

bool Game_Message::Busy() {
	return texts.size() > 0;
}
