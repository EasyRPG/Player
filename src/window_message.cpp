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
#include <sstream>
#include "window_message.h"
#include "game_actors.h"
#include "game_message.h"
#include "game_party.h"
#include "game_variables.h"
#include "graphics.h"
#include "input.h"
#include "util_macro.h"

////////////////////////////////////////////////////////////
Window_Message::Window_Message(int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(ix, iy, iwidth, iheight),
	contents_x(0), contents_y(0), line_count(0), text_index(-1), text(""),
	halt_output(false)
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
	contents->GetFont()->color = Font::ColorDefault;
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

	// Contains at what frame the sleep is over
	static int sleep_until = -1;
	if (sleep_until > -1) {
		if (Graphics::GetFrameCount() >= sleep_until) {
			// Sleep over
			sleep_until = -1;
		} else {
			return;
		}
	}

	bool instant_speed = false;
	int loop_count = 0;
	int loop_max = 3;
	while (instant_speed || loop_count < loop_max) {
		++loop_count;
		++text_index;
		if ((unsigned)text_index == text.size()) {
			FinishMessageProcessing();
			text.clear();
			text_index = -1;
			pause = true;
			break;
		}

		if (text[text_index] == '\n') {
			instant_speed = false;
			InsertNewLine();
		} else if (text[text_index] == '\f') {
			// \f is a form feed (page break)
			instant_speed = false;
			pause = true;
			break;
		} else if (text[text_index] == '\\' && (unsigned)text_index != text.size() - 1) {
			// Special message codes
			++text_index;
			int parameter;
			bool is_valid;
			switch (text[text_index]) {
			case 'c':
				// Color
				parameter = ParseParameter(is_valid);
				if (is_valid) {
					contents->GetFont()->color = parameter > 19 ? 0 : parameter;
				} else {
					contents->GetFont()->color = Font::ColorDefault;
				}
				break;
			case 'n':
				// Output Hero name
				parameter = ParseParameter(is_valid);
				if (is_valid) {
					Game_Actor* actor = NULL;
					if (parameter == 0) {
						// Party hero
						actor = Game_Party::GetActors()[0];
					} else {
						actor = Game_Actors::GetActor(parameter);
					}
					if (actor != NULL) {
						contents->TextDraw(contents_x, contents_y, actor->GetName());
						contents_x += contents->GetTextSize(actor->GetName()).width;
					}
				}
				break;
			case 's':
				// Speed modifier
				// ToDo: Find out how long each \s take
				parameter = ParseParameter(is_valid);
				break;
			case 'v':
				// Show Variable value
				parameter = ParseParameter(is_valid);
				if (is_valid && Game_Variables.isValidVar(parameter)) {
					std::stringstream ss;
					ss << Game_Variables[parameter];
					contents->TextDraw(contents_x, contents_y, ss.str());
					contents_x += contents->GetTextSize(ss.str()).width;
				} else {
					// Invalid Var is always 0
					std::stringstream ss;
					ss << '0';
					contents->TextDraw(contents_x, contents_y, ss.str());
					contents_x += contents->GetTextSize(ss.str()).width;
				}
				break;
			case '\\':
				// Show Backslash
				contents->TextDraw(contents_x, contents_y, std::string("\\"));
				break;
			case '$':
				// Show Money Window ToDo
				break;
			case '!':
				// Text pause
				halt_output = true;
				pause = true;
				break;
			case '^':
				// Force message close
				TerminateMessage();
				break;
			case '>':
				// Instant speed start
				// ToDo: Not working properly
				instant_speed = true;
				break;
			case '<':
				// Instant speed stop
				instant_speed = false;
				break;
			case '.':
				// Millisecond sleep
				sleep_until = Graphics::GetFrameCount() + 60 / 4;
				return;
				break;
			case '|':
				// Second sleep
				sleep_until = Graphics::GetFrameCount() + 60;
				return;
				break;
			default:;
			}
		} else if (text[text_index] == '$' &&
			(unsigned)text_index != text.size() - 1 &&
			((text[text_index+1] >= 'a' && text[text_index+1] <= 'z') ||
			(text[text_index+1] >= 'A' && text[text_index+1] <= 'Z'))) {
			// ExFont
			contents->TextDraw(contents_x, contents_y, text.substr(text_index, 2));
			contents_x += 12;
			++text_index;
		} else {
			// Normal Text Draw
			contents->TextDraw(contents_x, contents_y, text.substr(text_index, 1));
			contents_x += 6;
		}
	}
	loop_count = 0;
}

////////////////////////////////////////////////////////////
int Window_Message::ParseParameter(bool& is_valid) {
	++text_index;

	if ((unsigned)text_index == text.size() ||
		text[text_index] != '[') {
		--text_index;
		is_valid = false;
		return 0;
	}

	++text_index; // Skip the [

	bool null_at_start = false;
	std::stringstream ss;
	for (;;) {
		if ((unsigned)text_index == text.size()) {
			break;
		} else if (text[text_index] == '\n' ||
			text[text_index] == '\f') {
			--text_index;
			break;
		}
		else if (text[text_index] == '0') {
			// Truncate 0 at the start
			if (!ss.str().empty()) {
				ss << '0';
			} else {
				null_at_start = true;
			}
		}
		else if (text[text_index] >= '1' &&
			text[text_index] <= '9') {
			ss << text[text_index];
		} else if (text[text_index] == ']') {
			break;
		} else {
			// End of number
			// Search for ] or line break
			while ((unsigned)text_index != text.size()) {
					if (text[text_index] == '\n' ||
						text[text_index] == '\f') {
						--text_index;
						break;
					} else if (text[text_index] == ']') {
						break;
					}
					++text_index;
			}
			break;
		}
		++text_index;
	}

	if (ss.str().empty()) {
		if (null_at_start) {
			ss << "0";
		} else {
			is_valid = false;
			return 0;
		}
	}

	int num;
	ss >> num;
	is_valid = true;
	printf("Parsed %d\n", num);
	return num;
}

////////////////////////////////////////////////////////////
void Window_Message::UpdateCursorRect() {

}

////////////////////////////////////////////////////////////
void Window_Message::WaitForInput() {
	if (Input::IsTriggered(Input::DECISION) ||
		Input::IsTriggered(Input::CANCEL)) {
		pause = false;
		if (halt_output) {
			halt_output = false;
		} else {
			InsertNewPage();
		}
		if (text.empty()) {
			TerminateMessage();
		}
	}
}
