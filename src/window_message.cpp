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
#include <iterator>

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

constexpr int message_animation_frames = 8;

// C4428 is nonsense
#ifdef _MSC_VER
#pragma warning (disable : 4428)
#endif

Window_Message::Window_Message(int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(ix, iy, iwidth, iheight),
	number_input_window(new Window_NumberInput(0, 0)),
	gold_window(new Window_Gold(232, 0, 88, 32))
{
	SetContents(Bitmap::Create(width - 16, height - 16));

	if (Data::battlecommands.battle_type != RPG::BattleCommands::BattleType_traditional &&
		Data::battlecommands.transparency == RPG::BattleCommands::Transparency_transparent) {
		SetBackOpacity(128);
	}
	gold_window->SetBackOpacity(GetBackOpacity());

	visible = false;
	// Above other windows
	SetZ(Priority_Window + 100);

	active = false;
	index = -1;
	text_color = Font::ColorDefault;

	number_input_window->SetVisible(false);

	gold_window->SetVisible(false);

	Game_Message::Init();
	Game_Message::SetWindow(this);
}

Window_Message::~Window_Message() {
	TerminateMessage();
	if (Game_Message::GetWindow() == this) {
		Game_Message::SetWindow(nullptr);
	}
}

void Window_Message::ApplyTextInsertingCommands() {
	text_index = text.end();
	end = text.end();

	// Contains already substitued \N actors to prevent endless recursion
	std::vector<int> replaced_actors;
	int actor_replacement_start = std::distance(text.begin(), end);

	if (!text.empty()) {
		// Move on first valid char
		--text_index;

		// Apply commands that insert text
		while (std::distance(text_index, text.begin()) <= -1) {
			char ch = tolower(*text_index--);
			switch (ch) {
			case 'n':
			case 'v':
			{
				if (*text_index != Player::escape_char) {
					continue;
				}
				++text_index;

				auto start_code = text_index - 1;
				bool success;
				int parsed_num;
				std::u32string command_result = Utils::DecodeUTF32(ParseCommandCode(success, parsed_num));
				if (start_code < text.begin() + actor_replacement_start) {
					replaced_actors.clear();
				}

				if (!success || std::find(replaced_actors.begin(), replaced_actors.end(), parsed_num) != replaced_actors.end()) {
					text_index = start_code;
					continue;
				}

				if (ch == 'n') {
					replaced_actors.push_back(parsed_num);
					actor_replacement_start = std::min<int>(std::distance(text.begin(), start_code), actor_replacement_start);
				}

				text.replace(start_code, text_index + 1, command_result);
				// Start from the beginning, the inserted text might add new commands
				text_index = text.end();
				end = text.end();
				actor_replacement_start = std::min<int> (std::distance(text.begin(), end), actor_replacement_start);

				// Move on first valid char
				--text_index;

				break;
			}
			default:
				break;
			}
		}
	}
}

void Window_Message::StartMessageProcessing(PendingMessage pm) {
	contents->Clear();
	pending_message = std::move(pm);
	allow_next_message = false;

	const auto& lines = pending_message.GetLines();
	if (!(pending_message.NumLines() > 0 || pending_message.HasNumberInput())) {
		return;
	}

	if (pending_message.IsWordWrapped()) {
		std::u32string wrapped_text;
		for (const std::string& line : lines) {
			/* TODO: don't take commands like \> \< into account when word-wrapping */
			if (pending_message.IsWordWrapped()) {
				// since ApplyTextInsertingCommands works for the text variable,
				// we store line into text and use wrapped_text for the real 'text'
				text = Utils::DecodeUTF32(line);
				ApplyTextInsertingCommands();
				Game_Message::WordWrap(
						Utils::EncodeUTF(text),
						width - 24,
						[&wrapped_text](const std::string& wrapped_line) {
							wrapped_text.append(Utils::DecodeUTF32(wrapped_line)).append(1, U'\n');
						}
				);
				text = wrapped_text;
			}
		}
	}
	else {
		text.clear();
		for (const std::string& line : lines) {
			text.append(Utils::DecodeUTF32(line)).append(1, U'\n');
		}
	}
	item_max = min(4, pending_message.GetNumChoices());

	ApplyTextInsertingCommands();
	text_index = text.begin();

	// If we're displaying a new message, reset the closing animation.
	if (closing) {
		SetCloseAnimation(Game_Temp::battle_running ? 0 : message_animation_frames);
	}

	InsertNewPage();
}

void Window_Message::FinishMessageProcessing() {
	if (pending_message.GetNumChoices() > 0) {
		StartChoiceProcessing();
	} else if (pending_message.HasNumberInput()) {
		StartNumberInputProcessing();
	} else if (kill_message) {
		TerminateMessage();
	} else {
		SetPause(true);
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
	number_input_window->SetMaxDigits(pending_message.GetNumberInputDigits());
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
		gold_window->SetOpenAnimation(message_animation_frames);
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
		gold_window->SetBackOpacity(0);
	} else {
		SetOpacity(255);
		gold_window->SetBackOpacity(GetBackOpacity());
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

	if (pending_message.GetChoiceStartLine() == 0 && pending_message.HasChoices()) {
		contents_x += 12;
	}

	contents_y = 2;
	line_count = 0;
	text_color = Font::ColorDefault;
	speed = 1;

	if (pending_message.GetNumberInputStartLine() == 0 && pending_message.HasNumberInput()) {
		// If there is an input window on the first line
		StartNumberInputProcessing();
	}
	line_char_counter = 0;
}

void Window_Message::InsertNewLine() {
	if (!Game_Message::GetFaceName().empty() && !Game_Message::IsFaceRightPosition()) {
		contents_x = LeftMargin + FaceSize + RightFaceMargin;
	} else {
		contents_x = 0;
	}

	contents_y += 16;
	++line_count;

	if (pending_message.HasChoices() && line_count >= pending_message.GetChoiceStartLine()) {
		unsigned choice_index = line_count - pending_message.GetChoiceStartLine();
		if (pending_message.GetChoiceResetColor()) {
			// Check for disabled choices
			if (!pending_message.IsChoiceEnabled(choice_index)) {
				text_color = Font::ColorDisabled;
			} else {
				text_color = Font::ColorDefault;
			}
		}

		contents_x += 12;
	}
	line_char_counter = 0;
}

void Window_Message::TerminateMessage() {
	active = false;
	SetPause(false);
	kill_message = false;
	line_char_counter = 0;
	index = -1;

	if (number_input_window->GetVisible()) {
		number_input_window->SetActive(false);
		number_input_window->SetVisible(false);
	}

	if (gold_window->GetVisible()) {
		gold_window->SetCloseAnimation(message_animation_frames);
	}

	// This clears the active flag.
	pending_message = {};
}

void Window_Message::ResetWindow() {

}

void Window_Message::Update() {
	bool update_message_processing = false;
	allow_next_message = false;

	if (pending_message.ShowGoldWindow()) {
		ShowGoldWindow();
	}

	if (wait_count == 0) {
		if (GetPause()) {
			WaitForInput();
		} else if (active) {
			InputChoice();
		} else if (number_input_window->GetVisible()) {
			InputNumber();
		} else if (!text.empty()) {
			if (!visible) {
				// The MessageBox is not open yet but text output is needed
				// Open and Close Animations are skipped in battle
				SetOpenAnimation(Game_Temp::battle_running ? 0 : message_animation_frames);
			} else if (closing) {
				// If a message was requested while closing, cancel it and display the message immediately.
				SetOpenAnimation(0);
			} else {
				//Handled after base class updates.
				if (text_index != text.end()) {
					update_message_processing = true;
				}
				if (text_index == text.end() && wait_count <= 0) {
					FinishMessageProcessing();
				}
			}
		}

		if (!Game_Message::IsMessagePending() && visible && !closing) {
			// Start the closing animation
			SetCloseAnimation(Game_Temp::battle_running ? 0 : message_animation_frames);
			// This frame a foreground event may push a new message and interupt the close animation.
			allow_next_message = true;
		}
	}

	Window_Selectable::Update();
	number_input_window->Update();
	gold_window->Update();

	if (wait_count > 0) {
		--wait_count;
		return;
	}

	if (update_message_processing) {
		UpdateMessage();
	}
}

void Window_Message::UpdateMessage() {
	if (IsOpeningOrClosing()) {
		return;
	}

	// Message Box Show Message rendering loop
	bool instant_speed = false;
	bool instant_speed_forced = false;

	if (Player::debug_flag && Input::IsPressed(Input::SHIFT)) {
		instant_speed = true;
		instant_speed_forced = true;
	}

	while (true) {
		if (wait_count > 0) {
			--wait_count;
			break;
		}

		if (text_index == end) {
			if (!instant_speed) {
				SetWaitForPage();
			}
			break;
		}

		if (line_count == 4) {
			// FIXME: Unify pause logic
			SetPause(true);
			new_page_after_pause = true;
			if (!instant_speed) {
				//FIXME: Does this wait happen when pause is enabled?
				SetWaitForPage();
			}
			break;
		}

		if (GetPause()) {
			break;
		}

		if (*text_index == '\r') {
			// Not handled
			++text_index;
			continue;
		}

		if (*text_index == '\n') {
			if (text_index + 1 != end) {
				if (!instant_speed && line_char_counter == 0) {
					// RPG_RT will always wait 1 frame for each empty line.
					SetWait(1);
				}
				if (instant_speed && !instant_speed_forced) {
					// instant_speed stops at the end of the line
					// unless it was triggered by the shift key.
					instant_speed = false;
				}
			}
			InsertNewLine();
			++text_index;
			continue;
		}

		if (*text_index == '\f') {
			// Used by our code to inject form feeds.
			instant_speed = false;
			++text_index;
			if (*text_index == '\n') {
				++text_index;
			}
			if (text_index != end) {
				SetPause(true);
				new_page_after_pause = true;
			}
			//FIXME: Delays formfeed?
			break;
		}

		if (*text_index == Player::escape_char && std::distance(text_index, end) > 1) {
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
				speed = Utils::Clamp(parameter, 1, 20);
				break;
			case '_':
				// Insert half size space
				contents_x += Font::Default()->GetSize(" ").width / 2;
				if (!instant_speed) {
					SetWaitForCharacter(1);
				}
				IncrementLineCharCounter(1);
				break;
			case '$':
				// Show Gold Window
				ShowGoldWindow();
				if (!instant_speed) {
					SetWait(speed);
				}
				break;
			case '!':
				// Text pause
				SetPause(true);
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
				// Instant speed stop - also cancels shift key and forces a delay.
				instant_speed = false;
				instant_speed_forced = false;
				SetWait(speed);
				break;
			case '.':
				// 1/4 second sleep
				if (!instant_speed) {
					// Despite documentation saying 1/4 second, RPG_RT waits for 20 frames.
					SetWait(20);
				}
				break;
			case '|':
				// Second sleep
				if (!instant_speed) {
					// Despite documentation saying 1 second, RPG_RT waits for 61 frames.
					SetWait(61);
				}
				break;
			case '\r':
			case '\n':
			case '\f':
				// \ followed by linebreak, don't skip them
				--text_index;
				break;
			default:
				if (*text_index == Player::escape_char) {
					DrawGlyph(Player::escape_symbol, instant_speed);
				}
				break;
			}
			++text_index;
			continue;
		}

		if (*text_index == '$'
				   && std::distance(text_index, end) > 1
				   && std::isalpha(*std::next(text_index))) {
			// ExFont
			DrawGlyph(Utils::EncodeUTF(std::u32string(text_index, std::next(text_index, 2))), instant_speed);
			text_index += 2;
			continue;
		}

		DrawGlyph(Utils::EncodeUTF(std::u32string(text_index, std::next(text_index))), instant_speed);
		++text_index;
	}
}

void Window_Message::DrawGlyph(const std::string& glyph, bool instant_speed) {
#ifdef EP_DEBUG_MESSAGE
	Output::Debug("Msg Draw Glyph %s %d", glyph.c_str(), instant_speed);
#endif
	contents->TextDraw(contents_x, contents_y, text_color, glyph);
	int glyph_width = Font::Default()->GetSize(glyph).width;
	contents_x += glyph_width;
	int width = (glyph_width - 1) / 6 + 1;
	if (!instant_speed && glyph_width > 0) {
		// RPG_RT compatible for half-width (6) and full-width (12)
		// generalizes the algo for even bigger glyphs
		SetWaitForCharacter(width);
	}
	IncrementLineCharCounter(width);
}

void Window_Message::IncrementLineCharCounter(int width) {
	// For speed 1, RPG_RT prints 2 half width chars every frame. This 
	// resets anytime we print a full width character or another
	// character with a different speed. 
	// To emulate this, we increment by 2 and clear the low bit anytime
	// we're not a speed 1 half width char.
	if (width == 1 && speed <= 1) {
		line_char_counter++;
	} else {
		line_char_counter = (line_char_counter & ~1) + 2;
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

std::string Window_Message::ParseCommandCode(bool& success, int& parameter) {
	bool is_valid;
	uint32_t cmd_char = *text_index;
	success = true;
	parameter = -1;

	switch (tolower(cmd_char)) {
	case 'n':
		// Output Hero name
		parameter = ParseParameter(is_valid);
		if (is_valid) {
			Game_Actor* actor = NULL;
			if (parameter == 0) {
				// Party hero
				if (Main_Data::game_party->GetBattlerCount() > 0) {
					actor = Main_Data::game_party->GetActors()[0];
				}
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
		if (is_valid) {
			return std::to_string(Main_Data::game_variables->Get(parameter));
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
		int y_pos = (pending_message.GetChoiceStartLine() + index) * 16;
		int width = contents->GetWidth();

		if (!Game_Message::GetFaceName().empty()) {
			if (!Game_Message::IsFaceRightPosition()) {
				x_pos += LeftMargin + FaceSize + RightFaceMargin;
			}
			width = width - LeftMargin - FaceSize - RightFaceMargin - 4;
		}

		cursor_rect = { x_pos, y_pos, width, 16 };
	} else {
		cursor_rect = { 0, 0, 0, 0 };
	}
}

void Window_Message::WaitForInput() {
	active = true; // Enables the Pause arrow
	if (Input::IsTriggered(Input::DECISION) ||
			Input::IsTriggered(Input::CANCEL)) {
		active = false;
		SetPause(false);

		if (text.empty()) {
			TerminateMessage();
		} else if (text_index != end && new_page_after_pause) {
			new_page_after_pause = false;
			InsertNewPage();
		}
	}
}

void Window_Message::InputChoice() {
	bool do_terminate = false;
	int choice_result = -1;

	if (Input::IsTriggered(Input::CANCEL)) {
		if (pending_message.GetChoiceCancelType() > 0) {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cancel));
			choice_result = pending_message.GetChoiceCancelType() - 1; // Cancel
			do_terminate = true;
		}
	} else if (Input::IsTriggered(Input::DECISION)) {
		if (!pending_message.IsChoiceEnabled(index)) {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
			return;
		}

		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
		choice_result = index;
		do_terminate = true;
	}

	if (do_terminate) {
		if (choice_result >= 0) {
			auto& continuation = pending_message.GetChoiceContinuation();
			if (continuation) {
				continuation(choice_result);
			}
		}
		TerminateMessage();
	}
}

void Window_Message::InputNumber() {
	if (Input::IsTriggered(Input::DECISION)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
		Main_Data::game_variables->Set(pending_message.GetNumberInputVariable(), number_input_window->GetNumber());
		Game_Map::SetNeedRefresh(Game_Map::Refresh_Map);
		TerminateMessage();
		number_input_window->SetNumber(0);
	}
}

void Window_Message::SetWaitForCharacter(int width) {
	int frames = 0;
	if (width > 0) {
		if (speed > 1) {
			frames = speed * width / 2 + 1;
		} else {
			frames = width / 2;
			if (width & 1) {
				// For odd widths, speed 1 adds a 1 frame delay for every odd character printed.
				// This logic assumes num chars is incremented after the wait.
				frames += !(line_char_counter & 1);
			}
		}
	}
	SetWait(frames);
}

void Window_Message::SetWaitForPage() {
	SetWait(speed);
}

void Window_Message::SetWait(int frames) {
	assert(speed >= 1 && speed <= 20);
#ifdef EP_DEBUG_MESSAGE
	Output::Debug("Msg Wait %d", frames);
#endif
	wait_count = frames;
}
