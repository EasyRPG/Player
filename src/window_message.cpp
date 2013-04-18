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

// Headers
#include <cctype>
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
#include "bitmap.h"
#include "font.h"
#include "text.h"

#include <boost/next_prior.hpp>

const int Window_Message::speed_table[21] = {0, 0, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
											7, 7, 8, 8, 9, 9, 10, 10, 11};

Window_Message::Window_Message(int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(ix, iy, iwidth, iheight),
	contents_x(0), contents_y(0), line_count(0), text(""),
	kill_message(false), speed_modifier(0),
	speed_frame_counter(0), new_page_after_pause(false),
	number_input_window(new Window_NumberInput(0, 0)),
	gold_window(new Window_Gold(232, 0, 88, 32))
{
	SetContents(Bitmap::Create(width - 16, height - 16));
	contents->SetTransparentColor(windowskin->GetTransparentColor());

	visible = false;
	SetZ(10000);

	active = false;
	index = -1;
	text_color = Font::ColorDefault;

	number_input_window->SetVisible(false);

	gold_window->SetVisible(false);

	Game_Message::Init();
}

Window_Message::~Window_Message() {
	TerminateMessage();
	Game_Message::visible = false;
}

void Window_Message::StartMessageProcessing() {
	contents->Clear();
	text.clear();
	for (size_t i = 0; i < Game_Message::texts.size(); ++i) {
		std::string const line = Game_Message::texts[i];
		text.append(line + "\n");
	}
	item_max = Game_Message::choice_max;

	text_index = boost::u8_to_u32_iterator<std::string::const_iterator>(text.begin(), text.begin(), text.end());
	end = boost::u8_to_u32_iterator<std::string::const_iterator>(text.end(), text.begin(), text.end());

	InsertNewPage();
}

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

	text.clear();
	text_index = boost::u8_to_u32_iterator<std::string::const_iterator>(text.begin(), text.begin(), text.end());
	end = boost::u8_to_u32_iterator<std::string::const_iterator>(text.end(), text.begin(), text.end());
}

void Window_Message::StartChoiceProcessing() {
	active = true;
	index = 0;
}

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

	contents_y = 2;
	line_count = 0;
	text_color = Font::ColorDefault;
	speed_modifier = 0;

	if (Game_Message::num_input_start == 0 && Game_Message::num_input_variable_id > 0) {
		// If there is an input window on the first line
		StartNumberInputProcessing();
	}
}

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

void Window_Message::TerminateMessage() {
	active = false;
	pause = false;
	index = -1;

	Game_Message::message_waiting = false;
	if (number_input_window->GetVisible()) {
		number_input_window->SetActive(false);
		number_input_window->SetVisible(false);
	}

	if (gold_window->GetVisible()) {
		gold_window->SetVisible(false);
	}
	// The other flag resetting is done in Game_Interpreter::CommandEnd
	Game_Message::SemiClear();
}

bool Window_Message::IsNextMessagePossible() {
	if (Game_Message::num_input_variable_id > 0) {
		return true;
	}

	if (Game_Message::texts.empty()) {
		return false;
	}

	return true;
}

void Window_Message::ResetWindow() {

}

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

void Window_Message::UpdateMessage() {
	// Message Box Show Message rendering loop

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
	int loop_max = speed_table[speed_modifier] == 0 ? 2 : 1;

	while (instant_speed || loop_count < loop_max) {
		// It's assumed that speed_modifier is between 0 and 20
		++speed_frame_counter;

		if (speed_table[speed_modifier] != 0 &&
			speed_table[speed_modifier] != speed_frame_counter) {
				break;
		}

		speed_frame_counter = 0;

		++loop_count;
		if (text_index == end) {
			FinishMessageProcessing();
			break;
		} else if (line_count == 4) {
			pause = true;
			new_page_after_pause = true;
			break;
		}

		if (*text_index == '\n') {
			instant_speed = false;
			InsertNewLine();
			if (pause) {
				break;
			}
		} else if (*text_index == '\f') {
			instant_speed = false;
			++text_index;
			if (*text_index == '\n') {
				++text_index;
			}
			if (text_index != end) {
				pause = true;
				new_page_after_pause = true;
			}
			break;
		} else if (*text_index == '\\' && std::distance(text_index, end) > 1) {
			// Special message codes
			++text_index;

			std::string command_result;

			switch (tolower(*text_index)) {
			case 'c':
			case 'n':
			case 's':
			case 'v':
				// These commands support indirect access via \v[]
				command_result = ParseCommandCode();
				contents->TextDraw(contents_x, contents_y, text_color, command_result);
				contents_x += contents->GetFont()->GetSize(command_result).width;
				break;
			case '\\':
				// Show Backslash
				contents->TextDraw(contents_x, contents_y, text_color, std::string("\\"));
				contents_x += contents->GetFont()->GetSize("\\").width;
				break;
			case '_':
				// Insert half size space
				contents_x += contents->GetFont()->GetSize(" ").width / 2;
				break;
			case '$':
				// Show Gold Window
				gold_window->SetY(y == 0 ? 240 - 32 : 0);
				gold_window->Refresh();
				gold_window->SetOpenAnimation(5);
				gold_window->SetVisible(true);
				break;
			case '!':
				// Text pause
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
				instant_speed = true;
				break;
			case '<':
				// Instant speed stop
				instant_speed = false;
				break;
			case '.':
				// 1/4 second sleep
				sleep_until = Graphics::GetFrameCount() + 60 / 4;
				++text_index;
				return;
				break;
			case '|':
				// Second sleep
				sleep_until = Graphics::GetFrameCount() + 60;
				++text_index;
				return;
				break;
			default:;
			}
		} else if (*text_index == '$'
				   && std::distance(text_index, end) > 1
				   && std::isalpha(*boost::next(text_index))) {
			// ExFont
			contents->TextDraw(contents_x, contents_y, text_color,
							   std::string(text_index.base(), boost::next(text_index, 2).base()));
			contents_x += 12;
			++text_index;
		} else {
			std::string const glyph(text_index.base(), boost::next(text_index).base());

			contents->TextDraw(contents_x, contents_y, text_color, glyph);
			contents_x += contents->GetFont()->GetSize(glyph).width;
		}

		++text_index;
	}
	loop_count = 0;
}

