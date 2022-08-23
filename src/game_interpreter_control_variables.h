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


#ifndef EP_GAME_INTERPRETER_CONTROL_VARIABLES
#define EP_GAME_INTERPRETER_CONTROL_VARIABLES

#include <lcf/rpg/eventcommand.h>

class Game_Interpreter;

namespace ControlVariables {
	int Random(int value, int value2);
	int Item(int op, int item);
	int Actor(int op, int actor_id);
	int Party(int op, int party_idx);
	int Event(int op, int event_id, const Game_Interpreter& interpreter);
	int Other(int op);
	int Enemy(int op, int enemy_idx);
	int Pow(int arg1, int arg2);
	int Sqrt(int arg, int mul);
	int Sin(int arg1, int arg2, int mul);
	int Cos(int arg1, int arg2, int mul);
	int Atan2(int arg1, int arg2, int mul);
	int Min(int arg1, int arg2);
	int Max(int arg1, int arg2);
	int Abs(int arg);
	int Binary(int op, int arg1, int arg2);
	// Functions only used by expressions
	int Clamp(int arg1, int arg2, int arg3);
	int Muldiv(int arg1, int arg2, int arg3);
	int Divmul(int arg1, int arg2, int arg3);
	int Between(int arg1, int arg2, int arg3);
}

#endif
