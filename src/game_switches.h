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

#ifndef __game_switches__
#define __game_switches__

#include <vector>

class Game_Switches {

public:
	Game_Switches();
	~Game_Switches();

	bool operator[] (unsigned int switch_id){
		return data[switch_id];
	}

	void SetAt(unsigned int switch_id, bool value) {
		data[switch_id] = value;
	}

	void ToggleAt(unsigned int switch_id) {
		data[switch_id] = !data[switch_id];
	}
private:
	std::vector<bool> data;

};
#endif // __game_switches__
