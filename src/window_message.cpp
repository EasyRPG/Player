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
#include "utils.h"
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

// FIXME: Off by 1 bug in window base class
constexpr int message_animation_frames = 7;
constexpr int gold_window_width = 88;
constexpr int gold_window_height = 32;

//#define EP_DEBUG_MESSAGE
//#define EP_DEBUG_MESSAGE_TEXT

namespace {
#if defined(EP_DEBUG_MESSAGE) || defined(EP_DEBUG_MESSAGE_TEXT)
static int frame_offset = 0;

void DebugLogResetFrameCounter() {
	frame_offset = Main_Data::game_system->GetFrameCounter();
}
#else
void DebugLogResetFrameCounter() { }
#endif

#ifdef EP_DEBUG_MESSAGE
template <typename... Args>
void DebugLog(const char* fmt, Args&&... args) {
	int frames = Main_Data::game_system->GetFrameCounter() - frame_offset;
	Output::Debug(fmt, frames, std::forward<Args>(args)...);
}
#else

template <typename... Args>
void DebugLog(const char*, Args&&...) { }
#endif

#ifdef EP_DEBUG_MESSAGE_TEXT
template <typename... Args>
void DebugLogText(const char* fmt, Args&&... args) {
	int frames = Main_Data::game_system->GetFrameCounter() - frame_offset;
	Output::Debug(fmt, frames, std::forward<Args>(args)...);
}
#else

template <typename... Args>
void DebugLogText(const char*, Args&&...) { }
#endif
} //namespace

// C4428 is nonsense
#ifdef _MSC_VER
#pragma warning (disable : 4428)
#endif

