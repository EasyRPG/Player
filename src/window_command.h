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

#ifndef _WINDOW_COMMAND_H_
#define _WINDOW_COMMAND_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <vector>
#include "window_selectable.h"

////////////////////////////////////////////////////////////
/// Window Command Class
////////////////////////////////////////////////////////////
class Window_Command: public Window_Selectable {
public:
	Window_Command(int width, std::vector<std::string> icommands);
	~Window_Command();

	void Refresh();
	void DrawItem(int i, int color);
	void DisableItem(int i);

protected:
	std::vector<std::string> commands;
};



#endif
