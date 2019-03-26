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
#include <sstream>
#include "game_message.h"
#include "player.h"
#include "window_battlemessage.h"
#include "bitmap.h"
#include "font.h"
#include "utils.h"
#include "output.h"

Window_BattleMessage::Window_BattleMessage(int ix, int iy, int iwidth, int iheight) :
	Window_Base(ix, iy, iwidth, iheight)
{
	SetContents(Bitmap::Create(width - 20, height - 16));

	visible = false;
	// Above other windows but below the messagebox
	SetZ(Priority_Window + 50);
}

void Window_BattleMessage::Push(const std::string& message) {
#ifdef EP_DEBUG_BATTLE2K_MESSAGE
	Output::Debug("Battle2k Message Push \"%s\"", message.c_str());
#endif
	Utils::ForEachLine(message, [this](const std::string& line)
			{ PushLine(line); });
}

void Window_BattleMessage::PushLine(const std::string& line) {
	if (Player::IsRPG2kE()) {
		Game_Message::WordWrap(
				line,
				GetWidth() - 20,
				[this](const std::string& wrap_line) {
					lines.push_back(wrap_line);
				}
				);
	}
	else {
		lines.push_back(line);
	}

	needs_refresh = true;
}

void Window_BattleMessage::PushWithSubject(const std::string& message, const std::string& subject) {
	if (Player::IsRPG2kE()) {
		Push(Utils::ReplacePlaceholders(
			message,
			{'S'},
			{subject}
		));
	}
	else {
		Push(subject + message);
	}
	needs_refresh = true;
}

void Window_BattleMessage::Pop() {
#ifdef EP_DEBUG_BATTLE2K_MESSAGE
	Output::Debug("Battle2k Message Pop");
#endif
	lines.pop_back();
	needs_refresh = true;
	if (GetIndex() > (int)lines.size()) {
		SetIndex(lines.size());
	}
}

void Window_BattleMessage::PopUntil(int line_number) {
#ifdef EP_DEBUG_BATTLE2K_MESSAGE
	Output::Debug("Battle2k Message PopUntil %d", line_number);
#endif
	while (static_cast<int>(lines.size()) > line_number) {
		lines.pop_back();
	}
	needs_refresh = true;
}

void Window_BattleMessage::Clear() {
#ifdef EP_DEBUG_BATTLE2K_MESSAGE
	Output::Debug("Battle2k Message Clear");
#endif
	lines.clear();
	SetIndex(0);
	needs_refresh = true;
}

void Window_BattleMessage::ScrollToEnd() {
	const auto old_index = index;
	if (lines.size() > linesPerPage) {
		index = lines.size() - linesPerPage;
	} else {
		index = 0;
	}
	needs_refresh |= (index != old_index);
}

void Window_BattleMessage::Refresh() {
	contents->Clear();

	int i = GetIndex();
	const auto ed = std::min(i + linesPerPage, (int)lines.size());
	int y = 2;
	for (; i < ed; ++i) {
		contents->TextDraw(0, y, Font::ColorDefault, lines[i]);
		y+= 16;
	}
	needs_refresh = false;
}

void Window_BattleMessage::Update() {
	Window_Base::Update();
	if (needs_refresh) {
		Refresh();
	}
}

int Window_BattleMessage::GetLineCount() {
	return (int)lines.size();
}

bool Window_BattleMessage::IsPageFilled() {
	return (lines.size() - GetIndex()) >= linesPerPage;
}