Window_Message::Window_Message(int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(ix, iy, iwidth, iheight),
	number_input_window(new Window_NumberInput(0, 0)),
	gold_window(new Window_Gold(Player::screen_width - Player::menu_offset_x - gold_window_width, Player::menu_offset_y, gold_window_width, gold_window_height))
{
	SetContents(Bitmap::Create(width - 16, height - 16));

	// 2k3 transparent message boxes
	bool msg_transparent = Player::IsRPG2k3()
		// if the flag is set ..
		&& (lcf::Data::battlecommands.transparency == lcf::rpg::BattleCommands::Transparency_transparent)
		// if we're not in battle, or if we are in battle but not using mode A
		&& (!Game_Battle::IsBattleRunning() || lcf::Data::battlecommands.battle_type != lcf::rpg::BattleCommands::BattleType_traditional)
		// RPG_RT < 1.11 bug, map messages were not transparent if the battle type was mode A.
		&& (Player::IsRPG2k3E() || lcf::Data::battlecommands.battle_type != lcf::rpg::BattleCommands::BattleType_traditional);
	if (msg_transparent) {
		SetBackOpacity(160);
	}
	gold_window->SetBackOpacity(GetBackOpacity());

	SetVisible(false);
	// Above other windows
	SetZ(Priority_Window + 100);

	active = true;
	SetIndex(-1);
	text_color = Font::ColorDefault;

	number_input_window->SetVisible(false);

	gold_window->SetVisible(false);

	Main_Data::game_system->ClearMessageFace();
	Game_Message::SetWindow(this);
}

Window_Message::~Window_Message() {
	if (Game_Message::GetWindow() == this) {
		Game_Message::SetWindow(nullptr);
	}
}

void Window_Message::StartMessageProcessing(PendingMessage pm) {
	text.clear();
	pending_message = std::move(pm);

	if (!IsVisible()) {
		DebugLogResetFrameCounter();
	}
	DebugLog("{}: MSG START");

	if (!pending_message.IsActive()) {
		return;
	}

	const auto& lines = pending_message.GetLines();

	int num_lines = 0;
	auto append = [&](const std::string& line) {
		bool force_page_break = (!line.empty() && line.back() == '\f');

		text.append(line, 0, line.size() - force_page_break);
		if (line.empty() || text.back() != '\n') {
			text.push_back('\n');
		}
		++num_lines;

		if (num_lines == GetMaxLinesPerPage() || force_page_break) {
			text.push_back('\f');
			num_lines = 0;
		}
	};

	if (pending_message.IsWordWrapped()) {
		for (const std::string& line : lines) {
			/* TODO: don't take commands like \> \< into account when word-wrapping */
			Game_Message::WordWrap(
					line,
					width - 24,
					[&](StringView wrapped_line) {
						append(std::string(wrapped_line));
					}
			);
		}
	} else {
		for (const std::string& line : lines) {
			append(line);
		}
	}

	if (text.empty() || text.back() != '\f') {
		text.push_back('\f');
	}

	item_max = min(4, pending_message.GetNumChoices());

	text_index = text.data();

	DebugLog("{}: MSG TEXT \n{}", text);

	auto open_frames = (!IsVisible() && !Game_Battle::IsBattleRunning()) ? message_animation_frames : 0;
	SetOpenAnimation(open_frames);
	DebugLog("{}: MSG START OPEN {}", open_frames);

	InsertNewPage();
}

void Window_Message::OnFinishPage() {
	DebugLog("{}: FINISH PAGE");

	if (pending_message.GetNumChoices() > 0) {
		StartChoiceProcessing();
	} else if (pending_message.HasNumberInput()) {
		StartNumberInputProcessing();
	} else if (!kill_page) {
		DebugLog("{}: SET PAUSE");
		SetPause(true);
	}

	line_count = 0;
	kill_page = false;
	line_char_counter = 0;
}

void Window_Message::StartChoiceProcessing() {
	SetIndex(0);
}

void Window_Message::StartNumberInputProcessing() {
	number_input_window->SetMaxDigits(pending_message.GetNumberInputDigits());
	if (IsFaceEnabled() && !Main_Data::game_system->IsMessageFaceRightPosition()) {
		number_input_window->SetX(Player::message_box_offset_x + LeftMargin + FaceSize + RightFaceMargin);
	} else {
		number_input_window->SetX(x);
	}
	number_input_window->SetY(y + contents_y - 2);
	number_input_window->SetActive(true);
	if (IsVisible() && !IsOpeningOrClosing()) {
		number_input_window->SetVisible(true);
	}
	number_input_window->Update();
}

void Window_Message::ShowGoldWindow() {
	if (Game_Battle::IsBattleRunning()) {
		return;
	}
	if (!gold_window->IsVisible()) {
		gold_window->SetY(y == 0 ? Player::screen_height - Player::menu_offset_y - 32 : Player::menu_offset_y);
		gold_window->SetOpenAnimation(message_animation_frames);
	} else if (gold_window->IsClosing()) {
		gold_window->SetOpenAnimation(0);
	}
	gold_window->Refresh();
}

void Window_Message::InsertNewPage() {
	DebugLog("{}: MSG NEW PAGE");
	// Cancel pending face requests for async
	// Otherwise they render on the wrong page
	face_request_ids.clear();

	contents->Clear();
	SetIndex(-1);
	SetPause(false);
	number_input_window->SetActive(false);
	number_input_window->SetVisible(false);
	kill_page = false;
	line_count = 0;
	text_color = Font::ColorDefault;
	speed = 1;
	kill_page = false;
	instant_speed = false;
	prev_char_printable = false;
	prev_char_waited = true;

	// Position the message box vertically
	// Game_Message::GetRealPosition() specify top/middle/bottom
	if (Game_Message::GetRealPosition() == 0) {
		y = Player::menu_offset_y;
	}
	else if (Game_Message::GetRealPosition() == 1) {
		y = (Player::screen_height / (float)2) - (MESSAGE_BOX_HEIGHT / (float)2);
	}
	else if (Game_Message::GetRealPosition() == 2) {
		y = Player::screen_height - Player::menu_offset_y - MESSAGE_BOX_HEIGHT ;
	}

	if (Main_Data::game_system->IsMessageTransparent()) {
		SetOpacity(0);
		gold_window->SetBackOpacity(0);
	} else {
		SetOpacity(255);
		gold_window->SetBackOpacity(GetBackOpacity());
	}

	if (IsFaceEnabled()) {
		if (!Main_Data::game_system->IsMessageFaceRightPosition()) {
			contents_x = LeftMargin + FaceSize + RightFaceMargin;
			DrawFace(Main_Data::game_system->GetMessageFaceName(), Main_Data::game_system->GetMessageFaceIndex(), LeftMargin, TopMargin, Main_Data::game_system->IsMessageFaceFlipped());
		} else {
			contents_x = 0;
			DrawFace(Main_Data::game_system->GetMessageFaceName(), Main_Data::game_system->GetMessageFaceIndex(), 248, TopMargin, Main_Data::game_system->IsMessageFaceFlipped());
		}
	} else {
		contents_x = 0;
	}

	if (pending_message.GetChoiceStartLine() == 0 && pending_message.HasChoices()) {
		contents_x += 12;
	}

	contents_y = 2;

	if (pending_message.GetNumberInputStartLine() == 0 && pending_message.HasNumberInput()) {
		// If there is an input window on the first line
		StartNumberInputProcessing();
	}
	line_char_counter = 0;

	if (pending_message.ShowGoldWindow()) {
		ShowGoldWindow();
	} else {
		// If first character is gold, the gold window appears immediately and animates open with the main window.
		auto tret = Utils::TextNext(text_index, (text.data() + text.size()), Player::escape_char);
		if (tret && tret.is_escape && tret.ch == '$') {
			ShowGoldWindow();
		}
	}

}

void Window_Message::InsertNewLine() {
	DebugLog("{}: MSG NEW LINE");
	if (IsFaceEnabled() && !Main_Data::game_system->IsMessageFaceRightPosition()) {
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
	prev_char_printable = false;
	prev_char_waited = true;
}

void Window_Message::FinishMessageProcessing() {
	DebugLog("{}: FINISH MSG");
	text.clear();
	text_index = text.data();

	SetPause(false);
	kill_page = false;
	line_char_counter = 0;
	SetIndex(-1);

	pending_message = {};

	auto close_frames = Game_Battle::IsBattleRunning() ? 0 : message_animation_frames;

	if (number_input_window->IsVisible()) {
		number_input_window->SetActive(false);
		number_input_window->SetVisible(false);
	}

	SetCloseAnimation(close_frames);
	close_started_this_frame = true;
	DebugLog("{}: MSG START CLOSE {}", close_frames);

	// RPG_RT updates window open/close at the end of the main loop.
	// To simulate this timing, we run base class Update() again once
	// to animate the closing by 1 frame.
	Window::Update();

	if (gold_window->IsVisible()) {
		gold_window->SetCloseAnimation(close_frames);
		gold_window->Update();
	}
}

void Window_Message::ResetWindow() {

}

void Window_Message::Update() {
	aop = {};
	if (IsOpening()) { DebugLog("{}: MSG OPENING"); }
	if (IsClosing()) { DebugLog("{}: MSG CLOSING"); }

	close_started_this_frame = false;
	close_finished_this_frame = false;

	const bool was_closing = IsClosing();

	Window_Selectable::Update();
	number_input_window->Update();
	gold_window->Update();

	if (was_closing && !IsClosing()) {
		close_finished_this_frame = true;
	}

	if (!IsVisible()) {
		return;
	}

	if (IsOpeningOrClosing()) {
		return;
	}

	if (wait_count > 0) {
		DebugLog("{}: MSG WAIT {}", wait_count);
		--wait_count;
		return;
	}

	if (GetPause()) {
		DebugLog("{}: MSG PAUSE");
		WaitForInput();

		if (GetPause()) {
			return;
		}
	}

	if (GetIndex() >= 0) {
		DebugLog("{}: MSG CHOICE");
		InputChoice();
		if (GetIndex() >= 0) {
			return;
		}
	}

	if (number_input_window->GetActive()) {
		DebugLog("{}: MSG NUMBER");
		InputNumber();
		if (number_input_window->GetActive()) {
			return;
		}
	}

	DebugLog("{}: MSG UPD");
	UpdateMessage();
}

void Window_Message::UpdateMessage() {
	// Message Box Show Message rendering loop
	bool instant_speed_forced = false;

	if (Player::debug_flag && Input::IsPressed(Input::SHIFT)) {
		instant_speed = true;
		instant_speed_forced = true;
	}

	auto system = Cache::SystemOrBlack();
	auto font = Font::Default();

	while (true) {
		const auto* end = text.data() + text.size();

		if (wait_count > 0) {
			DebugLog("{}: MSG WAIT LOOP {}", wait_count);
			--wait_count;
			break;
		}

		if (!shape_ret.empty()) {
			if (!DrawGlyph(*font, *system, shape_ret[0])) {
				continue;
			}

			shape_ret.erase(shape_ret.begin());
			continue;
		}

		if (GetPause() || GetIndex() >= 0 || number_input_window->GetActive()) {
			break;
		}

		if (text_index == end) {
			FinishMessageProcessing();
			break;
		}

		auto tret = Utils::TextNext(text_index, end, Player::escape_char);
		auto text_prev = text_index;
		text_index = tret.next;

		if (EP_UNLIKELY(!tret)) {
			continue;
		}

		const auto ch = tret.ch;
		if (tret.is_exfont) {
			if (!DrawGlyph(*font, *system, ch, true)) {
				text_index = text_prev;
			}
			continue;
		}

		if (ch == '\f') {
			if (text_index != end) {
				InsertNewPage();
				SetWait(1);
			}
			continue;
		}

		if (ch == '\n') {
			int wait_frames = 0;
			bool end_page = (*text_index == '\f');

			if (!instant_speed) {
				if (!prev_char_printable) {
					wait_frames += 1 + end_page;
				}
			} else if (end_page) {
				// When the page ends and speed is instant, RPG_RT always waits 2 frames.
				wait_frames += 2;
			}

			InsertNewLine();

			if (end_page) {
				OnFinishPage();
			}
			SetWait(wait_frames);

			if (instant_speed && !instant_speed_forced) {
				// instant_speed stops at the end of the line
				// unless it was triggered by the shift key.
				instant_speed = false;
			}
			continue;
		}

		if (Utils::IsControlCharacter(ch)) {
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
					text_index = pres.next;
					DebugLogText("{}: MSG Color \\c[{}]", value);
					SetWaitForNonPrintable(0);
					text_color = value > 19 ? 0 : value;
				}
				break;
			case 's':
			case 'S':
				{
					// Speed modifier
					auto pres = Game_Message::ParseSpeed(text_index, end, Player::escape_char, true);
					text_index = pres.next;
					DebugLogText("{}: MSG Speed \\s[{}]", pres.value);
					SetWaitForNonPrintable(0);
					speed = Utils::Clamp(pres.value, 1, 20);
				}
				break;
			case '_':
				// Insert half size space
				contents_x += Text::GetSize(*Font::Default(), " ").width / 2;
				DebugLogText("{}: MSG HalfWait \\_");
				SetWaitForCharacter(1);
				break;
			case '$':
				// Show Gold Window
				ShowGoldWindow();
				DebugLogText("{}: MSG Gold \\$");
				SetWaitForNonPrintable(speed);
				break;
			case '!':
				// Text pause
				DebugLogText("{}: MSG Pause \\!");
				SetWaitForNonPrintable(0);
				SetPause(true);
				break;
			case '^':
				// Force message close
				// The close happens at the end of the message, not where
				// the ^ is encountered
				DebugLogText("{}: MSG Kill Page \\^");
				kill_page = true;
				SetWaitForNonPrintable(speed);
				break;
			case '>':
				// Instant speed start
				DebugLogText("{}: MSG Instant Speed Start \\>");
				SetWaitForNonPrintable(0);
				instant_speed = true;
				break;
			case '<':
				// Instant speed stop - also cancels shift key and forces a delay.
				instant_speed = false;
				instant_speed_forced = false;
				DebugLogText("{}: MSG Instant Speed Stop \\<");
				SetWaitForNonPrintable(speed);
				break;
			case '.':
				// 1/4 second sleep
				// Despite documentation saying 1/4 second, RPG_RT waits for 16 frames.
				// RPG_RT also has a bug(??) where speeds >= 17 slow this down by 1 more frame per speed.
				SetWaitForNonPrintable(16 + Utils::Clamp(speed - 16, 0, 4));
				DebugLogText("{}: MSG Quick Sleep \\.");
				break;
			case '|':
				// Second sleep
				// Despite documentation saying 1 second, RPG_RT waits for 61 frames.
				SetWaitForNonPrintable(61);
				DebugLogText("{}: MSG Sleep \\|");
				break;
			default:
				// Unknown characters will not display anything but do wait.
				SetWaitForNonPrintable(speed);
				break;
			}
			continue;
		}

		if (font->CanShape()) {
			assert(shape_ret.empty());

			auto text_index_shape = text_index;
			std::u32string text32;
			text32 += ch;

			while (true) {
				tret = Utils::TextNext(text_index_shape, end, Player::escape_char);

				if (EP_UNLIKELY(!tret)) {
					break;
				}

				auto text_prev_shape = text_index_shape;
				text_index_shape = tret.next;
				auto chs = tret.ch;

				if (text_index_shape == end || tret.is_exfont || tret.is_escape || Utils::IsControlCharacter(chs)) {
					text_index = text_prev_shape;
					break;
				}

				text32 += tret.ch;
			}

			shape_ret = font->Shape(text32);
			continue;
		} else {
			if (!DrawGlyph(*font, *system, ch, false)) {
				text_index = text_prev;
				continue;
			}
		}
	}
}

