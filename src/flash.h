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

#ifndef EP_FLASH_H
#define EP_FLASH_H

#include <cstdint>
#include "color.h"

/** Contains helper functions for flash effect */
namespace Flash {
/**
 * Perform one time step of flash data animation
 *
 * @param current_level flash level
 * @param time_left amount of flash time left
 * @param continuous true if continuous flash
 * @param period original flash period
 * @param sat original flash saturation
 */
inline void Update(double& current_level, int32_t& time_left, bool continuous, int32_t period, int32_t sat)
{
	if (current_level > 0 || continuous) {
		if (time_left > 0) {
			current_level = current_level - (current_level / time_left);
			--time_left;
		}
		if (time_left <= 0) {
			time_left = 0;
			current_level = 0;
			if (continuous) {
				time_left = period;
				current_level = sat;
			}
		}
	}
}

/**
 * Perform one time step of flash data animation
 *
 * @param current_level flash level
 * @param time_left amount of flash time left
 */
inline void Update(double& current_level, int32_t& time_left) {
	Update(current_level, time_left, false, 0, 0);
}

/**
 * Create a Color object from RPG_RT flash values [0,31]
 *
 * @param r red color
 * @param g green color
 * @param b blue color
 * @param current_level strength of flash
 */
constexpr Color MakeColor(int r, int g, int b, double current_level) {
	return current_level > 0.0 ? Color(r * 8, g * 8, b * 8, current_level * 8) : Color();
}

} //namespace Flash

#endif
