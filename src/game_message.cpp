/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

#include "game_message.h"

Game_Message::Game_Message():
	visible(false)
{
	Clear();
}

void Game_Message::Clear() {
	texts.clear();
	face_name.clear();
	face_index = 0;
	background = 0;
	position = 2;
	choice_start = 99;
	choice_max = 0;
	choice_cancel_type = 0;
	num_input_variable_id = 0;
	num_input_digits_max = 0;
}

bool Game_Message::Busy() {
	return (texts.size() > 0);
}
