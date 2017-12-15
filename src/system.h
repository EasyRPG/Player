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

#if !(defined(USE_SDL) || defined(_3DS) || defined(PSP2))
#  error "This build doesn't target a backend"
#endif

/*
 * TODO?: Combine system.h and options.h ?
 * options.h is oriented to configurations that the EasyRPG user might want
 * to change.
 * system.h is oriented to what used libraries and OS are capable of.
 */
#include "options.h"

/*
 * Smart pointer header.
 */
#include "memory_management.h"

#ifdef _3DS
#  define SUPPORT_JOYSTICK_AXIS
#endif

#ifdef PSP2
#  define SUPPORT_JOYSTICK
#  define SUPPORT_JOYSTICK_AXIS
#endif

#ifdef GEKKO
#  include "stdint.h"

#  define WORDS_BIGENDIAN
#endif

#ifdef OPENDINGUX
#  include <sys/types.h>
#endif

#define SUPPORT_BMP
#define SUPPORT_PNG
#define SUPPORT_XYZ
#define SUPPORT_TTF
#define SUPPORT_FON

#define SUPPORT_ZOOM

#ifdef USE_SDL
#  if defined(GEKKO) || defined(OPENDINGUX) || defined(EMSCRIPTEN)
#    undef SUPPORT_ZOOM
#  endif

#  if !defined(OPENDINGUX) && !defined(GEKKO)
#    define SUPPORT_KEYBOARD
#    define SUPPORT_MOUSE

     // We have our own touch input solution on Android
#    if !defined(__ANDROID__)
#      define SUPPORT_TOUCH
#    endif
#  endif

#  if !defined(OPENDINGUX)
#    define SUPPORT_JOYSTICK
#    define SUPPORT_JOYSTICK_HAT
#    define SUPPORT_JOYSTICK_AXIS
#    define JOYSTICK_AXIS_SENSIBILITY 20000
#  else
#    define SUPPORT_KEYBOARD
#  endif

#  ifdef HAVE_SDL_MIXER
#    define SUPPORT_WAV
#    define SUPPORT_MID
#    define SUPPORT_OGG
#    define SUPPORT_MP3
#  endif

#  ifdef WANT_FMMIDI
#    if WANT_FMMIDI != 1 && WANT_FMMIDI != 2
#      error "WANT_FMMIDI must be set to 1 (use instead of sdl) or 2 (fallback on sdl error)"
#    endif

#    if !defined(HAVE_SDL_MIXER) && WANT_FMMIDI == 2
#      error "WANT_FMMIDI must be set to 1 for non-SDL Mixer builds"
#    endif 
#  endif
#else
// ifndef USE_SDL
#  ifdef WANT_FMMIDI
#    if WANT_FMMIDI != 1
#      error "WANT_FMMIDI must be set to 1 for non-SDL builds"
#    endif
#  endif
#endif

#if defined(HAVE_LIBSAMPLERATE) || defined(HAVE_LIBSPEEXDSP)
#  define USE_AUDIO_RESAMPLER
#endif

#endif
