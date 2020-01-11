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

#include <thread>

#if defined(PSP2)
#include <psp2/kernel/threadmgr.h>
#elif defined(_3DS)
#include <3ds/svc.h>
#endif

constexpr bool Game_Clock::is_steady;

void Game_Clock::SleepFor(std::chrono::nanoseconds ns) {
#if defined(_3DS)
	svcSleepThread(ns.count());
#elif defined(PSP2)
	auto us = std::chrono::duration_cast<std::chrono::microseconds>(ns);
	sceKernelDelayThread(us.count());
#else
	std::this_thread::sleep_for(ns);
#endif
}
