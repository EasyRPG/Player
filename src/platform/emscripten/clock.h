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

#ifndef EP_EMSCRIPTEN_CLOCK_H
#define EP_EMSCRIPTEN_CLOCK_H

#include <chrono>

struct EmscriptenClock {
	using clock = std::chrono::steady_clock;

	using rep = clock::rep;
	using period = clock::period;
	using duration = clock::duration;
	using time_point = clock::time_point;

	static constexpr bool is_steady = clock::is_steady;

	/** Get current time */
	static time_point now();

	/** Sleep for the specified duration */
	template <typename R, typename P>
	static void SleepFor(std::chrono::duration<R,P> dt);

	static constexpr const char* Name();
};

inline EmscriptenClock::time_point EmscriptenClock::now() {
	return clock::now();
}

template <typename R, typename P>
inline void EmscriptenClock::SleepFor(std::chrono::duration<R,P> dt) {
	// Browser handles sleep
}

constexpr const char* EmscriptenClock::Name() {
	return "Emscripten";
}

#endif
