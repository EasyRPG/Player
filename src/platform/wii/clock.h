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

#ifndef EP_PLATFORM_WII_CLOCK_H
#define EP_PLATFORM_WII_CLOCK_H

#include <ogc/lwp_watchdog.h>
#include <cstdint>
#include <chrono>
#include <thread>

struct WiiClock {
	static constexpr int64_t ticks_per_sec = TB_TIMER_CLOCK * 1000;

	using rep = uint64_t;
	using period = std::ratio<1,ticks_per_sec>;
	using duration = std::chrono::duration<rep,period>;
	using time_point = std::chrono::time_point<WiiClock,duration>;

	static constexpr bool is_steady = true;

	static time_point now();

	template <typename R, typename P>
	static void SleepFor(std::chrono::duration<R,P> dt);

	static constexpr const char* Name();
};

inline WiiClock::time_point WiiClock::now() {
	auto ticks = gettime();
	return time_point(duration(ticks));
}

template <typename R, typename P>
inline void WiiClock::SleepFor(std::chrono::duration<R,P> dt) {
	std::this_thread::sleep_for(dt);
}

constexpr const char* WiiClock::Name() {
	return "WiiClock";
}

#endif