bool Window_Message::DrawGlyph(Font& font, const Bitmap& system, char32_t glyph, bool is_exfont) {
	if (is_exfont) {
		DebugLogText("{}: MSG DrawGlyph Exfont {}", static_cast<uint32_t>(glyph));
	} else {
		if (glyph < 128) {
			DebugLogText("{}: MSG DrawGlyph ASCII {}", static_cast<char>(glyph));
		} else {
			DebugLogText("{}: MSG DrawGlyph UTF32 {#:X}", static_cast<uint32_t>(glyph));
		}
	}

	// RPG_RT compatible for half-width (6) and full-width (12)
	// generalizes the algo for even bigger glyphs
	// FIXME: When using Freetype this can cause slow rendering speeds due to dynamic width
	auto get_width = [](int w) {
		return (w > 0) ? (w - 1) / 6 + 1 : 0;
	};

	// Wide characters cause an extra wait if the last printed character did not wait.
	if (prev_char_printable && !prev_char_waited) {
		auto& wide_font = is_exfont ? *Font::exfont : font;
		auto rect = wide_font.GetSize(glyph);
		auto width = get_width(rect.width);
		if (width >= 2) {
			prev_char_waited = true;
			++line_char_counter;
			SetWait(1);
			return false;
		}
	}

	auto rect = Text::Draw(*contents, contents_x, contents_y, font, system, text_color, glyph, is_exfont);

	int glyph_width = rect.x;
	contents_x += glyph_width;
	int width = get_width(glyph_width);
	SetWaitForCharacter(width);

	return true;
}

