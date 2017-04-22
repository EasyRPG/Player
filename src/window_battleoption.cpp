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
#include "data.h"
#include "color.h"
#include "cache.h"
#include "bitmap.h"
#include "input.h"
#include "game_system.h"
#include "game_temp.h"
#include "window_battleoption.h"

Window_BattleOption::Window_BattleOption(int x, int y, int width, int height) :
	Window_Base(x, y, width, height) {

	//commands.push_back(Data::terms.battle_fight);
	//commands.push_back(Data::terms.battle_auto);
	//commands.push_back(Data::terms.battle_escape);

	commands.push_back("Victory");
	commands.push_back("Defeat");
	commands.push_back("Escape");

	index = -1;
	top_row = 0;

	SetContents(Bitmap::Create(width - 16, height - 16));

	num_rows = contents->GetHeight() / 16;

	Refresh();
}

void Window_BattleOption::SetActive(bool active) {
	index = active ? 0 : -1;
	top_row = 0;
	Window::SetActive(active);
	Refresh();
}

void Window_BattleOption::Update() {
	Window_Base::Update();

	int num_commands = commands.size();

	if (active && num_commands > 0 && index >= 0) {
		if (Input::IsRepeated(Input::DOWN) || Input::IsTriggered(Input::SCROLL_DOWN)) {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cursor));
			index++;
		}

		if (Input::IsRepeated(Input::UP) || Input::IsTriggered(Input::SCROLL_UP)) {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cursor));
			index--;
		}

		index += num_commands;
		index %= num_commands;

		if (index < top_row)
			top_row = index;
		if (index > top_row + num_rows - 1)
			top_row = index - num_rows + 1;
	}

	UpdateCursorRect();
}

void Window_BattleOption::UpdateCursorRect() {
	if (index >= 0)
		SetCursorRect(Rect(0, (index - top_row) * 16, contents->GetWidth(), 16));
	else
		SetCursorRect(Rect());
}

void Window_BattleOption::Refresh() {
	contents->Clear();

	Font::SystemColor color = Font::ColorDefault;

	DrawItem(0, color);
	color = Game_Temp::battle_defeat_mode == 0 ? Font::ColorDisabled : Font::ColorDefault;
	DrawItem(1, color);
	color = Game_Temp::battle_escape_mode == 0 ? Font::ColorDisabled : Font::ColorDefault;
	DrawItem(2, color);
	/*for (int i = 0; i < (int) commands.size(); i++) {
		Font::SystemColor color = (i == 2 && Game_Temp::battle_escape_mode == 0)
			? Font::ColorDisabled
			: Font::ColorDefault;
		DrawItem(i, color);
	}*/
}

void Window_BattleOption::DrawItem(int index, Font::SystemColor color) {
	int y = 16 * (index - top_row);
	if (y < 0 || y + 16 > contents->GetHeight())
		return;
	contents->ClearRect(Rect(0, y, contents->GetWidth(), 16));
	contents->TextDraw(2, y + 2, color, commands[index]);
}

int Window_BattleOption::GetIndex() {
	return index;
}

void Window_BattleOption::SetIndex(int _index) {
	index = _index;
}
