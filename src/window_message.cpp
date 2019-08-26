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

	visible = false;
	// Above other windows
	SetZ(Priority_Window + 100);

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

	const auto& pm = Game_Message::GetPendingMessage();

	if (pm.ShowGoldWindow()) {
		ShowGoldWindow();
	}

	const auto& pm_text = pm.GetText();

	if (pm.IsWordWrapped()) {
		std::string wrapped_text;
		/* TODO: don't take commands like \> \< into account when word-wrapping */
		Game_Message::WordWrap(
				pm_text,
				width - 24,
				[&wrapped_text](const std::string& wrapped_line) {
				wrapped_text.append(wrapped_line).append(1, '\n');
				}
				);
		text = Utils::DecodeUTF32(wrapped_text);
	} else {
		text = Utils::DecodeUTF32(pm_text);
	}
	item_max = min(4, pm.GetNumChoices());

	text_index = text.begin();

	InsertNewPage();
}

void Window_Message::FinishMessageProcessing() {
	auto& pm = Game_Message::GetPendingMessage();

	if (pm.GetNumChoices() > 0) {
		StartChoiceProcessing();
	} else if (pm.HasNumberInput()) {
		StartNumberInputProcessing();
	} else if (kill_message) {
		TerminateMessage();
	} else {
		pause = true;
	}

	text.clear();
	text_index = text.begin();
}

void Window_Message::StartChoiceProcessing() {
	active = true;
	index = 0;
}

void Window_Message::StartNumberInputProcessing() {
	auto& pm = Game_Message::GetPendingMessage();

	number_input_window->SetMaxDigits(pm.GetNumberInputDigits());
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

	auto& pm = Game_Message::GetPendingMessage();

	if (pm.GetChoiceStartLine() == 0 && pm.HasChoices()) {
		contents_x += 12;
	}

	contents_y = 2;
	line_count = 0;
	text_color = Font::ColorDefault;
	speed = 1;

	if (pm.GetNumberInputStartLine() == 0 && pm.HasNumberInput()) {
		// If there is an input window on the first line
		StartNumberInputProcessing();
	}
	num_chars_printed_this_line = 0;
}

void Window_Message::InsertNewLine() {
	if (!Game_Message::GetFaceName().empty() && !Game_Message::IsFaceRightPosition()) {
		contents_x = LeftMargin + FaceSize + RightFaceMargin;
	} else {
		contents_x = 0;
	}

	contents_y += 16;
	++line_count;

	auto& pm = Game_Message::GetPendingMessage();

	if (pm.HasChoices() && line_count >= pm.GetChoiceStartLine()) {
		unsigned choice_index = line_count - pm.GetChoiceStartLine();
		// Check for disabled choices
		if (!pm.IsChoiceEnabled(choice_index)) {
			text_color = Font::ColorDisabled;
		}

		contents_x += 12;
	}
	num_chars_printed_this_line = 0;
}

void Window_Message::TerminateMessage() {
	active = false;
	pause = false;
	kill_message = false;
	num_chars_printed_this_line = 0;
	index = -1;

	if (number_input_window->GetVisible()) {
		number_input_window->SetActive(false);
		number_input_window->SetVisible(false);
	}

	if (gold_window->GetVisible()) {
		gold_window->SetCloseAnimation(message_animation_frames);
	}
	Game_Message::ResetPendingMessage();
}

bool Window_Message::IsNextMessagePossible() {
	auto& pm = Game_Message::GetPendingMessage();
	return pm.NumLines() > 0 || pm.HasNumberInput();
}

void Window_Message::ResetWindow() {

}

