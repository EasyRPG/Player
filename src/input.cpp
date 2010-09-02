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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "input.h"
#include "input_buttons.h"
#include "SDL.h"
#include "output.h"

////////////////////////////////////////////////////////////
/// Global Variables
////////////////////////////////////////////////////////////
namespace Input {
	std::vector<int> press_time;
	std::vector<bool> triggered;
	std::vector<bool> repeated;
	std::vector<bool> released;
	int dir4;
	int dir8;
	int start_repeat_time;
	int repeat_time;
}

////////////////////////////////////////////////////////////
/// Initialize
////////////////////////////////////////////////////////////
void Input::Init() {
	if (!(SDL_WasInit(SDL_INIT_JOYSTICK) & SDL_INIT_JOYSTICK)) {
		if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0) {
			Output::Warning("Couldn't initialize joystick.\n%s\n", SDL_GetError());
		}
	}

	InitButtons();

	press_time.resize(buttons.size(), 0);
	triggered.resize(buttons.size(), false);
	repeated.resize(buttons.size(), false);
	released.resize(buttons.size(), false);

	start_repeat_time = 20;
	repeat_time = 10;
}

////////////////////////////////////////////////////////////
/// Update keys state
////////////////////////////////////////////////////////////
void Input::Update() {
	Uint8* keystates = SDL_GetKeyState(NULL); 
	unsigned int i, e;
	bool pressed;
	for (i = 0; i < buttons.size(); ++i) {
		pressed = false;
		for (e = 0; e < buttons[i].size(); e++) {
			if (keystates[(SDLKey)buttons[i][e]]) {
				pressed = true;
				break;
			}
		}
		if (pressed) {
			press_time[i] += 1;
			released[i] = false;
		} else {
			released[i] = press_time[i] > 0;
			press_time[i] = 0;
		}
		if (press_time[i] > 0) {
			triggered[i] = press_time[i] == 1;
			repeated[i] = press_time[i] == 1 || 
				(press_time[i] >= start_repeat_time &&
				press_time[i] % repeat_time == 0);
		} else {
			triggered[i] = false;
			repeated[i] = false;
		}
	}
	
	unsigned int dirpress[10];
	for (i = 1; i < 10; i++) {
		dirpress[i] = 0;
		if (i != 5) {
			unsigned int max_presstime;
			for (e = 0; e < dirkeys[i].size(); e++) {
				max_presstime = press_time[dirkeys[i][e]];
				if (max_presstime > dirpress[i]) {
					dirpress[i] = max_presstime;
				}
			}
		}
	}

	dirpress[1] += (dirpress[2] > 0 && dirpress[4] > 0) ? dirpress[2] + dirpress[4] : 0;
	dirpress[3] += (dirpress[2] > 0 && dirpress[6] > 0) ? dirpress[2] + dirpress[6] : 0;
	dirpress[7] += (dirpress[8] > 0 && dirpress[4] > 0) ? dirpress[8] + dirpress[4] : 0;
	dirpress[9] += (dirpress[8] > 0 && dirpress[6] > 0) ? dirpress[8] + dirpress[6] : 0;

	dir4 = 0;
	dir8 = 0;
		
	if (!(dirpress[2] > 0 && dirpress[8] > 0) && !(dirpress[4] > 0 && dirpress[6] > 0)) {
		e = 0;
		for (i = 0; i < 4; i++) {
			if (dirpress[(i + 1) * 2] > 0) {
				if (e == 0 || dirpress[(i + 1) * 2] < e) {
					dir4 = (i + 1) * 2;
					e = dirpress[(i + 1) * 2];
				}
			}
		}
		dir8 = dir4;
		if (dirpress[9] > 0) {
			dir8 = 9;
		} else if (dirpress[7] > 0) {
			dir8 = 7;
		} else if (dirpress[3] > 0) {
			dir8 = 3;
		} else if (dirpress[1] > 0) {
			dir8 = 1;
		}
	}
}

////////////////////////////////////////////////////////////
/// Clear keys state
////////////////////////////////////////////////////////////
void Input::ClearKeys() {
	for (unsigned int i = 0; i < buttons.size(); i++) {
		press_time[i] = 0;
		triggered[i] = false;
		repeated[i] = false;
		released[i] = false;
	}
	dir4 = 0;
	dir8 = 0;
}

////////////////////////////////////////////////////////////
/// Is pressed?
////////////////////////////////////////////////////////////
bool Input::IsPressed(InputButton button) {
	return press_time[button] > 0;
}

////////////////////////////////////////////////////////////
/// Is triggered?
////////////////////////////////////////////////////////////
bool Input::IsTriggered(InputButton button) {
	return triggered[button];
}

////////////////////////////////////////////////////////////
/// Is repeated?
////////////////////////////////////////////////////////////
bool Input::IsRepeated(InputButton button) {
	return repeated[button];
}

////////////////////////////////////////////////////////////
/// Is released?
////////////////////////////////////////////////////////////
bool Input::IsReleased(InputButton button) {
	return released[button];
}
