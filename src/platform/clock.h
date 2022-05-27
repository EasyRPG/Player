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

#if defined(__3DS__)
#include "platform/3ds/clock.h"
using Platform_Clock = CtrClock;
#elif defined(GEKKO)
#include "platform/wii/clock.h"
using Platform_Clock = WiiClock;
#elif defined(__SWITCH__)
#include "platform/switch/clock.h"
using Platform_Clock = NxClock;
#elif defined(__vita__)
#include "platform/psvita/clock.h"
using Platform_Clock = Psp2Clock;
#elif defined(USE_LIBRETRO)
// Only use libretro clock on platforms with no custom clock
#include "platform/libretro/clock.h"
using Platform_Clock = LibretroClock;
#else
#include "std_clock.h"
using Platform_Clock = StdClock;
#endif

#endif
