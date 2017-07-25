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

#ifndef EP_GAME_SWITCHES_H
#define EP_GAME_SWITCHES_H

// Headers
#include <vector>
#include <string>

/**
 * Game_Switches class
 */
class Game_Switches_Class {
public:
	Game_Switches_Class();

	std::vector<bool>::reference operator[](int switch_id);
	
	std::string GetName(int _id) const;

	bool IsValid(int switch_id) const;

	int GetSize() const;

	void Reset();

private:
	std::vector<bool> dummy;
};

// Global variable
extern Game_Switches_Class Game_Switches;

#endif
