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

#ifndef _WINDOW_MESSAGE_H_
#define _WINDOW_MESSAGE_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>
#include "window_selectable.h"

////////////////////////////////////////////////////////////
/// Window Message Class.
/// This class displays the message boxes from
/// ShowMessageBox command code.
////////////////////////////////////////////////////////////
class Window_Message: public Window_Selectable {
public:
	Window_Message(int ix, int iy, int iwidth, int iheight);
	~Window_Message();

	////////////////////////////////////////////////////////
	/// Starts message processing by reading all non-
	/// displayed from Game_Message
	////////////////////////////////////////////////////////
	void StartMessageProcessing();

	////////////////////////////////////////////////////////
	/// Ends the message processing.
	////////////////////////////////////////////////////////
	void FinishMessageProcessing();

	////////////////////////////////////////////////////////
	/// Clears the Messagebox and places the write pointer
	/// in the top left corner.
	////////////////////////////////////////////////////////
	void InsertNewPage();

	////////////////////////////////////////////////////////
	/// Inserts a line break.
	////////////////////////////////////////////////////////
	void InsertNewLine();

	////////////////////////////////////////////////////////
	/// Closes the Messagebox and clears the waiting-flag
	/// (Allows the interpreter to continue)
	////////////////////////////////////////////////////////
	void TerminateMessage();

	////////////////////////////////////////////////////////
	/// Checks if the Text Output shall start.
	/// @return If the text output can start.
	////////////////////////////////////////////////////////
	bool IsTextOutputPossible();

	////////////////////////////////////////////////////////
	/// Stub.
	////////////////////////////////////////////////////////
	void ResetWindow();

	void Update();

	////////////////////////////////////////////////////////
	/// Continues outputting more text. Also handles the
	/// CommandCode parsing.
	////////////////////////////////////////////////////////
	void UpdateMessage();

	////////////////////////////////////////////////////////
	/// Parses the parameter part of a \-message-command.
	/// It starts parsing after the [ and stops after
	/// encountering ], a non-number or a line break.
	/// @param is_valid : Contains if a number was read
	/// @param call_depth: How many ] to skip, used for
	/// chained commands
	/// @return the read number
	////////////////////////////////////////////////////////
	int ParseParameter(bool& is_valid, int call_depth = 1);

	////////////////////////////////////////////////////////
	/// Parses a message command code (\ followed by a char).
	/// This should only be used for codes that accept
	/// parameters!
	/// The text_index must be on the char following the \
	/// when calling.
	/// @param call_depth : Directly passed to ParseParameter
	/// and automatically increased by 1 in every recursion.
	/// @return The final text output of the code.
	////////////////////////////////////////////////////////
	std::string ParseCommandCode(int call_depth = 1);

	////////////////////////////////////////////////////////
	/// Stub. For Choice.
	////////////////////////////////////////////////////////
	void UpdateCursorRect();

	////////////////////////////////////////////////////////
	/// Waits for a key press before the text output
	/// continutes.
	////////////////////////////////////////////////////////
	void WaitForInput();

protected:
	/// X-position of next char
	int contents_x;
	/// Y-position of next char
	int contents_y;
	/// Current number of lines on this page
	int line_count;
	//bool contents_showing;
	//int cursor_width;
	/// Index of the next char in text that will be outputted
	int text_index;
	/// text message that will be displayed
	std::string text;
	/// Prevents new page call when a halt \! was found
	bool halt_output;
	//Window_InputNumber* input_number_window; // TODO: Implement Window_InputNumber
	//Window_Gold* gold_window; // TODO: Implement Window_Gold
};

#endif