bool Window_Message::DrawGlyph(Font& font, const Bitmap& system, const Font::ShapeRet& shape) {
	// RPG_RT compatible for half-width (6) and full-width (12)
	// generalizes the algo for even bigger glyphs
	// FIXME: This can cause slow rendering speeds for complex shapes
	auto get_width = [](int w) {
		return (w > 0) ? (w - 1) / 6 + 1 : 0;
	};

	DebugLogText("{}: MSG DrawGlyph Shape {}, {}", static_cast<uint32_t>(shape.code), get_width(shape.advance.x));

	// Wide characters cause an extra wait if the last printed character did not wait.
	if (prev_char_printable && !prev_char_waited) {
		auto width = get_width(shape.advance.x);
		if (width >= 2) {
			prev_char_waited = true;
			++line_char_counter;
			SetWait(1);
			return false;
		}
	}

	auto rect = font.Render(*contents, contents_x, contents_y, system, text_color, shape);

	int glyph_width = rect.x;
	contents_x += glyph_width;
	int width = get_width(glyph_width);
	SetWaitForCharacter(width);

	return true;
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
		int width = contents->GetWidth() - 4;

		if (IsFaceEnabled()) {
			if (!Main_Data::game_system->IsMessageFaceRightPosition()) {
				x_pos += LeftMargin + FaceSize + RightFaceMargin;
			}
			width = width - LeftMargin - FaceSize - RightFaceMargin;
		}

		SetCursorRect(Rect(x_pos, y_pos, width, 16));
	} else {
		SetCursorRect(Rect());
	}
}

