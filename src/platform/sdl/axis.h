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

#ifndef EP_SDL_AXIS_H
#define EP_SDL_AXIS_H

/**
 * Determines the axis to stick/trigger mapping for SDL 1.2.
 *
 * Can be overwritten in two ways:
 * Provide a "SdlAxis Input::GetSdlAxis()" in your input_buttons.cpp.
 *  While you are at it, add a proper mapping for the buttons in it
 * Provide a EP_SDL_AXIS environment variable with comma seperated indices.
 *  Example: EP_SDL_AXIS="1,2,3,4,6,5,1,0" (false = 0, true = 1)
 */
struct SdlAxis {
	int stick_primary_x = -1;
	int stick_primary_y = -1;
	int stick_secondary_x = -1;
	int stick_secondary_y = -1;
	int trigger_left = -1;
	int trigger_right = -1;
	/** When true inverts the y axis of the sticks */
	bool stick_invert_y = false;
	/** When true inverts the y axis of the hat (for D-Pad) */
	bool hat_invert_y = false;
};

#endif
