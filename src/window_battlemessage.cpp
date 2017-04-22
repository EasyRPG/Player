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
	while (getline(smessage, line))
		lines.push_back(line);

	needs_refresh = true;
}

void Window_BattleMessage::Pop() {
	lines.pop_back();
	needs_refresh = true;
}

void Window_BattleMessage::Clear() {
	lines.clear();
	needs_refresh = true;
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
