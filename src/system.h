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

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

/*
 * Includes GNU Autotools build configuration parameters.
 * This option may have defined USE_SDL and others.
 */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#if !defined(USE_SDL)
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

#ifdef GEKKO
#  include "stdint.h"
#endif

#define SUPPORT_BMP
#define SUPPORT_PNG
#define SUPPORT_XYZ

/*
#define SUPPORT_JPG
#define SUPPORT_GIF
#define SUPPORT_TIFF
*/

#define SUPPORT_TTF
#define SUPPORT_FON

#define SUPPORT_ZOOM

#ifdef USE_SDL
#  define USE_SDL_MIXER


#  ifdef PSP
#    undef SUPPORT_AUDIO
#    undef USE_SDL_MIXER
#  endif

#  if defined(GEKKO) || defined(OPENDINGUX)
#    undef SUPPORT_ZOOM
#  endif

#  if !defined(OPENDINGUX) && !defined(GEKKO)
#    define SUPPORT_KEYBOARD
#    define SUPPORT_MOUSE
#  endif

#  if !defined(OPENDINGUX)
#    define SUPPORT_JOYSTICK
#    define SUPPORT_JOYSTICK_HAT
#    define SUPPORT_JOYSTICK_AXIS
#    define JOYSTICK_AXIS_SENSIBILITY 20000
#  else
#    define SUPPORT_KEYBOARD
#  endif

#  ifdef USE_SDL_MIXER
#    define SUPPORT_WAV
#    define SUPPORT_MID
#    define SUPPORT_OGG
#    define SUPPORT_MP3
#  endif

#  ifdef NO_SDL_MIXER
#    undef SUPPORT_AUDIO
#  else
#    define SUPPORT_AUDIO
#  endif
#endif

#endif