void Window_Message::Update() {
	bool update_message_processing = false;
	if (wait_count == 0) {
		if (pause) {
			WaitForInput();
		} else if (active) {
			InputChoice();
		} else if (number_input_window->GetVisible()) {
			InputNumber();
		} else if (!text.empty()) {
			//Handled after base class updates.
			if (text_index != text.end()) {
				update_message_processing = true;
			}
			if (text_index == text.end() && wait_count <= 0) {
				FinishMessageProcessing();
			}
		} else if (IsNextMessagePossible()) {
			StartMessageProcessing();
			//printf("Text: %s\n", text.c_str());
			if (!visible) {
				// The MessageBox is not open yet but text output is needed
				// Open and Close Animations are skipped in battle
				SetOpenAnimation(Game_Temp::battle_running ? 0 : message_animation_frames);
			} else if (closing) {
				// Cancel closing animation
				SetOpenAnimation(0);
			}
			Game_Message::visible = true;
		}

		if (!Game_Message::message_waiting && Game_Message::visible) {
			if (visible && !closing) {
				// Start the closing animation
				SetCloseAnimation(Game_Temp::battle_running ? 0 : message_animation_frames);
				Game_Message::closing = true;
			}
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

	if (!visible) {
		// The closing animation has finished
		Game_Message::closing = false;
		Game_Message::visible = false;
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

		if (text_index == text.end()) {
			if (!instant_speed) {
				SetWaitForPage();
			}
			break;
		}

		if (line_count == 4) {
			// FIXME: Unify pause logic
			pause = true;
			new_page_after_pause = true;
			if (!instant_speed) {
				//FIXME: Does this wait happen when pause is enabled?
				SetWaitForPage();
			}
			break;
		}

		if (pause) {
			break;
		}

		if (*text_index == '\r') {
			// Not handled
			++text_index;
			continue;
		}

		if (*text_index == '\n') {
			if (text_index + 1 != text.end()) {
				if (!instant_speed && num_chars_printed_this_line == 0) {
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
			if (text_index != text.end()) {
				pause = true;
				new_page_after_pause = true;
			}
			//FIXME: Delays formfeed?
			break;
		}

		if (*text_index == Player::escape_char && std::distance(text_index, text.end()) > 1) {
			// Special message codes
			++text_index;

			auto ch = *text_index;

			int parameter;
			bool is_valid;

			switch (ch) {
			case 'c':
			case 'C':
				// Color
				parameter = ParseParameter(is_valid);
				text_color = parameter > 19 ? 0 : parameter;
				break;
			case 's':
			case 'S':
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
				++num_chars_printed_this_line;
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
				if (ch == Player::escape_char) {
					DrawGlyph(Player::escape_symbol, instant_speed);
				}
				break;
			}

			++text_index;
			continue;
		}

		if (*text_index == '$'
				   && std::distance(text_index, text.end()) > 1
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

int Window_Message::ParseParameter(bool& is_valid) {
	++text_index;

	if (text_index == text.end() ||
		*text_index != '[') {
		--text_index;
		is_valid = false;
		return 0;
	}

	++text_index; // Skip the [

	bool null_at_start = false;
	std::stringstream ss;
	for (;;) {
		if (text_index == text.end()) {
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
			while (text_index != text.end()) {
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

void Window_Message::DrawGlyph(const std::string& glyph, bool instant_speed) {
#ifdef EP_DEBUG_MESSAGE
	Output::Debug("Msg Draw Glyph %s %d", glyph.c_str(), instant_speed);
#endif
	contents->TextDraw(contents_x, contents_y, text_color, glyph);
	int glyph_width = Font::Default()->GetSize(glyph).width;
	contents_x += glyph_width;
	if (!instant_speed && glyph_width > 0) {
		// RPG_RT compatible for half-width (6) and full-width (12)
		// generalizes the algo for even bigger glyphs
		// FIXME: Verify RPG_RT on full width
		int width = (glyph_width - 1) / 6 + 1;
		SetWaitForCharacter(width);
	}
	num_chars_printed_this_line += width;
}

void Window_Message::UpdateCursorRect() {
	auto& pm = Game_Message::GetPendingMessage();

	if (index >= 0) {
		int x_pos = 2;
		int y_pos = (pm.GetChoiceStartLine() + index) * 16;
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
		} else if (text_index != text.end() && new_page_after_pause) {
			new_page_after_pause = false;
			InsertNewPage();
		}
	}
}

void Window_Message::InputChoice() {
	const auto& pm = Game_Message::GetPendingMessage();

	bool do_terminate = false;
	int choice_result = -1;

	if (Input::IsTriggered(Input::CANCEL)) {
		if (pm.GetChoiceCancelType() > 0) {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cancel));
			choice_result = pm.GetChoiceCancelType() - 1; // Cancel
			do_terminate = true;
		}
	} else if (Input::IsTriggered(Input::DECISION)) {
		if (!pm.IsChoiceEnabled(index)) {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
			return;
		}

		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
		choice_result = index;
		do_terminate = true;
	}

	if (do_terminate) {
		if (choice_result >= 0) {
			auto& continuation = pm.GetChoiceContinuation();
			if (continuation) {
				continuation(choice_result);
			}
		}
		TerminateMessage();
	}
}

void Window_Message::InputNumber() {
	if (Input::IsTriggered(Input::DECISION)) {
		const auto& pm = Game_Message::GetPendingMessage();
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
		Game_Variables.Set(pm.GetNumberInputVariable(), number_input_window->GetNumber());
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
				frames += !(num_chars_printed_this_line & 1);
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
