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
#include "input_buttons.h"
#include "SDL.h"

////////////////////////////////////////////////////////////
/// Global Variables
////////////////////////////////////////////////////////////
namespace Input {
	std::vector< std::vector<int> > buttons;
	std::vector< std::vector<int> > dirkeys;
}

////////////////////////////////////////////////////////////
/// Initialize
////////////////////////////////////////////////////////////
void Input::InitButtons() {
	buttons.resize(Count);
	buttons[UP].push_back(SDLK_UP);
	buttons[UP].push_back(SDLK_k);
	buttons[UP].push_back(SDLK_KP8);
	buttons[DOWN].push_back(SDLK_DOWN);
	buttons[DOWN].push_back(SDLK_j);
	buttons[DOWN].push_back(SDLK_KP2);
	buttons[LEFT].push_back(SDLK_LEFT);
	buttons[LEFT].push_back(SDLK_h);
	buttons[LEFT].push_back(SDLK_KP4);
	buttons[RIGHT].push_back(SDLK_RIGHT);
	buttons[RIGHT].push_back(SDLK_l);
	buttons[RIGHT].push_back(SDLK_KP6);
	buttons[DECISION].push_back(SDLK_z);
	buttons[DECISION].push_back(SDLK_SPACE);
	buttons[DECISION].push_back(SDLK_RETURN);
	buttons[DECISION].push_back(SDLK_KP_ENTER);
	buttons[CANCEL].push_back(SDLK_x);
	buttons[CANCEL].push_back(SDLK_c);
	buttons[CANCEL].push_back(SDLK_v);
	buttons[CANCEL].push_back(SDLK_b);
	buttons[CANCEL].push_back(SDLK_n);
	buttons[CANCEL].push_back(SDLK_ESCAPE);
	buttons[SHIFT].push_back(SDLK_LSHIFT);
	buttons[SHIFT].push_back(SDLK_RSHIFT);
	buttons[N0].push_back(SDLK_0);
	buttons[N1].push_back(SDLK_1);
	buttons[N2].push_back(SDLK_2);
	buttons[N3].push_back(SDLK_3);
	buttons[N4].push_back(SDLK_4);
	buttons[N5].push_back(SDLK_5);
	buttons[N6].push_back(SDLK_6);
	buttons[N7].push_back(SDLK_7);
	buttons[N8].push_back(SDLK_8);
	buttons[N9].push_back(SDLK_9);
	buttons[PLUS].push_back(SDLK_KP_PLUS);
	buttons[MINUS].push_back(SDLK_KP_MINUS);
	buttons[MULTIPLY].push_back(SDLK_KP_MULTIPLY);
	buttons[DIVIDE].push_back(SDLK_KP_DIVIDE);
	buttons[PERIOD].push_back(SDLK_KP_PERIOD);

	dirkeys.resize(10);
	dirkeys[2].push_back(DOWN);
	dirkeys[4].push_back(LEFT);
	dirkeys[6].push_back(RIGHT);
	dirkeys[8].push_back(UP);
}
