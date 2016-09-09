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

#ifndef _GAMEMESSAGE_H_
#define _GAMEMESSAGE_H_

#include <vector>
#include <bitset>
#include <string>

namespace Game_Message {

	static const int MAX_LINE = 4;

	void Init();

	/**
	 * Used by Window_Message to reset some flags.
	 */
	void SemiClear();
	/**
	 * Used by the Game_Interpreter to completly reset all flags.
	 */
	void FullClear();

	/** Contains the different lines of text. */
	extern std::vector<std::string> texts;

	/** ID of the event that activated this message */
	extern unsigned int owner_id;

	/**
	 * Returns name of file that contains the face.
	 *
	 * @return FaceSet file
	 */
	std::string GetFaceName();

	/**
	 * Set FaceSet graphic file containing the face.
	 *
	 * @param face FaceSet file
	 */
	void SetFaceName(const std::string& face);

	/**
	 * Gets index of the face to display.
	 *
	 * @return face index
	 */
	int GetFaceIndex();

	/**
	 * Sets index of the face to display
	 *
	 * @param index face index
	 */
	void SetFaceIndex(int index);

	/** 
	 * Whether to mirror the face. 
	 *
	 * @return true: flipped, false: normal
	 */
	bool IsFaceFlipped();

	/**
	 * Sets whether to mirror the face.
	 *
	 * @param flipped Enable/Disable mirroring
	 */
	void SetFaceFlipped(bool flipped);

	/**
	 * If the face shall be placed right.
	 *
	 * @return true: right side, false: left side
	 */
	bool IsFaceRightPosition();

	/**
	 * Sets the face position.
	 *
	 * @param right true: right side, false: left side
	 */
	void SetFaceRightPosition(bool right);

	/**
	 * Gets if the message background is transparent.
	 *
	 * @return message transparent
	 */
	bool IsTransparent();

	/**
	 * Sets message box background state
	 *
	 * @param transparent true: transparent, false: opaque
	 */
	void SetTransparent(bool transparent);


	/**
	 * Gets the message box position.
	 *
	 * @return 0: top, 1: middle, 2: bottom
	 */
	int GetPosition();

	/**
	 * Sets the message box position.
	 * Depending on the player position this value is ignored to prevent overlap.
	 * (see SetPositionFixed)
	 *
	 * @param new_position 0: top, 1: middle, 2: bottom
	 */
	void SetPosition(int new_position);

	/**
	 * Gets whether message box position is fixed.
	 * In that case the hero can be obstructed.
	 *
	 * @return fixed
	 */
	bool IsPositionFixed();

	/**
	 * Sets if message box is moved to avoid obscuring the player.
	 *
	 * @param fixed position fixed
	 */
	void SetPositionFixed(bool fixed);

	/**
	 * Gets if parallel events continue while message box is displayed.
	 *
	 * @return whether events continue
	 */
	bool GetContinueEvents();

	/**
	 * Sets if parallel events continue while message box is displayed.
	 *
	 * @param continue_events continue events
	 */
	void SetContinueEvents(bool continue_events);

	/**
	 * Determines the position of the message box respecting the player's map
	 * position and if obstructing is allowed.
	 *
	 * @return Best message box position
	 */
	int GetRealPosition();

	/**
	 * Number of lines before the start
	 * of selection options.
	 * +-----------------------------------+
	 * |	Hi, hero, What's your name?    |
	 * |- Alex                             |
	 * |- Brian                            |
	 * |- Carol                            |
	 * +-----------------------------------+
	 * In this case, choice_start would be 1.
	 * Same with num_input_start.
	 */
	extern int choice_start;
	extern int num_input_start;

	/** Number of choices */
	extern int choice_max;

	/**
	 * Disabled choices:
	 * choice_disabled is true if choice is disabled (zero-based).
	 */
	extern std::bitset<8> choice_disabled;

	/** Option to choose if cancel. */
	extern int choice_cancel_type;

	extern int num_input_variable_id;
	extern int num_input_digits_max;

	/** Don't wait for a key to be pressed. */
	extern bool dont_halt;
	/** If a message is currently being processed. */
	extern bool message_waiting;
	extern bool visible;

	/** Selected option (4 => cancel). */
	extern int choice_result;
}

#endif
