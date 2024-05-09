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
#include <vector>
#include "window_stringview.h"
#include "bitmap.h"
#include "game_message.h"
#include "game_system.h"
#include "input.h"
#include "player.h"

Window_StringView::Window_StringView(int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(ix, iy, iwidth, iheight) {
	column_max = 1;
}

Window_StringView::~Window_StringView() {

}

void Window_StringView::SetDisplayData(StringView data) {
	display_data_raw = ToString(data);
}

std::string Window_StringView::GetDisplayData(bool eval_cmds) {
	if (eval_cmds)
		return PendingMessage::ApplyTextInsertingCommands(display_data_raw, Player::escape_char, Game_Message::CommandCodeInserter);
	return display_data_raw;
}

void Window_StringView::Refresh() {
	lines.clear();
	line_numbers.clear();
	line_count = 0;

	std::string value = GetDisplayData(cmd_eval);

	// determine line count
	size_t pos = 0;
	while ((pos = value.find("\n", pos)) != std::string::npos) {
		value = value.substr(pos + 1);
		pos = 0;
		line_count++;
	}
	if (!value.empty()) {
		line_count++;
	}

	//compute how many digits there are needed for the line numbers
	line_no_max_digits = std::log10(line_count) + 1;

	value = GetDisplayData(cmd_eval);

	//create vector of display lines
	pos = 0;
	int c = 0;
	int limit = 6 * (max_str_length - line_no_max_digits);

	while ((pos = value.find("\n", pos)) != std::string::npos) {
		std::string new_line = value.substr(0, pos);

		line_numbers.push_back(++c);
		if (auto_linebreak) {
			bool skipFirstNo = true;
			Game_Message::WordWrap(new_line, limit, [&](StringView line) {
				if (!skipFirstNo)
					line_numbers.push_back(0);
				skipFirstNo = false;
				lines.push_back(std::string(line));
			}, *Font::DefaultBitmapFont());
		} else {
			lines.push_back(new_line);
		}
		value = value.substr(pos + 1);
		pos = 0;
	}
	if (!value.empty()) {
		lines.push_back(value);
		line_numbers.push_back(++c);
	}

	item_max = lines.size() + top_lines_reserved;

	CreateContents();
	contents->Clear();
	DrawCmdLines();

	for (int i = 0; i < item_max - top_lines_reserved; ++i) {
		DrawLine(i);
	}
}

void Window_StringView::Update() {
	Window_Selectable::Update();
	if (active && index >= 0 && index <= 1 && Input::IsTriggered(Input::DECISION)) {
		if (index == 0)
			auto_linebreak = !auto_linebreak;
		else
			cmd_eval = !cmd_eval;

		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
		Refresh();
	}
}

void Window_StringView::DrawCmdLines() {
	Rect rect = GetItemRect(0);
	contents->ClearRect(rect);

	contents->TextDraw(rect.x, rect.y, Font::ColorHeal, "Automatic line break: ");
	contents->TextDraw(GetWidth() - 16, rect.y, Font::ColorCritical, auto_linebreak ? "[ON]" : "[OFF]", Text::AlignRight);

	rect = GetItemRect(1);
	contents->ClearRect(rect);

	contents->TextDraw(rect.x, rect.y, Font::ColorHeal, "Command evaluation: ");
	contents->TextDraw(GetWidth() - 16, rect.y, Font::ColorCritical, cmd_eval ? "[ON]" : "[OFF]", Text::AlignRight);
}

void Window_StringView::DrawLine(int index) {
	Rect rect = GetItemRect(index + top_lines_reserved);
	contents->ClearRect(rect);

	std::string line = lines[index];

	if (!line.empty()) {
		if (line_numbers[index]) {
			contents->TextDraw(rect.x, rect.y, Font::ColorDisabled, fmt::format("{:0" + std::to_string(line_no_max_digits) + "d}", line_numbers[index]));
		}
		contents->TextDraw(rect.x + line_no_max_digits * 6 + 6, rect.y, Font::ColorDefault, line);
	}
}
