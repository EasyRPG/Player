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

#ifndef EP_GAME_MESSAGE_H
#define EP_GAME_MESSAGE_H

#include <vector>
#include <bitset>
#include <string>
#include <functional>

namespace Game_Message {

	static const int MAX_LINE = 4;

	class PendingMessage {
		public:
			using ChoiceContinuation = std::function<void(int)>;

			int PushLine(std::string msg);
			int PushChoice(std::string msg, bool enabled = true);
			int PushNumInput(int variable_id, int num_digits);
			void PushPageEnd();

			void SetWordWrapped(bool value);
			void SetChoiceCancelType(int value);
			void SetChoiceContinuation(ChoiceContinuation f);
			void SetShowGoldWindow(bool value) { show_gold_window = true; }

			const std::string& GetText() const { return text; }

			int NumLines() const { return num_lines; }
			bool IsWordWrapped() const { return word_wrapped; }
			bool ShowGoldWindow() const { return show_gold_window; }

			bool HasChoices() const { return choice_start >= 0; }
			int GetChoiceStartLine() const { return choice_start; }
			int GetNumChoices() const { return HasChoices() ? NumLines() - choice_start : 0; }
			int GetChoiceCancelType() const { return choice_cancel_type; }
			bool IsChoiceEnabled(int idx) const { return choice_enabled[idx]; }
			const ChoiceContinuation& GetChoiceContinuation() const { return choice_continuation; }

			bool HasNumberInput() const { return num_input_digits > 0; }
			int GetNumberInputDigits() const { return num_input_digits; }
			int GetNumberInputVariable() const { return num_input_variable; }
			int GetNumberInputStartLine() const { return NumLines(); }
		private:
			int PushLineImpl(std::string msg);
		private:
			ChoiceContinuation choice_continuation;
			std::string text;
			int num_lines = 0;
			int choice_start = -1;
			int choice_cancel_type = 5;
			int num_input_variable = 0;
			int num_input_digits = 0;
			std::bitset<8> choice_enabled = {};
			bool word_wrapped = false;
			bool show_gold_window = false;
	};

	void Init();

	/**
	 * Reset the face graphic.
	 */
	void ClearFace();

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

	void SetPendingMessage(PendingMessage&& pm);

	const PendingMessage& GetPendingMessage();

	void ResetPendingMessage();

	/**
	 * Breaks the line into lines, each of which is equal
	 * or less than a specified limit in pixels in the
	 * given font (except in cases when breaking by spaces
	 * can't produce a short line), and calls the callback
	 * for each resulting line.
	 * 
	 * Font::Default() will be used to determine the word breaking.
	 * The caller is responsible for ensuring that Font::Default()
	 * either does not change between calling this function and
	 * displaying the results, or at least that the changed font
	 * has same metrics as the font used to calculate the line sizes.
	 *
	 * @param[in] line The line that will be broken into lines
	 * and added into the lines vector.
	 * @param[in] limit maximum size of each line after word-breaking.
	 * @param callback a function to be called for each word-wrapped line
	 */
	int WordWrap(const std::string& line, int limit, const std::function<void(const std::string &line)> callback);

	struct ParseParamResult {
		int value;
		const char* iter;
	};

	// EasyRPG extension allowing more recursive variables \v[\v[...]]
	static constexpr int easyrpg_default_max_recursion = 8;
	// RPG_RT only allows 1 level of recursion.
	static constexpr int rpg_rt_default_max_recursion = 1;
	// Which one we'll use by default.
	static constexpr int default_max_recursion = easyrpg_default_max_recursion;

	ParseParamResult ParseVariable(const char* iter, const char* end, uint32_t escape_char, bool skip_prefix = false, int max_recursion = default_max_recursion);
	ParseParamResult ParseColor(const char* iter, const char* end, uint32_t escape_char, bool skip_prefix = false, int max_recursion = default_max_recursion);
	ParseParamResult ParseSpeed(const char* iter, const char* end, uint32_t escape_char, bool skip_prefix = false, int max_recursion = default_max_recursion);
	ParseParamResult ParseActor(const char* iter, const char* end, uint32_t escape_char, bool skip_prefix = false, int max_recursion = default_max_recursion);

	void ApplyTextInsertingCommands(std::string& output, const std::string& input, uint32_t escape_char);

	/** If we're waiting for a message to finish processing. This flag is set to true from when the
	 * message box is requested up until it's finished writing text and ready to close.
	 */
	extern bool message_waiting;
	/** Set to true when after the message box has started animating closed */
	extern bool closing;
	/** Set to true while the message box is visible on the screen */
	extern bool visible;

	/**
	 * Return if it's legal to show a new message box.
	 *
	 * @param foreground true if this is in the foreground context, otherwise parallel context.
	 * @return true if we can show a message box.
	 */
	bool CanShowMessage(bool foreground);
}


#endif
