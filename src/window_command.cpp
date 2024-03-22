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
			max = std::max(max, Text::GetSize(*Font::Default(), commands[i]).width);
		}
		return max + 16;
	} else {
		return width;
	}
}

Window_Command::Window_Command(Scene* parent, std::vector<std::string> in_commands, int width, int max_item) :
	Window_Selectable(parent, 0, 0, CalculateWidth(in_commands, width), (max_item < 0 ? in_commands.size() : max_item) * 16 + 16)
{
	ReplaceCommands(std::move(in_commands));
}

void Window_Command::Refresh() {
	contents->Clear();
	for (int i = 0; i < item_max; i++) {
		DrawItem(i, IsItemEnabled(i) ? Font::ColorDefault : Font::ColorDisabled);
	}
}

void Window_Command::DrawItem(int index, Font::SystemColor color) {
	contents->ClearRect(Rect(0, menu_item_height * index, contents->GetWidth() - 0, menu_item_height));
	contents->TextDraw(0, menu_item_height * index + menu_item_height / 8, color, commands[index]);
}

void Window_Command::DisableItem(int i) {
	SetItemEnabled(i, false);
}

void Window_Command::EnableItem(int i) {
	SetItemEnabled(i, true);
}

void Window_Command::SetItemEnabled(int index, bool enabled) {
	DrawItem(index, enabled ? Font::ColorDefault : Font::ColorDisabled);
	commands_enabled[index] = enabled;
}

bool Window_Command::IsItemEnabled(int index) {
	if (index < 0 || index >= static_cast<int>(commands_enabled.size())) {
		return false;
	}

	return commands_enabled[index];
}

void Window_Command::SetItemText(unsigned index, StringView text) {
	if (index < commands.size()) {
		commands[index] = ToString(text);
		DrawItem(index, IsItemEnabled(index) ? Font::ColorDefault : Font::ColorDisabled);
	}
}

void Window_Command::ReplaceCommands(std::vector<std::string> in_commands) {
	commands = std::move(in_commands);
	commands_enabled.clear();
	commands_enabled.resize(commands.size(), true);
	index = 0;
	item_max = commands.size();
	const int num_contents = item_max > 0 ? item_max : 1;
	SetContents(Bitmap::Create(this->width - 16, num_contents * menu_item_height));
	SetTopRow(0);

	Refresh();
}