void Window_Message::WaitForInput() {
	if (Input::IsTriggered(Input::DECISION) ||
			Input::IsTriggered(Input::CANCEL)) {
		SetPause(false);
	}
}

void Window_Message::InputChoice() {
	int choice_result = -1;

	if (Input::IsTriggered(Input::CANCEL)) {
		if (pending_message.GetChoiceCancelType() > 0) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
			choice_result = pending_message.GetChoiceCancelType() - 1; // Cancel
		}
	} else if (Input::IsTriggered(Input::DECISION)) {
		if (!pending_message.IsChoiceEnabled(index)) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
			return;
		}

		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
		choice_result = index;
	}

	if (choice_result >= 0) {
		auto& continuation = pending_message.GetChoiceContinuation();
		if (continuation) {
			aop = continuation(choice_result);
		}
		// This disables choices
		index = -1;
	}
}

void Window_Message::InputNumber() {
	number_input_window->SetVisible(true);
	if (Input::IsTriggered(Input::DECISION)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
		Main_Data::game_variables->Set(pending_message.GetNumberInputVariable(), number_input_window->GetNumber());
		Game_Map::SetNeedRefresh(true);
		number_input_window->SetNumber(0);
		number_input_window->SetActive(false);
	}
}

void Window_Message::SetWaitForNonPrintable(int frames) {
	if (!instant_speed) {
		if (speed <= 1) {
			frames += (line_char_counter & 1);
		}
		SetWait(frames);
	}
	prev_char_waited = (instant_speed || frames > 0);
	prev_char_printable = false;
	// Non printables only contribute to character count after the first printable..
	if (line_char_counter > 0) {
		IncrementLineCharCounter(1);
	}
}

