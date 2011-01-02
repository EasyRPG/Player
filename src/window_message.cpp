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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "window_message.h"
#include "game_message.h"
#include "input.h"

////////////////////////////////////////////////////////////
Window_Message::Window_Message(int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(ix, iy, iwidth, iheight),
	contents_x(0), contents_y(0), line_count(0), current_char(-1), text("")
{
	SetContents(Bitmap::CreateBitmap(width - 16, height - 16));
	contents->SetTransparentColor(windowskin->GetTransparentColor());

	visible = false;
	z = 9998;
	//contents_showing = false;
	//cursor_width = 0;
	active = false;
	index = -1;
}

////////////////////////////////////////////////////////////
Window_Message::~Window_Message() {
	TerminateMessage();
	//Game_Temp::message_window_showing = false;
	/*if (input_number_window != NULL) {
		input_number_window.dispose
	}*/
}

////////////////////////////////////////////////////////////
void Window_Message::StartMessageProcessing() {
	contents->Clear();
	text.clear();
	for (size_t i = 0; i < Game_Message::texts.size(); ++i) {
		std::string line = Game_Message::texts[i];
		if (line.length() > 50) {
			line.resize(50);
		}
		text.append(line);
		if (text[text.size() - 1] != '\f') {
			// Prevent line break on next page when there is a form feed
			// A form feed happens when there were two or more ShowMessage
			// Events in a row
			text.append("\n");
		}
	}
	InsertNewPage();
}

////////////////////////////////////////////////////////////
void Window_Message::FinishMessageProcessing() {
	text.clear();
}

////////////////////////////////////////////////////////////
void Window_Message::InsertNewPage() {
	contents->Clear();
	contents_x = 0;
	contents_y = 0;
	line_count = 0;
}

////////////////////////////////////////////////////////////
void Window_Message::InsertNewLine() {
	contents_x = 0;
	contents_y += 16;
	++line_count;
}

////////////////////////////////////////////////////////////
void Window_Message::TerminateMessage() {
	pause = false;
	Game_Message::message_waiting = false;
	Game_Message::Clear();
	SetCloseAnimation(5);
	visible = false;
}

////////////////////////////////////////////////////////////
bool Window_Message::IsTextOutputPossible() {
	if (Game_Message::texts.empty()) {
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////
void Window_Message::ResetWindow() {

}

////////////////////////////////////////////////////////////
void Window_Message::Update() {
	Window::Update();

	if (pause) {
		WaitForInput();
	}
	else if (!text.empty()) {
		UpdateMessage();
	}
	else if (IsTextOutputPossible()) {
		StartMessageProcessing();
		Game_Message::visible = true;
		SetOpenAnimation(5);
		visible = true;
	} else {
		
	}
}

////////////////////////////////////////////////////////////
void Window_Message::UpdateMessage() {
	// Message Box Show Message rendering loop
	// At the moment 3 chars per frame are drawn
	// ToDo: Value must depend on Speedevent

	int loop_count = 0;
	while (loop_count != 3) {
		++loop_count;
		++current_char;
		if (current_char == text.size()) {
			FinishMessageProcessing();
			text.clear();
			current_char = -1;
			pause = true;
			break;
		}

		if (text[current_char] == '\n') {
			InsertNewLine();
		} else if (text[current_char] == '\f') {
			// \f is a form feed
			pause = true;
			break;
		} else if (text[current_char] == '\\' && current_char != text.size() - 1) {
			// Special message codes
			++current_char;
			switch (text[current_char]) {
			case '\\':
				contents->TextDraw(contents_x, contents_y, std::string("\\"));
				break;
			default:;
			}
		} else if (text[current_char] == '$' &&
			current_char != text.size() - 1 &&
			((text[current_char+1] >= 'a' && text[current_char+1] <= 'z') ||
			(text[current_char+1] >= 'A' && text[current_char+1] <= 'Z'))) {
			// ExFont
			contents->TextDraw(contents_x, contents_y, text.substr(current_char, 2));
			contents_x += 12;
			++current_char;
		} else {
			// Normal Text Draw
			contents->TextDraw(contents_x, contents_y, text.substr(current_char, 1));
			contents_x += 6;
		}
	}
	loop_count = 0;
	// ToDo: break on Instant Message Command code
}

////////////////////////////////////////////////////////////
void Window_Message::UpdateCursorRect() {

}

////////////////////////////////////////////////////////////
void Window_Message::WaitForInput() {
	if (Input::IsTriggered(Input::DECISION) ||
		Input::IsTriggered(Input::CANCEL)) {
		pause = false;
		InsertNewPage();
		if (text.empty()) {
			TerminateMessage();
		}
	}
}
