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

#ifndef EP_PLATFORM_SWITCH_CLOCK_H
#define EP_PLATFORM_SWITCH_CLOCK_H

#include <switch.h>
#include <cstdint>
#include <chrono>

struct NxClock {
	static constexpr int64_t ticks_per_sec = 19200000;

	using rep = int64_t;
	using period = std::ratio<1,ticks_per_sec>;
	using duration = std::chrono::duration<rep,period>;
	using time_point = std::chrono::time_point<NxClock,duration>;

	static constexpr bool is_steady = true;

	static time_point now();

	template <typename R, typename P>
	static void SleepFor(std::chrono::duration<R,P> dt);

	static constexpr const char* Name();
};

inline NxClock::time_point NxClock::now() {
	auto ticks = armGetSystemTick();
	return time_point(duration(ticks));
}

template <typename R, typename P>
inline void NxClock::SleepFor(std::chrono::duration<R,P> dt) {
	auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(dt);
	svcSleepThread(ns.count());
}

constexpr const char* NxClock::Name() {
	return "NxClock";
}

#endif
