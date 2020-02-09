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

#include "game_clock.h"
#include "output.h"

#include <thread>
#include <cinttypes>

constexpr bool Game_Clock::is_steady;

void Game_Clock::logClockInfo() {
	const char* clock_name = "Unknown";
#if defined(_3DS)
	clock_name = "CtrClock";
#elif defined(PSP2)
	clock_name = "Psp2Clock";
#else
	if (std::is_same<clock,std::chrono::high_resolution_clock>::value) {
		clock_name = "StdHigRes";
	} else if (std::is_same<clock,std::chrono::steady_clock>::value) {
		clock_name = "StdSteady";
	} else if (std::is_same<clock,std::chrono::system_clock>::value) {
		clock_name = "StdSystem";
	}
#endif
	const char* period_name = "custom";
	if (std::is_same<period,std::nano>::value) {
		period_name = "ns";
	} else if (std::is_same<period,std::micro>::value) {
		period_name = "us";
	} else if (std::is_same<period,std::milli>::value) {
		period_name = "ms";
	}
	Output::Debug("Clock: %s steady=%d period=%s (%" PRIdMAX " / %" PRIdMAX ")",
			clock_name,
			is_steady,
			period_name,
			period::num,
			period::den);
}
