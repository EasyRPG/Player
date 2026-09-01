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

Window_Command::Window_Command(std::vector<std::string> in_commands, int width, int max_item, std::vector<bool>* in_commands_enabled) :
	Window_Selectable(0, 0, CalculateWidth(in_commands, width), (max_item < 0 ? in_commands.size() : max_item) * 16 + 16)
{
	ReplaceCommands(std::move(in_commands), in_commands_enabled);
}

void Window_Command::Refresh() {
	contents->Clear();
	for (int i = 0; i < item_max; i++) {
		DrawItem(i, GetItemColor(index));
	}
}

void Window_Command::ClearItem(int index) {
	contents->ClearRect(Rect(0, menu_item_height * index, contents->GetWidth() - 0, menu_item_height));
}

void Window_Command::DrawItem(int index, Font::SystemColor color) {
	contents->TextDraw(0, menu_item_height * index + menu_item_height / 8, color, commands[index]);
}

void Window_Command::DisableItem(int i) {
	SetItemEnabled(i, false);
}

void Window_Command::EnableItem(int i) {
	SetItemEnabled(i, true);
}

void Window_Command::SetItemEnabled(int index, bool enabled) {
	if (index < commands.size() && commands_enabled[index] != enabled) {
		commands_enabled[index] = enabled;
		ClearItem(index);
		DrawItem(index, GetItemColor(index));
	}
}

bool Window_Command::IsItemEnabled(int index) {
	if (index < 0 || index >= static_cast<int>(commands_enabled.size())) {
		return false;
	}

	return commands_enabled[index];
}

void Window_Command::SetItemText(unsigned index, std::string_view text) {
	if (index < commands.size() && commands[index].compare(text) != 0) {
		commands[index] = ToString(text);
		ClearItem(index);
		DrawItem(index, GetItemColor(index));
	}
}

void Window_Command::ReplaceCommands(std::vector<std::string> in_commands, std::vector<bool>* in_commands_enabled) {
	bool redraw_all = false;

	// If the number of commands changes, then the size of the underlying bitmap 
	// changes, and everything will need to be redrawn.
	if (in_commands.size() != commands.size()) {
		redraw_all = true;
		commands.resize(in_commands.size(), "");
		commands_enabled.resize(in_commands.size(), true);
		item_max = in_commands.size();
		const int num_contents = item_max > 0 ? item_max : 1;
		SetContents(Bitmap::Create(this->width - 16, num_contents * menu_item_height));
	}
	
	index = 0;
	SetTopRow(0);

	for (int i = 0; i < in_commands.size(); i++) {
		// If NULL was passed for in_commands_enabled, all commands are enabled
		bool command_enabled_in = in_commands_enabled ? (*in_commands_enabled)[i] : true;
		if (redraw_all || command_enabled_in != commands_enabled[i] || commands[i].compare(in_commands[i]) != 0) {
			commands[i] = in_commands[i];
			commands_enabled[i] = command_enabled_in;
			if (!redraw_all) {
				ClearItem(i);
			}
			DrawItem(i, GetItemColor(i));
		}
	}
}

Font::SystemColor Window_Command::GetItemColor(int index) {
	return IsItemEnabled(index) ? Font::ColorDefault : Font::ColorDisabled;
};