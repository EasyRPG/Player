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

#ifndef _WIN32
#ifndef UNIX
#ifndef DINGOO
#ifndef GEKKO
	#error "This build doesn't target an os"
#endif
#endif
#endif
#endif

#ifdef USE_OPENGL
	#define HAVE_BMP
	#define HAVE_GIF
	//#define HAVE_JPG
	#define HAVE_PNG
#endif

#ifdef USE_SDL
	#ifndef USE_OPENGL
		#define USE_SDL_IMAGE
	#endif
	#define USE_SDL_MIXER
	#define USE_SDL_TTF
	//#define USE_ALPHA
#endif

//#define USE_FIXED_TIMESTEP_FPS

#ifdef USE_SDL_IMAGE
	#define HAVE_BMP
	#define HAVE_GIF
	//#define HAVE_JPG
	#define HAVE_PNG
#endif

#ifdef USE_SDL_MIXER
	#define HAVE_WAV
	#define HAVE_MID
	#define HAVE_OGG
	#define HAVE_MP3
#endif

#ifdef USE_SDL_TTF
	#define HAVE_TTF
#endif

#define HAVE_XYZ

#ifdef _WIN32
	#define DEFAULT_FONTS { "Font/DejaVuLGCSansMono.ttf", "Lucida Console", "MS Sans Serif", "" }
#endif

#endif
