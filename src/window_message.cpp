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
#include "game_map.h"
#include "game_message.h"
#include "game_party.h"
#include "game_player.h"
#include "game_system.h"
#include "game_variables.h"
#include "graphics.h"
#include "input.h"
#include "player.h"
#include "util_macro.h"
#include "utils.h"


#ifdef NO_WCHAR
// This is a workaround if your system has no wchar
#undef wstring
#define wstring string
#endif

////////////////////////////////////////////////////////////
Window_Message::Window_Message(int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(ix, iy, iwidth, iheight),
	contents_x(0), contents_y(0), line_count(0), text_index(-1), text(utf("")),
	kill_message(false), halt_output(false), number_input_window(NULL)
{
	SetContents(Surface::CreateSurface(width - 16, height - 16));
	contents->SetTransparentColor(windowskin->GetTransparentColor());

	visible = false;
	SetZ(10000);
	//contents_showing = false;
	//cursor_width = 0;
	active = false;
	index = -1;
	text_color = Font::ColorDefault;

	number_input_window = new Window_NumberInput(0, 0);
	number_input_window->SetVisible(false);

	Game_Message::Init();
}

////////////////////////////////////////////////////////////
Window_Message::~Window_Message() {
	TerminateMessage();
	Game_Message::visible = false;
	//Game_Temp::message_window_showing = false;
	// The Windows are already deleted in Graphics during closing
	// But this probably memleaks during scene change?
	if (!Player::exit_flag) {
		delete number_input_window;
	}
}

////////////////////////////////////////////////////////////
void Window_Message::StartMessageProcessing() {
	contents->Clear();
	text.clear();
	for (size_t i = 0; i < Game_Message::texts.size(); ++i) {
		std::string line = Game_Message::texts[i];
#if defined(NO_WCHAR)
		text.append(line + "\n");
#else
		text.append(Utils::DecodeUTF(line + "\n"));
#endif
	}
	item_max = Game_Message::choice_max;

	InsertNewPage();
}

////////////////////////////////////////////////////////////
void Window_Message::FinishMessageProcessing() {
	if (Game_Message::choice_max > 0) {
		StartChoiceProcessing();
	} else if (Game_Message::num_input_variable_id > 0) {
		StartNumberInputProcessing();
	} else if (kill_message) {
		TerminateMessage();
		kill_message = false;
	} else {
		pause = true;
	}

	text_index = -1;
	text.clear();
}

////////////////////////////////////////////////////////////
void Window_Message::StartChoiceProcessing() {
	active = true;
	index = 0;
}

////////////////////////////////////////////////////////////
void Window_Message::StartNumberInputProcessing() {
	number_input_window->SetMaxDigits(Game_Message::num_input_digits_max);
	if (!Game_Message::face_name.empty() && Game_Message::face_left_position) {
		number_input_window->SetX(LeftMargin + FaceSize + RightFaceMargin);
	} else {
		number_input_window->SetX(x);
	}
	number_input_window->SetY(y + contents_y - 2);
	number_input_window->SetActive(true);
	number_input_window->SetVisible(true);
	number_input_window->Update();
}

////////////////////////////////////////////////////////////
void Window_Message::InsertNewPage() {
	contents->Clear();

	if (Game_Message::fixed_position) {
		y = Game_Message::position * 80;
	} else {
		// Move Message Box to prevent player hiding
		int disp = Main_Data::game_player->GetScreenY();

		switch (Game_Message::position) {
		case 0: // Up
			y = disp > (16 * 7) ? 0 : 2 * 80;
			break;
		case 1: // Center
			if (disp <= 16 * 7) {
				y = 2 * 80;
			} else if (disp >= 16 * 10) {
				y = 0;
			} else {
				y = 80;
			}
			break;
		case 2: // Down
			y = disp >= (16 * 10) ? 0 : 2 * 80;
			break;
		};
	}
	

	if (Game_Message::background) {
		opacity = 255;
	} else {
		opacity = 0;
	}

	if (!Game_Message::face_name.empty()) {
		if (Game_Message::face_left_position) {
			contents_x = LeftMargin + FaceSize + RightFaceMargin;
			DrawFace(Game_Message::face_name, Game_Message::face_index, LeftMargin, TopMargin, Game_Message::face_flipped);
		} else {
			contents_x = 0;
			DrawFace(Game_Message::face_name, Game_Message::face_index, 248, TopMargin, Game_Message::face_flipped);
		}
	} else {
		contents_x = 0;
	}

	if (Game_Message::choice_start == 0 && Game_Message::choice_max > 0) {
		contents_x += 12;
	}

	if (Game_Message::num_input_start == 0 && Game_Message::num_input_variable_id > 0) {
		// If there is an input window on the first line
		StartNumberInputProcessing();
	}

	contents_y = 2;
	line_count = 0;
	text_color = Font::ColorDefault;
}

////////////////////////////////////////////////////////////
void Window_Message::InsertNewLine() {
	if (!Game_Message::face_name.empty() && Game_Message::face_left_position) {
		contents_x = LeftMargin + FaceSize + RightFaceMargin;
	} else {
		contents_x = 0;
	}

	contents_y += 16;
	++line_count;

	if (line_count >= Game_Message::choice_start && Game_Message::choice_max > 0) {
		// A choice resets the font color
		text_color = Font::ColorDefault;

		unsigned choice_index = line_count - Game_Message::choice_start;
		// Check for disabled choices
		if (Game_Message::choice_disabled.test(choice_index)) {
			text_color = Font::ColorDisabled;
		}

		contents_x += 12;
	}
}

////////////////////////////////////////////////////////////
void Window_Message::TerminateMessage() {
	active = false;
	pause = false;
	index = -1;

	Game_Message::message_waiting = false;
	number_input_window->SetActive(false);
	number_input_window->SetVisible(false);
	// The other flag resetting is done in Game_Interpreter::CommandEnd
	Game_Message::SemiClear();
}

////////////////////////////////////////////////////////////
bool Window_Message::IsNextMessagePossible() {
	if (Game_Message::num_input_variable_id > 0) {
		return true;
	}

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
	Window_Selectable::Update();
	number_input_window->Update();

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
	} else if (active) {
		InputChoice();
	} else if (number_input_window->GetVisible()) {
		InputNumber();
	} else if (!text.empty()) {
		// Output the remaining text for the current page
		UpdateMessage();
	}
	else if (IsNextMessagePossible()) {
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
	int loop_count = 0;
	int loop_max = 3;
	while (instant_speed || loop_count < loop_max) {
		++loop_count;
		++text_index;
		if ((unsigned)text_index == text.size()) {
			FinishMessageProcessing();
			break;
		}

		if (text[text_index] == utf('\n')) {
			instant_speed = false;
			InsertNewLine();
		} else if (text[text_index] == utf('\\') && (unsigned)text_index != text.size() - 1) {
			// Special message codes
			++text_index;

			std::wstring command_result;

			switch (text[text_index]) {
			case utf('c'):
			case utf('C'):
			case utf('n'):
			case utf('N'):
			case utf('s'):
			case utf('S'):
			case utf('v'):
			case utf('V'):
				// These commands support indirect access via \v[]
				command_result = ParseCommandCode();
				contents->TextDraw(contents_x, contents_y, text_color, command_result);
				contents_x += contents->Surface::GetTextSize(command_result).width;
				break;
			case utf('\\'):
				// Show Backslash
				contents->TextDraw(contents_x, contents_y, text_color, std::string("\\"));
				contents_x += contents->GetTextSize("\\").width;
				break;
			case utf('_'):
				// Insert half size space
				contents_x += contents->GetTextSize(" ").width / 2;
			case utf('$'):
				// Show Money Window ToDo
				break;
			case utf('!'):
				// Text pause
				halt_output = true;
				pause = true;
				break;
			case utf('^'):
				// Force message close
				// The close happens at the end of the message, not where
				// the ^ is encoutered
				kill_message = true;
				break;
			case utf('>'):
				// Instant speed start
				// ToDo: Not working properly
				instant_speed = true;
				break;
			case utf('<'):
				// Instant speed stop
				instant_speed = false;
				break;
			case utf('.'):
				// 1/4 second sleep
				sleep_until = Graphics::GetFrameCount() + 60 / 4;
				return;
				break;
			case utf('|'):
				// Second sleep
				sleep_until = Graphics::GetFrameCount() + 60;
				return;
				break;
			default:;
			}
		} else if (text[text_index] == utf('$') &&
			(unsigned)text_index != text.size() - 1 &&
			((text[text_index+1] >= utf('a') && text[text_index+1] <= utf('z')) ||
			(text[text_index+1] >= utf('A') && text[text_index+1] <= utf('Z')))) {
			// ExFont
			contents->TextDraw(contents_x, contents_y, text_color, text.substr(text_index, 2));
			contents_x += 12;
			++text_index;
		} else {
			// Normal Text
#ifdef NO_WCHAR
			int utfsize = Utils::GetUtf8ByteSize(text[text_index]);
			std::string glyph = text.substr(text_index, utfsize);
			text_index += utfsize - 1;
#else
			std::wstring glyph = text.substr(text_index, 1);
#endif

			contents->TextDraw(contents_x, contents_y, text_color, glyph);
			contents_x += contents->Surface::GetTextSize(glyph).width;
		}
	}
	loop_count = 0;
}

////////////////////////////////////////////////////////////
int Window_Message::ParseParameter(bool& is_valid, int call_depth) {
	++text_index;

	if ((unsigned)text_index == text.size() ||
		text[text_index] != utf('[')) {
		--text_index;
		is_valid = false;
		return 0;
	}

	++text_index; // Skip the [

	bool null_at_start = false;
	std::wstringstream ss;
	for (;;) {
		if ((unsigned)text_index == text.size()) {
			break;
		} else if (text[text_index] == utf('\n')) {
			--text_index;
			break;
		}
		else if (text[text_index] == utf('0')) {
			// Truncate 0 at the start
			if (!ss.str().empty()) {
				ss << '0';
			} else {
				null_at_start = true;
			}
		}
		else if (text[text_index] >= utf('1') &&
			text[text_index] <= utf('9')) {
			ss << text[text_index];
		} else if (text[text_index] == utf(']')) {
			--call_depth;
			if (call_depth == 0) {
				break;
			}
		} else {
			// End of number
			// Search for ] or line break
			while ((unsigned)text_index != text.size()) {
					if (text[text_index] == utf('\n')) {
						--text_index;
						break;
					} else if (text[text_index] == utf(']')) {
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
			ss << L"0";
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
std::wstring Window_Message::ParseCommandCode(int call_depth) {
	int parameter;
	bool is_valid;
	// sub_code is used by chained arguments like \v[\v[1]]
	// In that case sub_code contains the result from \v[1]
	int sub_code = -1;
	wchar_t cmd_char = text[text_index];
	if ((unsigned)text_index + 3 < text.size() &&
		text[text_index + 2] == utf('\\') &&
		(text[text_index + 3] == utf('v') ||
		 text[text_index + 3] == utf('V'))) {
		text_index += 3;
		// The result is an int value, str-to-int is safe in this case
		std::wstringstream ss;
		ss << ParseCommandCode(++call_depth).c_str();
		ss >> sub_code;
	}
	switch (cmd_char) {
	case utf('c'):
	case utf('C'):
		// Color
		if (sub_code >= 0) {
			parameter = sub_code;
		} else {
			parameter = ParseParameter(is_valid, call_depth);
		}
		text_color = parameter > 19 ? 0 : parameter;
		break;
	case utf('n'):
	case utf('N'):
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
#ifdef NO_WCHAR
				return actor->GetName();
#else
				return Utils::DecodeUTF(actor->GetName());
#endif
			}
		} else {
			Output::Warning("Invalid argument for \\n-Command");
		}
		break;
	case utf('s'):
	case utf('S'):
		// Speed modifier
		// ToDo: Find out how long each \s take
		if (sub_code >= 0) {
			is_valid = true;
			parameter = sub_code;
		} else {
			parameter = ParseParameter(is_valid, call_depth);
		}
		break;
	case utf('v'):
	case utf('V'):
		// Show Variable value
		if (sub_code >= 0) {
			is_valid = true;
			parameter = sub_code;
		} else {
			parameter = ParseParameter(is_valid, call_depth);
		}
		if (is_valid && Game_Variables.isValidVar(parameter)) {
			std::wstringstream ss;
			ss << Game_Variables[parameter];
			return ss.str();
		} else {
			// Invalid Var is always 0
			std::wstringstream ss;
			ss << utf('0');
			return ss.str();
		}
	default:;
		// When this happens text_index was not on a \ during calling
	}
	return utf("");
}

////////////////////////////////////////////////////////////
void Window_Message::UpdateCursorRect() {
	if (index >= 0) {
		int x_pos = 2;
		int y_pos = (Game_Message::choice_start + index) * 16;
		int width = contents->GetWidth();

		if (!Game_Message::face_name.empty()) {
			if (Game_Message::face_left_position) {
				x_pos += LeftMargin + FaceSize + RightFaceMargin;
			}
			width = width - LeftMargin - FaceSize - RightFaceMargin - 4;
		}

		cursor_rect.Set(x_pos, y_pos, width, 16);
	} else {
		cursor_rect.Set(0, 0, 0, 0);
	}
}

////////////////////////////////////////////////////////////
void Window_Message::WaitForInput() {
	active = true; // Enables the Pause arrow
	if (Input::IsTriggered(Input::DECISION) ||
		Input::IsTriggered(Input::CANCEL)) {
		active = false;
		pause = false;
		if (halt_output) {
			halt_output = false;
		}

		if (text.empty()) {
			TerminateMessage();
		}
	}
}

////////////////////////////////////////////////////////////
void Window_Message::InputChoice() {
	if (Input::IsTriggered(Input::CANCEL)) {
		if (Game_Message::choice_cancel_type > 0) {
			Game_System::SePlay(Data::system.cancel_se);
			Game_Message::choice_result = Game_Message::choice_cancel_type - 1; // Cancel
			TerminateMessage();
		}
	} else if (Input::IsTriggered(Input::DECISION)) {
		if (Game_Message::choice_disabled.test(index)) {
			Game_System::SePlay(Data::system.buzzer_se);
			return;
		}

		Game_System::SePlay(Data::system.decision_se);
		Game_Message::choice_result = index;
		TerminateMessage();
	}
}

////////////////////////////////////////////////////////////
void Window_Message::InputNumber() {
	if (Input::IsTriggered(Input::DECISION)) {
		Game_System::SePlay(Data::system.decision_se);
		Game_Variables[Game_Message::num_input_variable_id] = number_input_window->GetNumber();
		Game_Map::SetNeedRefresh(true);
		TerminateMessage();
		number_input_window->SetNumber(0);
	}
}
