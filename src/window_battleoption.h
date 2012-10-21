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

#ifndef _WINDOW_BATTLEOPTION_H_
#define _WINDOW_BATTLEOPTION_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <vector>
#include "window_base.h"
#include "font.h"

////////////////////////////////////////////////////////////
/// Window_BattleOption class.
////////////////////////////////////////////////////////////
class Window_BattleOption: public Window_Base {
public:
	////////////////////////////////////////////////////////
	/// Constructor.
	////////////////////////////////////////////////////////
	Window_BattleOption(int x, int y, int width, int height);

	////////////////////////////////////////////////////////
	/// Refresh the window contents.
	////////////////////////////////////////////////////////
	void Refresh();

	////////////////////////////////////////////////////////
	/// Update the window state.
	////////////////////////////////////////////////////////
	void Update();

	int GetIndex();
	void SetIndex(int index);
	void SetActive(bool active);
	void UpdateCursorRect();

protected:
	std::vector<std::string> commands;
	int index;
	int num_rows;
	int top_row;

	void DrawItem(int index, Font::SystemColor color);
};

#endif
