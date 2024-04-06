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
#include <iomanip>
#include <sstream>
#include "window_target.h"
#include "window_command.h"
#include <input.h>
#include "main_data.h"
#include <baseui.h>
#include "game_system.h"

static int CalculateWidth(const std::vector<std::string>& commands, int width) {
	if (width < 0) {
		int max = 0;
		for (size_t i = 0; i < commands.size(); ++i) {
			max = std::max(max, Text::GetSize(*Font::Default(), commands[i]).width);
		}
		return max + 16;
	}
	else {
		return width;
	}
}

Window_Target::Window_Target(Scene* parent, std::vector<std::string> in_commands, int width, int max_item) :
	Window_Command(parent, in_commands, width, max_item)
{
	ReplaceCommands(std::move(in_commands));
}

void Window_Target::Update() {
	// Window Selectable update logic skipped on purpose
	// (breaks up/down-logic)
	Window_Command::Update();

	if (Input::GetUseMouseButton()) {
		if (GetActive() && IsVisible()) {
			Point mouse_pos = Input::GetMousePosition();
			int index = CursorHitTest({ mouse_pos.x - GetX(), mouse_pos.y - GetY() });
			if (index >= 0)
				DisplayUi->ChangeCursor(1);
			if (index >= 0 && index != GetIndex() && Input::MouseMoved()) {
				// FIXME: Index changed callback?
				SetIndex(index);
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
			}

			if (GetIndex() == -999 && (Input::IsRepeated(Input::DOWN) || Input::IsRepeated(Input::RIGHT) || Input::IsTriggered(Input::SCROLL_DOWN) ||
				Input::IsRepeated(Input::UP) || Input::IsRepeated(Input::LEFT) || Input::IsTriggered(Input::SCROLL_UP))) {
				SetIndex(GetMouseOldIndex());
			}
		}
	}
}
