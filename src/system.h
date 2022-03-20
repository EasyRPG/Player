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

#ifndef EP_SYSTEM_H
#define EP_SYSTEM_H

/*
 * Includes GNU Autotools build configuration parameters.
 * This option may have defined USE_SDL and others.
 */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#if !(defined(USE_SDL) || defined(__3DS__) || defined(__vita__) || defined(__SWITCH__) || defined(USE_LIBRETRO))
#  error "This build doesn't target a backend"
#endif

/*
 * TODO?: Combine system.h and options.h ?
 * options.h is oriented to configurations that the EasyRPG user might want
 * to change.
 * system.h is oriented to what used libraries and OS are capable of.
 */
#include "options.h"

// Smart pointer header.
#include "memory_management.h"

#if defined(USE_LIBRETRO)
// libretro must be first to prevent conflicts with other defines
#  define SUPPORT_KEYBOARD
#  define SUPPORT_JOYSTICK
#  define SUPPORT_JOYSTICK_HAT
#  define SUPPORT_JOYSTICK_AXIS
#  define JOYSTICK_AXIS_SENSIBILITY 20000
#elif defined(OPENDINGUX)
#  include <sys/types.h>
#elif defined(__ANDROID__)
#  define SUPPORT_ZOOM
#  define SUPPORT_JOYSTICK
#  define SUPPORT_JOYSTICK_HAT
#  define SUPPORT_JOYSTICK_AXIS
#elif defined(EMSCRIPTEN)
#  define SUPPORT_MOUSE
#  define SUPPORT_TOUCH
#  define SUPPORT_JOYSTICK
#  define SUPPORT_JOYSTICK_HAT
#  define SUPPORT_JOYSTICK_AXIS
#elif defined(__3DS__)
#  define SUPPORT_JOYSTICK_AXIS
#elif defined(__vita__)
#  define SUPPORT_JOYSTICK
#  define SUPPORT_JOYSTICK_AXIS
#elif defined(GEKKO)
#  include <cstdint>
#  define WORDS_BIGENDIAN
#  define SUPPORT_JOYSTICK
#  define SUPPORT_JOYSTICK_HAT
#  define SUPPORT_JOYSTICK_AXIS
#elif defined(_WIN32)
#  define SUPPORT_ZOOM
#  define SUPPORT_MOUSE
#  define SUPPORT_TOUCH
#  define SUPPORT_JOYSTICK
#  define SUPPORT_JOYSTICK_HAT
#  define SUPPORT_JOYSTICK_AXIS
#elif defined(__SWITCH__)
#elif defined(__MORPHOS__) || defined(__amigaos4__)
#  define SUPPORT_ZOOM
#  define SUPPORT_MOUSE
#  define WORDS_BIGENDIAN
#  define SUPPORT_JOYSTICK
#  define SUPPORT_JOYSTICK_HAT
#  define SUPPORT_JOYSTICK_AXIS
#else // Everything not catched above, e.g. Linux/*BSD/macOS
#  define USE_WINE_REGISTRY
#  define USE_XDG_RTP
#  define SUPPORT_ZOOM
#  define SUPPORT_MOUSE
#  define SUPPORT_TOUCH
#  define SUPPORT_JOYSTICK
#  define SUPPORT_JOYSTICK_HAT
#  define SUPPORT_JOYSTICK_AXIS
#endif

#ifdef USE_SDL

#  define SUPPORT_KEYBOARD
#  define JOYSTICK_AXIS_SENSIBILITY 20000

#endif

#if defined(HAVE_LIBSAMPLERATE) || defined(HAVE_LIBSPEEXDSP)
#  define USE_AUDIO_RESAMPLER
#endif

#endif

#if defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>
#endif
