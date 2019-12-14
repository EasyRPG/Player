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

	int Set(int variable_id, int value);
	int Add(int variable_id, int value);
	int Sub(int variable_id, int value);
	int Mult(int variable_id, int value);
	int Div(int variable_id, int value);
	int Mod(int variable_id, int value);

	void SetRange(int first_id, int last_id, int value);
	void AddRange(int first_id, int last_id, int value);
	void SubRange(int first_id, int last_id, int value);
	void MultRange(int first_id, int last_id, int value);
	void DivRange(int first_id, int last_id, int value);
	void ModRange(int first_id, int last_id, int value);

	std::string GetName(int _id) const;

	bool IsValid(int variable_id) const;

	int GetSize() const;

	void Reset();
private:
	mutable int _warnings = 0;
};

// Global variable
extern Game_Variables_Class Game_Variables;

inline int Game_Variables_Class::Add(int variable_id, int value) {
	return Set(variable_id, Get(variable_id) + value);
}

inline int Game_Variables_Class::Sub(int variable_id, int value) {
	return Set(variable_id, Get(variable_id) - value);
}

inline int Game_Variables_Class::Mult(int variable_id, int value) {
	return Set(variable_id, Get(variable_id) * value);
}

inline int Game_Variables_Class::Div(int variable_id, int value) {
	if (value != 0) {
		return Set(variable_id, Get(variable_id) / value);
	}
	return Get(variable_id);
}

inline int Game_Variables_Class::Mod(int variable_id, int value) {
	if (value != 0) {
		return Set(variable_id, Get(variable_id) % value);
	}
	return Set(variable_id, 0);
}

inline void Game_Variables_Class::SetRange(int first_id, int last_id, int value) {
	for (int i = first_id; i <= last_id; ++i) {
		Set(i, value);
	}
}

inline void Game_Variables_Class::AddRange(int first_id, int last_id, int value) {
	for (int i = first_id; i <= last_id; ++i) {
		Add(i, value);
	}
}

inline void Game_Variables_Class::SubRange(int first_id, int last_id, int value) {
	for (int i = first_id; i <= last_id; ++i) {
		Sub(i, value);
	}
}

inline void Game_Variables_Class::MultRange(int first_id, int last_id, int value) {
	for (int i = first_id; i <= last_id; ++i) {
		Mult(i, value);
	}
}

inline void Game_Variables_Class::DivRange(int first_id, int last_id, int value) {
	for (int i = first_id; i <= last_id; ++i) {
		Div(i, value);
	}
}

inline void Game_Variables_Class::ModRange(int first_id, int last_id, int value) {
	for (int i = first_id; i <= last_id; ++i) {
		Mod(i, value);
	}
}

#endif
