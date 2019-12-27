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

#ifndef EP_GAME_CLOCK_H
#define EP_GAME_CLOCK_H

#include <chrono>
#include <type_traits>

/**
 * Used for time keeping in Player
 */
class Game_Clock {
public:
	using clock = std::conditional_t<
		std::chrono::high_resolution_clock::is_steady,
		std::chrono::high_resolution_clock,
		std::chrono::steady_clock>;

	using rep = clock::rep;
	using period = clock::period;
	using duration = clock::duration;
	using time_point = clock::time_point;

	static constexpr bool is_steady = clock::is_steady;

	/** Get current time */
	static time_point now();

	// FIXME: Remove this and use now() everywhere.
	static uint32_t GetTicks();
	// FIXME: Remove this and use now() everywhere.
	static void InitTicks();
private:
	static time_point init_ticks;
};


inline Game_Clock::time_point Game_Clock::now() {
	return clock::now();
}

inline uint32_t Game_Clock::GetTicks() {
	auto d = now() - init_ticks;
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(d);
	return ms.count();
}

#endif
