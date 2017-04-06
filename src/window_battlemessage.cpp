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

Window_BattleMessage::Window_BattleMessage(int ix, int iy, int iwidth, int iheight) :
	Window_Base(ix, iy, iwidth, iheight),
	needs_refresh(true) {

	SetContents(Bitmap::Create(width - 16, height - 16));

	visible = false;
	SetZ(3001);
}

void Window_BattleMessage::Push(const std::string& message) {
	std::stringstream smessage(message);
	std::string line;
	while (getline(smessage, line)) {
		if (Player::IsRPG2kE()) {
			PushWordWrappedLine(line);
		}
		else {
			lines.push_back(line);
		}
	}

	needs_refresh = true;
}

int Window_BattleMessage::PushWordWrappedLine(const std::string& line) {
	FontRef font = Font::Default();

	int start = 0, lastfound = 0;
	int limit = GetWidth() - 24;
	int lineCount = 0;
	Rect size;

	do {
		lineCount++;
		int found = line.find(" ", start);
		std::string wrapped = line.substr(start, found - start);
		size = font->GetSize(wrapped);
		do {
			lastfound = found;
			found = line.find(" ", lastfound + 1);
			if (found == std::string::npos) {
				found = line.size();
			}
			wrapped = line.substr(start, found - start);
			size = font->GetSize(wrapped);
		} while (found < line.size() - 1 && size.width < limit);
		if (size.width < limit) {
			// It's end of the string, not a word-break
			lastfound = found;
		}
		lines.push_back(line.substr(start, lastfound - start));
		start = lastfound + 1;
	} while (start < line.size() && size.width >= limit);

	return lineCount;
}

void Window_BattleMessage::Pop() {
	lines.pop_back();
	needs_refresh = true;
}

void Window_BattleMessage::Clear() {
	lines.clear();
	needs_refresh = true;
}

bool Window_BattleMessage::NextPage() {
	int count = (int)lines.size();

	if (!count) {
		return false;
	}
	else if (count <= linesPerPage) {
		lines.clear();
		needs_refresh = true;
		return false;
	}
	else {
		lines.erase(lines.begin(), lines.begin() + linesPerPage);
		needs_refresh = true;
		return true;
	}
}

void Window_BattleMessage::Refresh() {
	contents->Clear();

	int contents_y = 2;

	std::vector<std::string>::const_iterator it;
	int i = 0;
	for (it = lines.begin(); it != lines.end(); ++it) {
		contents->TextDraw(0, contents_y, Font::ColorDefault, *it);
		contents_y += 16;

		++i;
		if (i > 3) {
			break;
		}
	}
}

void Window_BattleMessage::Update() {
	Window_Base::Update();
	if (needs_refresh) {
		needs_refresh = false;
		Refresh();
	}
}

int Window_BattleMessage::GetLineCount() {
	return (int)lines.size();
}

bool Window_BattleMessage::IsPageFilled() {
	return (int)lines.size() >= linesPerPage;
}
