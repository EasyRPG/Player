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
#include "game_system.h"
#include "game_variables.h"
#include "game_temp.h"
#include "input.h"
#include "output.h"
#include "player.h"
#include "util_macro.h"
#include "bitmap.h"
#include "font.h"

const int Window_Message::speed_table[21] = {0, 0, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
											7, 7, 8, 8, 9, 9, 10, 10, 11};

// C4428 is nonsense
#ifdef _MSC_VER
#pragma warning (disable : 4428)
#endif

Window_Message::Window_Message(int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(ix, iy, iwidth, iheight),
	contents_x(0), contents_y(0), line_count(0),
	kill_message(false), speed_modifier(0),
	speed_frame_counter(0), new_page_after_pause(false),
	number_input_window(new Window_NumberInput(0, 0)),
	gold_window(new Window_Gold(232, 0, 88, 32))
{
	SetContents(Bitmap::Create(width - 16, height - 16));

	if (Data::battlecommands.battle_type != RPG::BattleCommands::BattleType_traditional &&
		Data::battlecommands.transparency == RPG::BattleCommands::Transparency_transparent) {
		SetBackOpacity(128);
	}

	visible = false;
	SetZ(10000);

	escape_char = Utils::DecodeUTF32(Player::escape_symbol).front();
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
	for (const std::string& line : Game_Message::texts) {
		text.append(Utils::DecodeUTF32(line)).append(1, U'\n');
	}
	Game_Message::texts.clear();
	item_max = min(4, Game_Message::choice_max);

	text_index = text.end();
	end = text.end();

	if (!text.empty()) {
		// Move on first valid char
		--text_index;

		// Apply commands that insert text
		while (std::distance(text_index, text.begin()) <= -1) {
			switch (tolower(*text_index--)) {
			case 'n':
			case 'v':
			{
				if (*text_index != escape_char) {
					continue;
				}
				++text_index;

				auto start_code = text_index - 1;
				bool success;
				std::u32string command_result = Utils::DecodeUTF32(ParseCommandCode(success));
				if (!success) {
					text_index = start_code - 2;
					continue;
				}
				text.replace(start_code, text_index + 1, command_result);
				// Start from the beginning, the inserted text might add new commands
				text_index = text.end();
				end = text.end();

				// Move on first valid char
				--text_index;

				break;
			}
			default:
				break;
			}
		}
	}

	text_index = text.begin();

	InsertNewPage();
}

void Window_Message::FinishMessageProcessing() {
	if (Game_Message::choice_max > 0) {
		StartChoiceProcessing();
	} else if (Game_Message::num_input_variable_id > 0) {
		StartNumberInputProcessing();
	} else if (kill_message) {
		TerminateMessage();
	} else {
		pause = true;
	}

	text.clear();
	text_index = text.begin();
	end = text.end();
}

void Window_Message::StartChoiceProcessing() {
	active = true;
	index = 0;
}

void Window_Message::StartNumberInputProcessing() {
	number_input_window->SetMaxDigits(Game_Message::num_input_digits_max);
	if (!Game_Message::GetFaceName().empty() && !Game_Message::IsFaceRightPosition()) {
		number_input_window->SetX(LeftMargin + FaceSize + RightFaceMargin);
	} else {
		number_input_window->SetX(x);
	}
	number_input_window->SetY(y + contents_y - 2);
	number_input_window->SetActive(true);
	number_input_window->SetVisible(true);
	number_input_window->Update();
}

void Window_Message::ShowGoldWindow() {
	if (!gold_window->GetVisible() && !Game_Temp::battle_running) {
		gold_window->SetY(y == 0 ? SCREEN_TARGET_HEIGHT - 32 : 0);
		gold_window->Refresh();
		gold_window->SetOpenAnimation(5);
	}
}

void Window_Message::InsertNewPage() {
	// Cancel pending face requests for async
	// Otherwise they render on the wrong page
	face_request_ids.clear();

	contents->Clear();

	y = Game_Message::GetRealPosition() * 80;

	if (Game_Message::IsTransparent()) {
		SetOpacity(0);
	} else {
		SetOpacity(255);
	}

	if (!Game_Message::GetFaceName().empty()) {
		if (!Game_Message::IsFaceRightPosition()) {
			contents_x = LeftMargin + FaceSize + RightFaceMargin;
			DrawFace(Game_Message::GetFaceName(), Game_Message::GetFaceIndex(), LeftMargin, TopMargin, Game_Message::IsFaceFlipped());
		} else {
			contents_x = 0;
			DrawFace(Game_Message::GetFaceName(), Game_Message::GetFaceIndex(), 248, TopMargin, Game_Message::IsFaceFlipped());
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
	if (!Game_Message::GetFaceName().empty() && !Game_Message::IsFaceRightPosition()) {
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
	kill_message = false;
	index = -1;

	Game_Message::message_waiting = false;
	if (number_input_window->GetVisible()) {
		number_input_window->SetActive(false);
		number_input_window->SetCloseAnimation(5);
	}

	if (gold_window->GetVisible()) {
		gold_window->SetCloseAnimation(5);
	}
	// The other flag resetting is done in Game_Interpreter::CommandEnd
	Game_Message::SemiClear();
}

bool Window_Message::IsNextMessagePossible() {
	return Game_Message::num_input_variable_id > 0 || !Game_Message::texts.empty();
}

void Window_Message::ResetWindow() {

}

void Window_Message::Update() {
	Window_Selectable::Update();
	number_input_window->Update();
	gold_window->Update();

	if (pause) {
		WaitForInput();
	} else if (active) {
		InputChoice();
	} else if (number_input_window->GetVisible()) {
		InputNumber();
	} else if (!text.empty()) {
		// Output the remaining text for the current page
		UpdateMessage();
	} else if (IsNextMessagePossible()) {
		// Output a new page
		if (Game_Temp::inn_calling) {
			ShowGoldWindow();
		}

		StartMessageProcessing();
		//printf("Text: %s\n", text.c_str());
		if (!visible) {
			// The MessageBox is not open yet but text output is needed
			// Open and Close Animations are skipped in battle
			SetOpenAnimation(Game_Temp::battle_running ? 0 : 5);
		} else if (closing) {
			// Cancel closing animation
			SetOpenAnimation(0);
		}
		Game_Message::visible = true;
	} else if (!Game_Message::message_waiting && Game_Message::visible) {
		if (visible && !closing) {
			// Start the closing animation
			SetCloseAnimation(Game_Temp::battle_running ? 0 : 5);
		} else if (!visible) {
			// The closing animation has finished
			Game_Message::visible = false;
			Game_Message::owner_id = 0;
		}
	}
}

void Window_Message::UpdateMessage() {
	// Message Box Show Message rendering loop

	// Contains at what frame the sleep is over
	static int sleep_until = -1;
	bool instant_speed = false;

	if (Player::debug_flag && Input::IsPressed(Input::SHIFT)) {
		sleep_until = -1;
		instant_speed = true;
	}

	if (sleep_until > -1) {
		if (Player::GetFrames() >= sleep_until) {
			// Sleep over
			sleep_until = -1;
		} else {
			return;
		}
	}

	int loop_count = 0;
	int loop_max = speed_table[speed_modifier] == 0 ? 2 : 1;

	while (instant_speed || loop_count < loop_max) {
		if (!instant_speed) {
			// It's assumed that speed_modifier is between 0 and 20
			++speed_frame_counter;

			if (speed_table[speed_modifier] != 0 &&
				speed_table[speed_modifier] != speed_frame_counter) {
				break;
			}

			speed_frame_counter = 0;
		}

		++loop_count;
		if (text_index == end) {
			FinishMessageProcessing();
			break;
		} else if (line_count == 4) {
			pause = true;
			new_page_after_pause = true;
			break;
		} else if (pause) {
			break;
		}

		if (*text_index == '\n') {
			if (instant_speed) {
				// instant_speed stops at the end of the line, unless
				// there's a /> at the beginning of the next line
				if (std::distance(text_index, end) > 2 &&
					*(text_index + 1) == escape_char &&
					*(text_index + 2) == '>') {
					text_index += 2;
				} else {
					instant_speed = false;
				}
			}
			InsertNewLine();
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
		} else if (*text_index == escape_char && std::distance(text_index, end) > 1) {
			// Special message codes
			++text_index;

			int parameter;
			bool is_valid;
			switch (tolower(*text_index)) {
			case 'c':
				// Color
				parameter = ParseParameter(is_valid);
				text_color = parameter > 19 ? 0 : parameter;
				break;
			case 's':
				// Speed modifier
				parameter = ParseParameter(is_valid);
				speed_modifier = max(0, min(parameter, 20));
				break;
			case '_':
				// Insert half size space
				contents_x += contents->GetFont()->GetSize(" ").width / 2;
				break;
			case '$':
				// Show Gold Window
				ShowGoldWindow();
				break;
			case '!':
				// Text pause
				pause = true;
				break;
			case '^':
				// Force message close
				// The close happens at the end of the message, not where
				// the ^ is encountered
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
				if (instant_speed) break;
				sleep_until = Player::GetFrames() + 60 / 4;
				++text_index;
				return;
			case '|':
				// Second sleep
				if (instant_speed) break;
				sleep_until = Player::GetFrames() + 60;
				++text_index;
				return;
			case '\n':
			case '\f':
				// \ followed by linebreak, don't skip them
				--text_index;
				break;
			default:
				if (*text_index == escape_char) {
					// Show Escape Symbol
					contents->TextDraw(contents_x, contents_y, text_color, Player::escape_symbol);
					contents_x += contents->GetFont()->GetSize(Player::escape_symbol).width;
				}
			}
		} else if (*text_index == '$'
				   && std::distance(text_index, end) > 1
				   && std::isalpha(*std::next(text_index))) {
			// ExFont
			std::string const glyph(Utils::EncodeUTF(std::u32string(text_index, std::next(text_index, 2))));
			contents->TextDraw(contents_x, contents_y, text_color, glyph);
			contents_x += 12;
			++loop_count;
			++text_index;
		} else {
			std::string const glyph(Utils::EncodeUTF(std::u32string(text_index, std::next(text_index))));

			contents->TextDraw(contents_x, contents_y, text_color, glyph);
			int glyph_width = contents->GetFont()->GetSize(glyph).width;
			// Show full-width characters twice as slow as half-width characters
			if (glyph_width >= 12)
				loop_count++;
			contents_x += glyph_width;
		}

		++text_index;
	}
}

int Window_Message::ParseParameter(bool& is_valid) {
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
				ss << "0";
			} else {
				null_at_start = true;
			}
		}
		else if (*text_index >= '1' &&
			*text_index <= '9') {
			ss << std::string(text_index, std::next(text_index));
		} else if (*text_index == ']') {
			break;
		} else {
			// End of number
			// Search for ] or line break
			while (text_index != end) {
					if (*text_index == '\n') {
						--text_index;
						break;
					} else if (*text_index == ']') {
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
	return num;
}

std::string Window_Message::ParseCommandCode(bool& success) {
	int parameter;
	bool is_valid;
	uint32_t cmd_char = *text_index;
	success = true;

	switch (tolower(cmd_char)) {
	case 'n':
		// Output Hero name
		parameter = ParseParameter(is_valid);
		if (is_valid) {
			Game_Actor* actor = NULL;
			if (parameter == 0) {
				// Party hero
				actor = Main_Data::game_party->GetActors()[0];
			} else {
				actor = Game_Actors::GetActor(parameter);
			}
			if (actor != NULL) {
				return actor->GetName();
			}
		}
		break;
	case 'v':
		// Show Variable value
		parameter = ParseParameter(is_valid);
		if (is_valid && Game_Variables.IsValid(parameter)) {
			std::stringstream ss;
			ss << Game_Variables[parameter];
			return ss.str();
		} else {
			// Invalid Var is always 0
			return "0";
		}
	default:;
		// When this happens text_index was not on a \ during calling
	}
	success = false;
	return "";
}

void Window_Message::UpdateCursorRect() {
	if (index >= 0) {
		int x_pos = 2;
		int y_pos = (Game_Message::choice_start + index) * 16;
		int width = contents->GetWidth();

		if (!Game_Message::GetFaceName().empty()) {
			if (!Game_Message::IsFaceRightPosition()) {
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
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cancel));
			Game_Message::choice_result = Game_Message::choice_cancel_type - 1; // Cancel
			TerminateMessage();
		}
	} else if (Input::IsTriggered(Input::DECISION)) {
		if (Game_Message::choice_disabled.test(index)) {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
			return;
		}

		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
		Game_Message::choice_result = index;
		TerminateMessage();
	}
}

void Window_Message::InputNumber() {
	if (Input::IsTriggered(Input::DECISION)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
		Game_Variables[Game_Message::num_input_variable_id] = number_input_window->GetNumber();
		Game_Map::SetNeedRefresh(Game_Map::Refresh_Map);
		TerminateMessage();
		number_input_window->SetNumber(0);
	}
}
