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

#ifndef EP_GAME_QUIT_H
#define EP_GAME_QUIT_H

#include "window_help.h"

class Game_Quit {
public:
	Game_Quit();
	void Update();
	void OnResolutionChange();

	bool ShouldQuit() const;
private:
	void Reset();

	Window_Help window;
	int time_left = 0;
};

inline bool Game_Quit::ShouldQuit() const {
	return time_left == 0;
}

#endif
