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

#ifndef _GAMEMESSAGE_H_
#define _GAMEMESSAGE_H_

#include <vector>
#include <string>

namespace Game_Message {

	static const int MAX_LINE = 4;

	void Init();
	//~Game_Message();

	/// Used by Window_Message to reset some flags
	void SemiClear();
	/// Used by the Game_Interpreter to completly reset all flags
	void FullClear();

	bool Busy();

	/// Contains the different lines of text
	extern std::vector<std::string> texts;
	/// Name of the file that contains the face
	extern std::string face_name;
	/// index of the face to display
	extern int face_index;
	/// Whether to mirror the face
	extern bool face_flipped;
	/// If the face shall be placed left
	extern bool face_left_position;
	/// If the background graphic is displayed
	extern bool background;

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
	extern int choice_start;
	extern int num_input_start;

	// Number of choices
	extern int choice_max;

	// Option to choose if cancel
	extern int choice_cancel_type;

	extern int num_input_variable_id;
	extern int num_input_digits_max;
	/// Where the msgbox is displayed
	extern int position;
	// don't move the message box to avoid obscuring the player
	extern bool fixed_position;
	// don't wait for a key to be pressed
	extern bool dont_halt;
	/// If a message is currently being processed
	extern bool message_waiting;
	extern bool visible;

	// selected option (4 => cancel)
	extern int choice_result;
}

#endif