int Window_Message::ParseParameter(bool& is_valid, int call_depth) {
	++text_index;

	if (text_index == end ||
		*text_index != '[') {
		--text_index;
		is_valid = false;
		return 0;
	}

	++text_index; // Skip the [

	bool null_at_start = false;
	std::stringstream ss;
	for (;;) {
		if (text_index == end) {
			break;
		} else if (*text_index == '\n') {
			--text_index;
			break;
		}
		else if (*text_index == '0') {
			// Truncate 0 at the start
			if (!ss.str().empty()) {
				ss << '0';
			} else {
				null_at_start = true;
			}
		}
		else if (*text_index >= '1' &&
			*text_index <= '9') {
			ss << std::string(text_index.base(), boost::next(text_index).base());
		} else if (*text_index == ']') {
			--call_depth;
			if (call_depth == 0) {
				break;
			}
		} else {
			// End of number
			// Search for ] or line break
			while (text_index != end) {
					if (*text_index == '\n') {
						--text_index;
						break;
					} else if (*text_index == ']') {
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

std::string Window_Message::ParseCommandCode(int call_depth) {
	int parameter;
	bool is_valid;
	// sub_code is used by chained arguments like \v[\v[1]]
	// In that case sub_code contains the result from \v[1]
	int sub_code = -1;
	uint32_t cmd_char = *text_index;
	if (std::distance(text_index, end) > 3 &&
		*boost::next(text_index, 2) == '\\' &&
		tolower(*boost::next(text_index, 3)) == 'v') {
		++(++(++text_index));
		// The result is an int value, str-to-int is safe in this case
		std::stringstream ss;
		ss << ParseCommandCode(++call_depth);
		ss >> sub_code;
	}
	switch (tolower(cmd_char)) {
	case 'c':
		// Color
		if (sub_code >= 0) {
			parameter = sub_code;
		} else {
			parameter = ParseParameter(is_valid, call_depth);
		}
		text_color = parameter > 19 ? 0 : parameter;
		break;
	case 'n':
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
		// Speed modifier
		if (sub_code >= 0) {
			is_valid = true;
			parameter = sub_code;
		} else {
			parameter = ParseParameter(is_valid, call_depth);
		}

		speed_modifier = min(parameter, 20);
		speed_modifier = max(0, speed_modifier);
		break;
	case 'v':
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
			ss << "0";
			return ss.str();
		}
	default:;
		// When this happens text_index was not on a \ during calling
	}
	return "";
}

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

void Window_Message::WaitForInput() {
	active = true; // Enables the Pause arrow
	if (Input::IsTriggered(Input::DECISION) ||
		Input::IsTriggered(Input::CANCEL)) {
		active = false;
		pause = false;

		if (text.empty()) {
			TerminateMessage();
		} else if (text_index != end && new_page_after_pause) {
			new_page_after_pause = false;
			InsertNewPage();
		}
	}
}

void Window_Message::InputChoice() {
	if (Input::IsTriggered(Input::CANCEL)) {
		if (Game_Message::choice_cancel_type > 0) {
			Game_System::SePlay(Main_Data::game_data.system.cancel_se);
			Game_Message::choice_result = Game_Message::choice_cancel_type - 1; // Cancel
			TerminateMessage();
		}
	} else if (Input::IsTriggered(Input::DECISION)) {
		if (Game_Message::choice_disabled.test(index)) {
			Game_System::SePlay(Main_Data::game_data.system.buzzer_se);
			return;
		}

		Game_System::SePlay(Main_Data::game_data.system.decision_se);
		Game_Message::choice_result = index;
		TerminateMessage();
	}
}

void Window_Message::InputNumber() {
	if (Input::IsTriggered(Input::DECISION)) {
		Game_System::SePlay(Main_Data::game_data.system.decision_se);
		Game_Variables[Game_Message::num_input_variable_id] = number_input_window->GetNumber();
		Game_Map::SetNeedRefresh(true);
		TerminateMessage();
		number_input_window->SetNumber(0);
	}
}
