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

#ifndef EP_GAME_VARIABLES_H
#define EP_GAME_VARIABLES_H

// Headers
#include "data.h"
#include <string>

/**
 * Game_Variables class.
 */
class Game_Variables_Class {
public:
	Game_Variables_Class();

	int Get(int variable_id) const;

	void Set(int variable_id, int value);

	std::string GetName(int _id) const;

	bool IsValid(int variable_id) const;

	int GetSize() const;

	void Reset();
private:
	mutable int _warnings = 0;
};

// Global variable
extern Game_Variables_Class Game_Variables;

#endif
