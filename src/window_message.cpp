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
		text.append("\n");
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
	if (!Game_Message::face_name.empty()) {
		// 8 + 48 + 16 (left margin + face size + right margin)
		// ToDo: All these magic numbers should be in an enum in Window
		if (Game_Message::face_left_position) {
			contents_x = 8 + 48 + 16;
			printf("%d", Game_Message::face_flipped);
			DrawFace(Game_Message::face_name, Game_Message::face_index, 8, 6, Game_Message::face_flipped);
		} else {
			contents_x = 0;
			printf("%d", Game_Message::face_flipped);
			DrawFace(Game_Message::face_name, Game_Message::face_index, 248, 6, Game_Message::face_flipped);
		}
	} else {
		contents_x = 0;
	}
	contents_y = 0;
	line_count = 0;
	contents->GetFont()->color = Font::ColorDefault;
}

////////////////////////////////////////////////////////////
void Window_Message::InsertNewLine() {
	if (!Game_Message::face_name.empty() && Game_Message::face_left_position) {
		contents_x = 8 + 48 + 16;
	} else {
		contents_x = 0;
	}
	contents_y += 16;
	++line_count;
}

////////////////////////////////////////////////////////////
void Window_Message::TerminateMessage() {
	pause = false;
	Game_Message::message_waiting = false;
	// Only remove the texts here, the other flag resetting is done in
	// Game_Interpreter::CommandEnd
	Game_Message::texts.clear();
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
	if (visible && !Game_Message::visible) {
		// The Event Page ended but the MsgBox was used in this Event
		// It can be closed now.
		TerminateMessage();
		SetCloseAnimation(5);
		// Remove this when the Close Animation is implemented
		// The close animation must set the visible false flag
		visible = false;
	}
	else if (pause) {
		WaitForInput();
	}
	else if (!text.empty()) {
		// Output the remaining text for the current page
		UpdateMessage();
	}
	else if (IsTextOutputPossible()) {
		// Output a new page
		StartMessageProcessing();
		//printf("Text: %s\n", text.c_str());
		if (!visible) {
			// The MessageBox is not open yet but text output is needed
			SetOpenAnimation(5);
			visible = true;
		}
		Game_Message::visible = true;
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
	static bool kill_message = false;
	int loop_count = 0;
	int loop_max = 3;
	while (instant_speed || loop_count < loop_max) {
		++loop_count;
		++text_index;
		if ((unsigned)text_index == text.size()) {
			FinishMessageProcessing();
			text_index = -1;
			pause = !kill_message;
			if (kill_message) {
				TerminateMessage();
				kill_message = false;
			}
			break;
		}

		if (text[text_index] == '\n') {
			if (instant_speed) {
				// Special case: When there is not a \< anywhere in the
				// current message only the current line is displayed instant
				bool instant_speed_stop_found = false;
				for (unsigned i = text_index + 1; i < text.size() - 2; ++i) {
					if (text[i] == '\\' && text[i + 1] == '<') {
						instant_speed_stop_found = true;
						break;
					}
				}
				if (!instant_speed_stop_found) {
					instant_speed = false;
				}
			}
			InsertNewLine();
		} else if (text[text_index] == '\\' && (unsigned)text_index != text.size() - 1) {
			// Special message codes
			++text_index;
			std::string command_result;
			switch (text[text_index]) {
			case 'c':
			case 'C':
			case 'n':
			case 'N':
			case 's':
			case 'S':
			case 'v':
			case 'V':
				// These commands support indirect access via \v[]
				command_result = ParseCommandCode();
				contents->TextDraw(contents_x, contents_y, command_result);
				contents_x += contents->GetTextSize(command_result).width;
				break;
			case '\\':
				// Show Backslash
				contents->TextDraw(contents_x, contents_y, std::string("\\"));
				contents_x += contents->GetTextSize("\\").width;
				break;
			case '_':
				// Insert half size space
				contents_x += contents->GetTextSize(" ").width / 2;
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
				// The close happens at the end of the message, not where
				// the ^ is encoutered
				kill_message = true;
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
			// Normal Text
			contents->TextDraw(contents_x, contents_y, text.substr(text_index, 1));
			contents_x += 6;
		}
	}
	loop_count = 0;
}

////////////////////////////////////////////////////////////
int Window_Message::ParseParameter(bool& is_valid, int call_depth) {
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
		} else if (text[text_index] == '\n') {
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
			--call_depth;
			if (call_depth == 0) {
				break;
			}
		} else {
			// End of number
			// Search for ] or line break
			while ((unsigned)text_index != text.size()) {
					if (text[text_index] == '\n') {
						--text_index;
						break;
					} else if (text[text_index] == ']') {
						--call_depth;
						if (call_depth == 0) {
							break;
						}
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
	return num;
}

////////////////////////////////////////////////////////////
std::string Window_Message::ParseCommandCode(int call_depth) {
	int parameter;
	bool is_valid;
	// sub_code is used by chained arguments like \v[\v[1]]
	// In that case sub_code contains the result from \v[1]
	int sub_code = -1;
	char cmd_char = text[text_index];
	if ((unsigned)text_index + 3 < text.size() &&
		text[text_index + 2] == '\\' &&
		(text[text_index + 3] == 'v' ||
		 text[text_index + 3] == 'V')) {
		text_index += 3;
		// The result is a variable value, str-to-int is safe in this case
		sub_code = atoi(ParseCommandCode(++call_depth).c_str());
	}
	switch (cmd_char) {
	case 'c':
	case 'C':
		// Color
		if (sub_code >= 0) {
			parameter = sub_code;
		} else {
			parameter = ParseParameter(is_valid, call_depth);
		}
		contents->GetFont()->color = parameter > 19 ? 0 : parameter;
		break;
	case 'n':
	case 'N':
		// Output Hero name
		if (sub_code >= 0) {
			is_valid = true;
			parameter = sub_code;
		} else {
			parameter = ParseParameter(is_valid, call_depth);
		}
		if (is_valid) {
			Game_Actor* actor = NULL;
			if (parameter == 0) {
				// Party hero
				actor = Game_Party::GetActors()[0];
			} else {
				actor = Game_Actors::GetActor(parameter);
			}
			if (actor != NULL) {
				return actor->GetName();
			}
		} else {
			Output::Warning("Invalid argument for \\n-Command");
		}
		break;
	case 's':
	case 'S':
		// Speed modifier
		// ToDo: Find out how long each \s take
		if (sub_code >= 0) {
			is_valid = true;
			parameter = sub_code;
		} else {
			parameter = ParseParameter(is_valid, call_depth);
		}
		break;
	case 'v':
	case 'V':
		// Show Variable value
		if (sub_code >= 0) {
			is_valid = true;
			parameter = sub_code;
		} else {
			parameter = ParseParameter(is_valid, call_depth);
		}
		if (is_valid && Game_Variables.isValidVar(parameter)) {
			std::stringstream ss;
			ss << Game_Variables[parameter];
			return ss.str();
		} else {
			// Invalid Var is always 0
			std::stringstream ss;
			ss << '0';
			return ss.str();
		}
	default:;
		// When this happens text_index was not on a \ during calling
	}

	return "";
}

////////////////////////////////////////////////////////////
void Window_Message::UpdateCursorRect() {

}

////////////////////////////////////////////////////////////
void Window_Message::WaitForInput() {
	active = true;
	if (Input::IsTriggered(Input::DECISION) ||
		Input::IsTriggered(Input::CANCEL)) {
		active = false;
		pause = false;
		if (halt_output) {
			halt_output = false;
		}/* else {
			InsertNewPage();
		}*/
		if (text.empty()) {
			TerminateMessage();
		}
	}
}
