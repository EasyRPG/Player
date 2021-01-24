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

#ifndef EP_PLATFORM_LIBRETRO_CLOCK_H
#define EP_PLATFORM_LIBRETRO_CLOCK_H

#ifdef USE_LIBRETRO

#include <cstdint>
#include <chrono>

#include "libretro.h"

struct LibretroClock {
	using rep = int64_t;
	using period = std::micro;
	using duration = std::chrono::duration<rep,period>;
	using time_point = std::chrono::time_point<LibretroClock,duration>;

	static constexpr bool is_steady = true;

	static time_point now();

	template <typename R, typename P>
	static void SleepFor(std::chrono::duration<R,P> dt);

	static constexpr const char* Name();

	static retro_usec_t time_in_microseconds;
};

inline LibretroClock::time_point LibretroClock::now() {
	auto ticks = time_in_microseconds;
	return time_point(duration(ticks));
}

template <typename R, typename P>
inline void LibretroClock::SleepFor(std::chrono::duration<R,P>) {
	// no-op external timing source
}

constexpr const char* LibretroClock::Name() {
	return "LibretroClock";
}

#endif
#endif
