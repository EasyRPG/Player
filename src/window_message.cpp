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

#include "compiler.h"
#include "window_message.h"
#include "game_actors.h"
#include "game_map.h"
#include "game_message.h"
#include "game_party.h"
#include "game_system.h"
#include "game_variables.h"
#include "input.h"
#include "output.h"
#include "player.h"
#include "util_macro.h"
#include "game_battle.h"
#include "bitmap.h"
#include "font.h"
#include "cache.h"
#include "text.h"

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

	SetVisible(false);
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

void Window_Message::StartMessageProcessing(PendingMessage pm) {
	contents->Clear();
	pending_message = std::move(pm);
	allow_next_message = false;

	const auto& lines = pending_message.GetLines();
	if (!(pending_message.NumLines() > 0 || pending_message.HasNumberInput())) {
		return;
	}

	text.clear();
	auto append = [&](const std::string& line) {
		text.append(line);
		if (text.empty() || (text.back() != '\n' && text.back() != '\f')) {
			text.append(1, '\n');
		}
	};
	if (pending_message.IsWordWrapped()) {
		for (const std::string& line : lines) {
			/* TODO: don't take commands like \> \< into account when word-wrapping */
			Game_Message::WordWrap(
					line,
					width - 24,
					[&](const std::string& wrapped_line) {
						append(wrapped_line);
					}
			);
		}
	} else {
		for (const std::string& line : lines) {
			append(line);
		}
	}
	item_max = min(4, pending_message.GetNumChoices());

	text_index = text.data();

	// If we're displaying a new message, reset the closing animation.
	if (closing) {
		SetCloseAnimation(Game_Battle::IsBattleRunning() ? 0 : message_animation_frames);
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
	text_index = text.data();
}

void Window_Message::StartChoiceProcessing() {
	active = true;
	index = 0;
}

void Window_Message::StartNumberInputProcessing() {
	number_input_window->SetMaxDigits(pending_message.GetNumberInputDigits());
	if (IsFaceEnabled() && !Game_Message::IsFaceRightPosition()) {
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
	if (!gold_window->IsVisible() && !Game_Battle::IsBattleRunning()) {
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

	if (IsFaceEnabled()) {
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
	if (IsFaceEnabled() && !Game_Message::IsFaceRightPosition()) {
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

	if (number_input_window->IsVisible()) {
		number_input_window->SetActive(false);
		number_input_window->SetVisible(false);
	}

	if (gold_window->IsVisible()) {
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
		} else if (number_input_window->IsVisible()) {
			InputNumber();
		} else if (!text.empty()) {
			if (!IsVisible()) {
				// The MessageBox is not open yet but text output is needed
				// Open and Close Animations are skipped in battle
				SetOpenAnimation(Game_Battle::IsBattleRunning() ? 0 : message_animation_frames);
			} else if (closing) {
				// If a message was requested while closing, cancel it and display the message immediately.
				SetOpenAnimation(0);
			} else {
				//Handled after base class updates.
				if (text_index != &*text.end()) {
					update_message_processing = true;
				}
				if (text_index == &*text.end() && wait_count <= 0) {
					FinishMessageProcessing();
				}
			}
		}

		if (!Game_Message::IsMessagePending() && IsVisible() && !closing) {
			// Start the closing animation
			SetCloseAnimation(Game_Battle::IsBattleRunning() ? 0 : message_animation_frames);
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

	auto system = Cache::SystemOrBlack();
	auto font = Font::Default();

	while (true) {
		const auto* end = &*text.end();

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

		auto tret = Utils::TextNext(text_index, end, Player::escape_char);
		text_index = tret.next;

		if (EP_UNLIKELY(!tret)) {
			continue;
		}

		const auto ch = tret.ch;
		if (tret.is_exfont) {
			DrawGlyph(*font, *system, ch, instant_speed, true);
			continue;
		}

		if (ch == '\n') {
			if (text_index != end) {
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
			continue;
		}

		if (ch == '\f') {
			// Used by our code to inject form feeds.
			instant_speed = false;

			if (text_index != end) {
				SetPause(true);
				new_page_after_pause = true;
			}
			//FIXME: Delays formfeed?
			break;
		}

		if (std::iscntrl(static_cast<unsigned char>(ch))) {
			// control characters not handled
			continue;
		}

		if (tret.is_escape && ch != Player::escape_char) {
			// Special message codes
			switch (ch) {
			case 'c':
			case 'C':
				{
					// Color
					auto pres = Game_Message::ParseColor(text_index, end, Player::escape_char, true);
					auto value = pres.value;
					text_color = value > 19 ? 0 : value;
					text_index = pres.next;
				}
				break;
			case 's':
			case 'S':
				{
					// Speed modifier
					auto pres = Game_Message::ParseSpeed(text_index, end, Player::escape_char, true);
					speed = Utils::Clamp(pres.value, 1, 20);
					text_index = pres.next;
				}
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
			default:
				break;
			}
			continue;
		}

		DrawGlyph(*font, *system, ch, instant_speed, false);
	}
}

void Window_Message::DrawGlyph(Font& font, const Bitmap& system, char32_t glyph, bool instant_speed, bool is_exfont) {
#ifdef EP_DEBUG_MESSAGE
	Output::Debug("Msg Draw Glyph %d %d", glyph, instant_speed);
#endif
	auto rect = Text::Draw(*contents, contents_x, contents_y, font, system, text_color, glyph, is_exfont);

	int glyph_width = rect.width;
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

void Window_Message::UpdateCursorRect() {
	if (index >= 0) {
		int x_pos = 2;
		int y_pos = (pending_message.GetChoiceStartLine() + index) * 16;
		int width = contents->GetWidth();

		if (IsFaceEnabled()) {
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
		} else if (text_index != &*text.end() && new_page_after_pause) {
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

bool Window_Message::IsFaceEnabled() const {
	return pending_message.IsFaceEnabled() && !Game_Message::GetFaceName().empty();
}

