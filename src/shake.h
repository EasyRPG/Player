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

#ifndef EP_SHAKE_H
#define EP_SHAKE_H

#include <cmath>
#include <cstdint>
#include "utils.h"

/** Contains helper functions for flash effect */
namespace Shake {
static constexpr int kShakeContinuousTimeStart = 65535;

/**
 * Computes new shake position from inputs
 *
 * @param strength strength of shake effect
 * @param speed speed of shake effect
 * @param time_left amount of time left for shake effect
 * @param position current shake position
 */
inline int NextPosition(int strength, int speed, int time_left, int position) {
	int amplitude = 1 + 2 * strength;
	int newpos = amplitude * sin((time_left * 4 * (speed + 2)) % 256 * M_PI / 128) * -1;
	int cutoff = (speed * amplitude / 8) + 1;

	return Utils::Clamp<int>(newpos, position - cutoff, position + cutoff);
}

/**
 * Perform one time step of shake animation
 *
 * @param position shake position
 * @param time_left amount of time left for shake effect
 * @param strength strength of shake effect
 * @param speed speed of shake effect
 * @param continous whether this is a continuous shake
 */
inline void Update(int32_t& position, int32_t& time_left, int32_t strength, int32_t speed, bool continuous)
{
	if (time_left > 0) {
		--time_left;

		// This fixes a bug in RPG_RT where continuous shake would actually stop after
		// 18m12s of gameplay.
		if (time_left <= 0 && continuous) {
			time_left = kShakeContinuousTimeStart;
		}

		if (time_left > 0) {
			position = NextPosition(strength, speed, time_left, position);
		} else {
			position = 0;
			time_left = 0;
		}
	}
}

} //namespace Shake

#endif
