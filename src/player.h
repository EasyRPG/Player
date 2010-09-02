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

#ifndef _PLAYER_H_
#define _PLAYER_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SDL.h"

////////////////////////////////////////////////////////////
/// Player namespace
////////////////////////////////////////////////////////////
namespace Player {
	void Init();
	void Run();
	void Update();
	void Exit();

	void ToggleFullscreen();
	void ToggleZoom();
	int GetWidth();
	int GetHeight();

	extern SDL_Surface* main_window;
	extern bool focus;
	//extern bool alt_pressing;
	extern bool fullscreen;
	extern bool zoom;
}

#endif
