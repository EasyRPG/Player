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
#include "game_message.h"
#include "game_actor.h"
#include "game_party.h"
#include "game_player.h"
#include "game_battle.h"
#include "game_system.h"
#include "main_data.h"
#include "window_message.h"
#include "font.h"
#include "player.h"
#include "game_variables.h"
#include <lcf/data.h>
#include <lcf/reader_util.h>
#include "output.h"
#include "feature.h"

#include <cctype>

static Window_Message* window = nullptr;

void Game_Message::SetWindow(Window_Message* w) {
	window = w;
}

Window_Message* Game_Message::GetWindow() {
	return window;
}

int Game_Message::GetRealPosition() {
	if (Game_Battle::IsBattleRunning()) {
		if (Feature::HasRpg2kBattleSystem()) {
			return 2;
		}
		else {
			return 0;
		}
	}

	if (Main_Data::game_system->IsMessagePositionFixed()) {
		return Main_Data::game_system->GetMessagePosition();
	}
	else {
		// Move Message Box to prevent player hiding
		int disp = Main_Data::game_player->GetScreenY();

		switch (Main_Data::game_system->GetMessagePosition()) {
		case 0: // Up
			return disp > (16 * 7) ? 0 : 2;
		case 1: // Center
			if (disp <= 16 * 7) {
				return 2;
			}
			else if (disp >= 16 * 10) {
				return 0;
			}
			else {
				return 1;
			}
		default: // Down
			return disp >= (16 * 10) ? 0 : 2;
		};
	}
}

int Game_Message::WordWrap(StringView line, const int limit, const WordWrapCallback& callback) {
	return WordWrap(line, limit, callback, *Font::Default());
}

int Game_Message::WordWrap(StringView line, const int limit, const WordWrapCallback& callback, const Font& font) {
	int start = 0;
	int line_count = 0;

	do {
		int next = start;
		do {
			auto found = line.find(' ', next);
			if (found == std::string::npos) {
				found = line.size();
			}

			auto wrapped = line.substr(start, found - start);
			auto width = Text::GetSize(font, wrapped).width;
			if (width > limit) {
				if (next == start) {
					next = found + 1;
				}
				break;
			}

			next = found + 1;
		} while(next < static_cast<int>(line.size()));

		if (start == (next - 1)) {
			start = next;
			continue;
		}

		auto wrapped = line.substr(start, (next - 1) - start);

		callback(wrapped);
		line_count++;

		start = next;
	} while (start < static_cast<int>(line.size()));

	return line_count;
}

AsyncOp Game_Message::Update() {
	if (window) {
		window->Update();
		return window->GetAsyncOp();
	}
	return {};
}

void Game_Message::SetPendingMessage(PendingMessage&& pm) {
	if (window) {
		// This flag has no known use, but RPG_RT sets it whenever an event message command
		// spawns a message. We replicate it for save game compatibility.
		Main_Data::game_system->SetMessageEventMessageActive(pm.IsEventMessage());

		window->StartMessageProcessing(std::move(pm));
	}
}

bool Game_Message::IsMessagePending() {
	return window ? window->IsMessagePending() : false;
}

bool Game_Message::IsMessageActive() {
	return window ? !window->GetAllowNextMessage(false) : false;
}

bool Game_Message::CanShowMessage(bool foreground) {
	return window ? window->GetAllowNextMessage(foreground) : false;
}


static Game_Message::ParseParamResult ParseParamImpl(
		const char upper,
		const char lower,
		const char* iter,
		const char* end,
		uint32_t escape_char,
		bool skip_prefix,
		int max_recursion)
{
	if (!skip_prefix) {
		const auto begin = iter;
		if (iter == end) {
			return { begin, 0 };
		}
		auto ret = Utils::UTF8Next(iter, end);
		// Invalid commands
		if (ret.ch != escape_char) {
			return { begin, 0 };
		}
		iter = ret.next;
		if (iter == end || (*iter != upper && *iter != lower)) {
			return { begin, 0 };
		}
		++iter;
	}

	// If no bracket, RPG_RT will return 0.
	if (iter == end || *iter != '[') {
		return { iter, 0 };
	}

	int value = 0;
	++iter;
	bool stop_parsing = false;
	bool got_valid_number = false;

	while (iter != end && *iter != ']') {
		if (stop_parsing) {
			++iter;
			continue;
		}

		// Fast inline isdigit()
		if (*iter >= '0' && *iter <= '9') {
			value *= 10;
			value += (*iter - '0');
			++iter;
			got_valid_number = true;
			continue;
		}

		if (max_recursion > 0) {
			auto ret = Utils::UTF8Next(iter, end);
			auto ch = ret.ch;
			iter = ret.next;

			// Recursive variable case.
			if (ch == escape_char) {
				if (iter != end && (*iter == 'V' || *iter == 'v')) {
					++iter;

					auto ret = ParseParamImpl('V', 'v', iter, end, escape_char, true, max_recursion - 1);
					iter = ret.next;
					int var_val = Main_Data::game_variables->Get(ret.value);

					got_valid_number = true;

					// RPG_RT concatenates the variable value.
					int m = 10;
					if (value != 0) {
						while (m < var_val) {
							m *= 10;
						}
					}
					value = value * m + var_val;
					continue;
				}
			}
		}

		// If we hit a non-digit, RPG_RT will stop parsing until the next closing bracket.
		stop_parsing = true;
	}

	if (iter != end) {
		++iter;
	}

	// Actor 0 references the first party member
	if (upper == 'N' && value == 0 && got_valid_number) {
		auto* party = Main_Data::game_party.get();
		if (party->GetBattlerCount() > 0) {
			value = (*party)[0].GetId();
		}
	}

	return { iter, value };
}

Game_Message::ParseParamResult Game_Message::ParseVariable(const char* iter, const char* end, uint32_t escape_char, bool skip_prefix, int max_recursion) {
	return ParseParamImpl('V', 'v', iter, end, escape_char, skip_prefix, max_recursion - 1);
}

Game_Message::ParseParamResult Game_Message::ParseColor(const char* iter, const char* end, uint32_t escape_char, bool skip_prefix, int max_recursion) {
	return ParseParamImpl('C', 'c', iter, end, escape_char, skip_prefix, max_recursion);
}

Game_Message::ParseParamResult Game_Message::ParseSpeed(const char* iter, const char* end, uint32_t escape_char, bool skip_prefix, int max_recursion) {
	return ParseParamImpl('S', 's', iter, end, escape_char, skip_prefix, max_recursion);
}

Game_Message::ParseParamResult Game_Message::ParseActor(const char* iter, const char* end, uint32_t escape_char, bool skip_prefix, int max_recursion) {
	return ParseParamImpl('N', 'n', iter, end, escape_char, skip_prefix, max_recursion);
}


