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

#ifndef _INPUT_H_
#define _INPUT_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <vector>
#include "input_buttons.h"
#include "SDL.h"

////////////////////////////////////////////////////////////
/// Input namespace
////////////////////////////////////////////////////////////
namespace Input {
	void Init();
	void Update();
	void ClearKeys();
	bool IsPressed(InputButton button);
	bool IsTriggered(InputButton button);
	bool IsRepeated(InputButton button);
	bool IsReleased(InputButton button);

	extern std::vector<int> press_time;
	extern std::vector<bool> triggered;
	extern std::vector<bool> repeated;
	extern std::vector<bool> released;
	extern int dir4;
	extern int dir8;
	extern int start_repeat_time;
	extern int repeat_time;

#ifdef GEKKO
	extern SDL_Joystick* joystick;
#endif
}

#endif
