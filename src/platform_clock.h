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

#ifndef EP_PLATFORM_CLOCK_H
#define EP_PLATFORM_CLOCK_H

#include <chrono>
#include <type_traits>
#include <thread>
#include "3ds_clock.h"
#include "switch_clock.h"
#include "psp2_clock.h"
#include "std_clock.h"

#if defined(_3DS)
using Platform_Clock = CtrClock;
#elif defined(__SWITCH__)
using Platform_Clock = NxClock;
#elif defined(PSP2)
using Platform_Clock = Psp2Clock;
#else
using Platform_Clock = StdClock;
#endif

#endif
