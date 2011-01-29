/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#if !(defined(_WIN32) || defined(UNIX) || defined(DINGOO) || defined(GEKKO) || defined(PSP))
	#error "This build doesn't target an os"
#endif

#if !(defined(USE_SDL) || defined(USE_OPENGL))
	#error "This build doesn't target a backend"
#endif

// TODO?: Combine system.h and options.h ?
// options.h is oriented to configurations that the EasyRPG user might want
// to change.
// system.h is oriented to what used libraries and OS are capable of.
#include "options.h"

#ifdef USE_OPENGL
	#define SUPPORT_ZOOM
	#define SUPPORT_FULL_SCALING

	#define USE_SDL_IMAGE

	#ifdef USE_SOIL
		#define SUPPORT_BMP
		#define SUPPORT_GIF
		#define SUPPORT_JPG
		#define SUPPORT_PNG
		//#define SUPPORT_XYZ
	#endif
#endif

#ifdef USE_SOFT_BITMAP
	//#define SUPPORT_BMP
	//#define SUPPORT_GIF
	//#define SUPPORT_JPG
	#define SUPPORT_PNG
	#define SUPPORT_XYZ

	#define SUPPORT_TTF
	#define SUPPORT_FON
#endif

#ifdef USE_PIXMAN_BITMAP
	//#define SUPPORT_BMP
	//#define SUPPORT_GIF
	//#define SUPPORT_JPG
	#define SUPPORT_PNG
	#define SUPPORT_XYZ

	#define SUPPORT_TTF
	#define SUPPORT_FON
#endif

#ifdef USE_SDL
	#if !defined(USE_OPENGL) && !defined(USE_SOFT_BITMAP) && !defined(USE_PIXMAN_BITMAP)
		#define USE_SDL_IMAGE
		#define USE_SDL_BITMAP
		//#define USE_RLE
		#define USE_SDL_TTF
	#endif

	#define USE_SDL_MIXER
	
	#if !defined(DINGOO) && !defined(GEKKO)
		#define SUPPORT_KEYBOARD
		#define SUPPORT_MOUSE
	#endif
	#if !defined(DINGOO)
		#define SUPPORT_JOYSTICK
		#define SUPPORT_JOYSTICK_HAT
		#define SUPPORT_JOYSTICK_AXIS
		#define JOYSTICK_AXIS_SENSIBILITY 20000
	#else
		#define SUPPORT_KEYBOARD
	#endif

	#ifdef USE_SDL_IMAGE
		#define SUPPORT_BMP
		#define SUPPORT_GIF
		//#define SUPPORT_JPG
		#define SUPPORT_PNG
		//#define SUPPORT_XYZ
	#endif

	#ifdef USE_SDL_MIXER
		#define SUPPORT_AUDIO
		#define SUPPORT_WAV
		#define SUPPORT_MID
		#define SUPPORT_OGG
		#define SUPPORT_MP3
	#endif

	#ifdef USE_SDL_TTF
		#define SUPPORT_TTF
		#define SUPPORT_FON
	#endif
#endif

#ifdef _WIN32
	#define DEFAULT_FONTS { "RM2000", "DejaVuLGCSansMono", "msgothic", "Lucida Console", "MS Sans Serif", "" }
#else
	#define DEFAULT_FONTS { "DejaVuLGCSansMono.ttf", "" }
#endif

#ifdef USE_SDL 
	#ifdef PSP
		#include "SDL.h"
	#else
		#include "SDL_stdinc.h"
	#endif
	typedef Sint8   int8;
	typedef Uint8   uint8;
	typedef Sint16  int16;
	typedef Uint16  uint16;
	typedef Sint32  int32;
	typedef Uint32  uint32;
	//typedef Sint64  int64;
	//typedef Uint64  uint64;

	typedef Uint32  uint;
#else
	#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
		#include <stdint.h>

		typedef int8_t		int8;
		typedef uint8_t		uint8;
		typedef int16_t		int16;
		typedef uint16_t	uint16;
		typedef int32_t		int32;
		typedef uint32_t	uint32;
		//typedef int64_t		int64;
		//typedef uint64_t	uint64;
		
		typedef uint32_t	uint;
	#else
		typedef signed char			int8;
		typedef	unsigned char		uint8;
		typedef	signed short		int16;
		typedef unsigned short		uint16;
		typedef	signed int			int32;
		typedef unsigned int		uint32;
		//typedef signed __int64	int64;
		//typedef unsigned __int64	uint64;

		typedef unsigned int	uint;
	#endif
#endif

#endif
