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

#ifndef EP_PLATFORM_PSVITA_CLOCK_H
#define EP_PLATFORM_PSVITA_CLOCK_H

#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <cstdint>
#include <chrono>

struct Psp2Clock {
	using rep = int64_t;
	using period = std::micro;
	using duration = std::chrono::duration<rep,period>;
	using time_point = std::chrono::time_point<Psp2Clock,duration>;

	static constexpr bool is_steady = true;

	static time_point now();

	template <typename R, typename P>
	static void SleepFor(std::chrono::duration<R,P> dt);

	static constexpr const char* Name();
};

inline Psp2Clock::time_point Psp2Clock::now() {
	auto ticks = sceKernelGetProcessTimeWide();
	return time_point(duration(ticks));
}

template <typename R, typename P>
inline void Psp2Clock::SleepFor(std::chrono::duration<R,P> dt) {
	auto us = std::chrono::duration_cast<std::chrono::microseconds>(dt);
	sceKernelDelayThread(us.count());
}

constexpr const char* Psp2Clock::Name() {
	return "Psp2Clock";
}

#endif
