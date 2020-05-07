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

#if defined(PSP)

// Headers
#include "input_buttons.h"
#include "keys.h"

Input::ButtonMappingArray Input::GetDefaultButtonMappings() {
	return {
		{DECISION, Keys::JOY_1}, // Circle
		{DECISION, Keys::JOY_2}, // Cross
		{CANCEL, Keys::JOY_3}, // Square

		{TOGGLE_FPS, Keys::JOY_0}, // Triangle

		{N1, Keys::JOY_4}, // Left trigger
		{N2, Keys::JOY_5}, // Right trigger

		{DOWN, Keys::JOY_6}, // Down
		{LEFT, Keys::JOY_7}, // Left
		{UP, Keys::JOY_8}, // Up
		{RIGHT, Keys::JOY_9}, // Right
	};
}

Input::DirectionMappingArray Input::GetDefaultDirectionMappings() {
	return {
		{ Direction::DOWN, DOWN },
		{ Direction::LEFT, LEFT },
		{ Direction::RIGHT, RIGHT },
		{ Direction::UP, UP },
	};
}

#endif
