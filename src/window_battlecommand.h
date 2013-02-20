/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

#ifndef _WINDOW_BATTLECOMMAND_H_
#define _WINDOW_BATTLECOMMAND_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <vector>
#include "window_base.h"
#include "window_help.h"
#include "rpg_battlecommand.h"
#include "font.h"

////////////////////////////////////////////////////////////
/// Window_BattleCommand class.
////////////////////////////////////////////////////////////
class Window_BattleCommand: public Window_Base {
public:
	////////////////////////////////////////////////////////
	/// Constructor.
	/// @param commands : commands to display
	////////////////////////////////////////////////////////
	Window_BattleCommand(int x, int y, int width, int height);

	////////////////////////////////////////////////////////
	/// Refresh the window contents.
	////////////////////////////////////////////////////////
	void Refresh();

	////////////////////////////////////////////////////////
	/// Update the window state.
	////////////////////////////////////////////////////////
	void Update();

	////////////////////////////////////////////////////////
	/// Enable or disable a command.
	/// @param index : command index
	/// @param enabled : whether the command is enabled
	////////////////////////////////////////////////////////
	void SetEnabled(int index, bool enabled);

	////////////////////////////////////////////////////////
	/// Enable or disable a command.
	/// @param index : command index
	/// @param enabled : whether the command is enabled
	////////////////////////////////////////////////////////
	void SetActor(int actor_id);

	int GetIndex();
	void SetIndex(int index);
	void SetActive(bool active);
	void UpdateCursorRect();
	RPG::BattleCommand GetCommand();
	int GetSkillSubset();

protected:
	int actor_id;
	std::vector<std::string> commands;
	int index;
	int num_rows;
	int top_row;
	std::vector<bool> disabled;
	int cycle;

	void DrawItem(int index, Font::SystemColor color);
};

#endif
