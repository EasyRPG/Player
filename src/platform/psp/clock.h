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

#ifndef EP_PLATFORM_PSP_CLOCK_H
#define EP_PLATFORM_PSP_CLOCK_H

#include <pspkernel.h>
#include <cstdint>
#include <chrono>

struct PspClock {
	using rep = int64_t;
	using period = std::micro;
	using duration = std::chrono::duration<rep,period>;
	using time_point = std::chrono::time_point<PspClock,duration>;

	static constexpr bool is_steady = true;

	static time_point now();

	template <typename R, typename P>
	static void SleepFor(std::chrono::duration<R,P> dt);

	static constexpr const char* Name();
};

inline PspClock::time_point PspClock::now() {
	// Microseconds since the PSP was booted
	auto ticks = sceKernelGetSystemTimeWide();
	return time_point(duration(ticks));
}

template <typename R, typename P>
inline void PspClock::SleepFor(std::chrono::duration<R,P> dt) {
	auto us = std::chrono::duration_cast<std::chrono::microseconds>(dt);
	sceKernelDelayThread(static_cast<unsigned int>(us.count()));
}

constexpr const char* PspClock::Name() {
	return "PspClock";
}

#endif
