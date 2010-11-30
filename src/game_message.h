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

#ifndef _H_GAMEMESSAGE
#define _H_GAMEMESSAGE

#include <vector>
#include <string>

class Game_Message {

public:

	static const int MAX_LINE = 4;

	Game_Message();
	//~Game_Message();

	void Clear();

	bool Busy();

	std::vector<std::string> texts;
	std::string face_name;
	int face_index;
	bool face_flipped;
	bool face_left_position;

	int background;

	/* Number of lines before the start
	of selection options.
	+-----------------------------------+
	|	Hi, hero, What's your name?		|
	|- Alex								|
	|- Brian							|
	|- Carol							|
	+-----------------------------------+
	In this case, choice_start would be 1.
	Same with num_input_start.
	*/
	int choice_start;
	int num_input_start;

	// Number of choices
	int choice_max;

	// Option to choose if cancel
	int choice_cancel_type;

	int num_input_variable_id;
	int num_input_digits_max;

	int position;

	bool visible;

};

#endif