void Window_Message::SetWaitForCharacter(int width) {
	int frames = 0;
	if (!instant_speed && width > 0) {
		bool is_last_for_page;
		if (!shape_ret.empty()) {
			is_last_for_page = (shape_ret.size() == 1) && (
				(text.data() + text.size() - text_index) <= 1 || (*text_index == '\n' && *(text_index + 1) == '\f'));
		} else {
			is_last_for_page = (text.data() + text.size() - text_index) <= 1 || (*text_index == '\n' && *(text_index + 1) == '\f');
		}

		if (is_last_for_page) {
			// RPG_RT always waits 2 frames for last character on the page.
			// FIXME: Exfonts / wide last on page?
			DebugLogText("{}: is_last_for_page");
			frames = 2;
		} else {
			if (speed > 1) {
				frames = speed * width / 2 + 1;
			} else {
				frames = width / 2;
				if (width & 1) {
					bool is_last_for_line;
					if (!shape_ret.empty()) {
						is_last_for_line = shape_ret.size() == 1 && (*text_index == '\n');
					} else {
						is_last_for_line = (*text_index == '\n');
					}
					if (is_last_for_line) {
						DebugLogText("{}: is_last_for_line");
					}

					// RPG_RT waits for every even character. Also always waits
					// for the last character.
					frames += (line_char_counter & 1) || is_last_for_line;
				}
			}
		}
	}
	prev_char_waited = (instant_speed || frames > 0);
	prev_char_printable = true;
	SetWait(frames);
	IncrementLineCharCounter(width);
}

void Window_Message::SetWait(int frames) {
	assert(speed >= 1 && speed <= 20);
	DebugLogText("{}: MSG SetWait {}", frames);
	wait_count = frames;
}

bool Window_Message::IsFaceEnabled() const {
	return pending_message.IsFaceEnabled() && !Main_Data::game_system->GetMessageFaceName().empty();
}

