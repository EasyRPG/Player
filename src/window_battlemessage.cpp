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
#include "game_message.h"
#include "window_battlemessage.h"
#include "bitmap.h"
#include "font.h"
#include "graphics.h"

#include <boost/next_prior.hpp>

Window_BattleMessage::Window_BattleMessage(int ix, int iy, int iwidth, int iheight) :
	Window_Message(ix, iy, iwidth, iheight),
	mode(Window_BattleMessage::Mode_Normal) {

}

void Window_BattleMessage::SetMessageMode(Window_BattleMessage::MessageMode new_mode) {
	mode = new_mode;
}

void Window_BattleMessage::UpdateMessage() {
	switch (mode) {
		case Window_BattleMessage::Mode_Normal:
			Window_Message::UpdateMessage();
			break;
		case Window_BattleMessage::Mode_EnemyEncounter:
			UpdateMessageEnemyEncounter();
			break;
		case Window_BattleMessage::Mode_Action:
			UpdateMessageAction();
			break;
	}
}

void Window_BattleMessage::UpdateMessageEnemyEncounter() {
	static int sleep_until = -1;
	if (sleep_until > -1) {
		if (Graphics::GetFrameCount() >= sleep_until) {
			// Sleep over
			sleep_until = -1;

			if (text_index == end) {
				TerminateMessage();
				return;
			}
		} else {
			return;
		}
	}

	while (sleep_until == -1) {
		if (line_count == 4) {
			InsertNewPage();
			break;
		}

		if (*text_index == '\n') {
			InsertNewLine();
			if (line_count == 4) {
				// Half second sleep
				sleep_until = Graphics::GetFrameCount() + 60 / 2;
			} else {
				// 1/10 second sleep
				sleep_until = Graphics::GetFrameCount() + 60 / 10;
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

		if (text_index == end) {
			sleep_until = Graphics::GetFrameCount() + 60 / 2;
		}
	}
}

void Window_BattleMessage::StartMessageProcessing() {
	switch (mode) {
		case Window_BattleMessage::Mode_Normal:
		case Window_BattleMessage::Mode_EnemyEncounter:
			Window_Message::StartMessageProcessing();
			break;
		case Window_BattleMessage::Mode_Action:
			text.clear();
			for (size_t i = 0; i < Game_Message::texts.size(); ++i) {
				std::string const line = Game_Message::texts[i];
				text.append(line + "\n");
			}
			item_max = Game_Message::choice_max;

			text_index = boost::u8_to_u32_iterator<std::string::const_iterator>(text.begin(), text.begin(), text.end());
			end = boost::u8_to_u32_iterator<std::string::const_iterator>(text.end(), text.begin(), text.end());
			break;
	}
}

void Window_BattleMessage::FinishMessageProcessing() {
	switch (mode) {
		case Window_BattleMessage::Mode_Normal:
			Window_Message::FinishMessageProcessing();
			break;
		case Window_BattleMessage::Mode_EnemyEncounter:
			Window_Message::FinishMessageProcessing();
			break;
		case Window_BattleMessage::Mode_Action:
			text.clear();
			text_index = boost::u8_to_u32_iterator<std::string::const_iterator>(text.begin(), text.begin(), text.end());
			end = boost::u8_to_u32_iterator<std::string::const_iterator>(text.end(), text.begin(), text.end());
			Game_Message::SemiClear();
			break;
	}
}

void Window_BattleMessage::UpdateMessageAction() {
	for (;;) {
		if (text_index == end) {
			FinishMessageProcessing();
			break;
		} else if (*text_index == '\n') {
			InsertNewLine();
		} else if (*text_index == '\r') {
			InsertNewPage();
			++text_index;
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
}
