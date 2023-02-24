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

#if defined(OPENDINGUX)

// Headers
#include "input_buttons.h"
#include "keys.h"

Input::ButtonMappingArray Input::GetDefaultButtonMappings() {
	return {
		{UP, Keys::UP},
		{DOWN, Keys::DOWN},
		{LEFT, Keys::LEFT},
		{RIGHT, Keys::RIGHT},
		{DECISION, Keys::RETURN}, //START
		{DECISION, Keys::LCTRL}, //A
		{CANCEL, Keys::ESCAPE}, //SELECT
		{CANCEL, Keys::LALT}, //B
		{N1, Keys::LSHIFT},//Y
		{N2, Keys::SPACE},//X

		{DEBUG_MENU, Keys::TAB},//L
		{DEBUG_THROUGH, Keys::BACKSPACE},//R
	};
}

Input::KeyNamesArray Input::GetInputKeyNames() {
	return {};
}

void Input::GetSupportedConfig(Game_ConfigInput& cfg) {}

#endif
