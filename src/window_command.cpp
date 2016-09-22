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
#include "window_command.h"
#include "color.h"
#include "bitmap.h"
#include "util_macro.h"

static int CalculateWidth(const std::vector<std::string>& commands, int width) {
	if (width < 0) {
		int max = 0;
		for (size_t i = 0; i < commands.size(); ++i) {
			max = std::max(max, Font::Default()->GetSize(commands[i]).width);
		}
		return max + 16;
	} else {
		return width;
	}
}

Window_Command::Window_Command(const std::vector<std::string>& commands, int width, int max_item) :
	Window_Selectable(0, 0, CalculateWidth(commands, width), (max_item < 0 ? commands.size() : max_item) * 16 + 16),
	commands(commands) {

	index = 0;
	item_max = commands.size();

	SetContents(Bitmap::Create(this->width - 16, item_max * 16));

	Refresh();
}

void Window_Command::Refresh() {
	contents->Clear();
	for (int i = 0; i < item_max; i++) {
		DrawItem(i, Font::ColorDefault);
	}
}

void Window_Command::DrawItem(int index, Font::SystemColor color) {
	contents->ClearRect(Rect(0, 16 * index, contents->GetWidth() - 0, 16));
	contents->TextDraw(0, 16 * index + 2, color, commands[index]);
}

void Window_Command::DisableItem(int i) {
	DrawItem(i, Font::ColorDisabled);
}

void Window_Command::SetItemText(unsigned index, std::string const& text) {
	if (index < commands.size()) {
		commands[index] = text;
		DrawItem(index, Font::ColorDefault);
	}
}
