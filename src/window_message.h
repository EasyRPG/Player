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

#ifndef _WINDOW_MESSAGE_H_
#define _WINDOW_MESSAGE_H_

// Headers
#include <string>
#include "window_gold.h"
#include "window_numberinput.h"
#include "window_selectable.h"

/**
 * Window Message Class.
 * This class displays the message boxes from
 * ShowMessageBox command code.
 */
class Window_Message: public Window_Selectable {
public:
	Window_Message(int ix, int iy, int iwidth, int iheight);
	~Window_Message() override;

	enum WindowMessageValues {
		LeftMargin = 8,
		FaceSize = 48,
		RightFaceMargin = 16,
		TopMargin = 8
	};

	/**
	 * Starts message processing by reading all
	 * non-displayed from Game_Message.
	 */
	void StartMessageProcessing();

	/**
	 * Ends the message processing.
	 */
	void FinishMessageProcessing();

	/**
	 * Does the initial steps to start a choice selection.
	 */
	void StartChoiceProcessing();

	/**
	 * Does the initial steps to start a number input.
	 */
	void StartNumberInputProcessing();

	/**
	 * Shows the Gold Window 
	 */
	void ShowGoldWindow();

	/**
	 * Clears the Messagebox and places the write pointer
	 * in the top left corner.
	 */
	void InsertNewPage();

	/**
	 * Inserts a line break.
	 */
	void InsertNewLine();

	/**
	 * Closes the Messagebox and clears the waiting-flag
	 * (allows the interpreter to continue).
	 */
	void TerminateMessage();

	/**
	 * Checks if the next message page can be displayed.
	 *
	 * @return If the text output can start.
	 */
	bool IsNextMessagePossible();

	/**
	 * Stub.
	 */
	void ResetWindow();

	void Update() override;

	/**
	 * Continues outputting more text. Also handles the
	 * CommandCode parsing.
	 */
	virtual void UpdateMessage();

	/**
	 * Parses the parameter part of a \-message-command.
	 * It starts parsing after the [ and stops after
	 * encountering ], a non-number or a line break.
	 *
	 * @param is_valid contains if a number was read
	 * @return the read number.
	 */
	int ParseParameter(bool& is_valid);

	/**
	 * Parses a message command code (\ followed by a char).
	 * This should only be used for codes that accept
	 * parameters!
	 * The text_index must be on the char following \ when
	 * calling.
	 *
	 * @param success Parsing was successful.
	 * @return the final text output of the code.
	 */
	std::string ParseCommandCode(bool& success);

	/**
	 * Stub. For choice.
	 */
	void UpdateCursorRect() override;

	/**
	 * Waits for a key press before the text output
	 * continutes.
	 */
	void WaitForInput();

	/**
	 * Stub. Handles choice selection.
	 */
	void InputChoice();

	/**
	 * Handles number input.
	 */
	void InputNumber();

protected:
	/** X-position of next char. */
	int contents_x;
	/** Y-position of next char. */
	int contents_y;
	/** Current number of lines on this page. */
	int line_count;
	/** Index of the next char in text that will be output. */
	std::u32string::iterator text_index, end;
	/** text message that will be displayed. */
	std::u32string text;
	/** Used by Message kill command \^. */
	bool kill_message;
	/** Text color. */
	int text_color;
	/** Current speed modifier. */
	int speed_modifier;
	/** Counts the frames since the last char rendering. */
	int speed_frame_counter;
	/** If true inserts a new page after pause ended */
	bool new_page_after_pause;

	/** Character used before message commands. */
	uint32_t escape_char;

	/**
	 * Table contains how many frames drawing one single char takes.
	 * 0 means: 2 chars per frame.
	 */
	static const int speed_table[21];

	/** Used by the number input event. */
	std::unique_ptr<Window_NumberInput> number_input_window;
	std::unique_ptr<Window_Gold> gold_window;
};

#endif
