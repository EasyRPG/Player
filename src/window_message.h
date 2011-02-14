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
#include "window_gold.h"
#include "window_numberinput.h"
#include "window_selectable.h"

#ifdef DINGOO
#define NO_WCHAR
#endif

#ifdef NO_WCHAR
// This is a workaround if your system has no wchar
#undef wstring
#define wstring string
#define wstringstream stringstream
#define utf(x) x
#else
#define utf(x) L##x
#endif

////////////////////////////////////////////////////////////
/// Window Message Class.
/// This class displays the message boxes from
/// ShowMessageBox command code.
////////////////////////////////////////////////////////////
class Window_Message: public Window_Selectable {
public:
	Window_Message(int ix, int iy, int iwidth, int iheight);
	~Window_Message();

	enum WindowMessageValues {
		LeftMargin = 8,
		FaceSize = 48,
		RightFaceMargin = 16,
		TopMargin = 6
	};

	/*
	Notes about MessageBox Open/Close:
	RPG2k does open a MessageBox when the first ShowMsgBox-Event
	on an event page occurs (or when there is a MsgBox event anywhere?).
	The Box is then open until the whole event page ended
	(Interpreter::CommandEnd).
	The following behaviour is not implemented yet:
	The MsgBox must stay open when the event following the current event
	is an auto start (or parallel process?)-event. RPG2k scans the whole event
	for a MsgBox call and keeps the MsgBox open when it finds one.
	*/

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
	/// Does the initial steps to start a Choice selection.
	////////////////////////////////////////////////////////
	void StartChoiceProcessing();

	////////////////////////////////////////////////////////
	/// Does the initial steps to start a number input.
	////////////////////////////////////////////////////////
	void StartNumberInputProcessing();

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
	/// Checks if the next message page can be displayed.
	/// @return If the text output can start.
	////////////////////////////////////////////////////////
	bool IsNextMessagePossible();

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
	/// The text_index must be on the char following \ when
	/// calling.
	/// @param call_depth : Directly passed to ParseParameter
	/// and automatically increased by 1 in every recursion.
	/// @return The final text output of the code.
	////////////////////////////////////////////////////////
	std::wstring ParseCommandCode(int call_depth = 1);

	////////////////////////////////////////////////////////
	/// Stub. For Choice.
	////////////////////////////////////////////////////////
	void UpdateCursorRect();

	////////////////////////////////////////////////////////
	/// Waits for a key press before the text output
	/// continutes.
	////////////////////////////////////////////////////////
	void WaitForInput();

	////////////////////////////////////////////////////////
	/// Stub. Handles Choice Selection.
	////////////////////////////////////////////////////////
	void InputChoice();

	////////////////////////////////////////////////////////
	/// Handles number input.
	////////////////////////////////////////////////////////
	void InputNumber();

protected:
	/// X-position of next char
	int contents_x;
	/// Y-position of next char
	int contents_y;
	/// Current number of lines on this page
	int line_count;
	/// Index of the next char in text that will be outputted
	int text_index;
	/// text message that will be displayed
	std::wstring text;
	/// Used by Message kill command \^
	bool kill_message;
	/// Prevents new page call when a halt \! was found
	bool halt_output;
	/// Text color
	int text_color;
	
	/// Used by the number input event
	Window_NumberInput* number_input_window;
	Window_Gold* gold_window;
};

#endif
