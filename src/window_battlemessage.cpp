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
#include "window_battlemessage.h"
#include "bitmap.h"
#include "font.h"

Window_BattleMessage::Window_BattleMessage(int ix, int iy, int iwidth, int iheight) :
	Window_Base(ix, iy, iwidth, iheight), num_lines(0), next_message("") {
	SetContents(Bitmap::Create(width - 16, height - 16));
	contents->SetTransparentColor(windowskin->GetTransparentColor());
}


Window_BattleMessage::~Window_BattleMessage() {
}

void Window_BattleMessage::AddMessage(const std::string& message) {
	next_message = message;
}

void Window_BattleMessage::Update() {
	Window::Update();

	if (num_lines == 4)	{
		num_lines = 0;
		contents->Clear();
	}

	if (!next_message.empty()) {
		contents->TextDraw(0, 2 + num_lines * 16, Font::ColorDefault, next_message);
		++num_lines;
		next_message = "";
	}
}

bool Window_BattleMessage::AllLinesFilled() {
	return num_lines == 4;
}

void Window_BattleMessage::Clear() {
	contents->Clear();
	num_lines = 0;
}
