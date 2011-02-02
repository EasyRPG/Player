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

#ifndef _FILEFINDER_H_
#define _FILEFINDER_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>
#include "system.h"

////////////////////////////////////////////////////////////
/// FileFinder contains helper methods for finding case
/// insensitive files paths.
////////////////////////////////////////////////////////////
namespace FileFinder {
	////////////////////////////////////////////////////////
	/// Initialize FileFinder.
	////////////////////////////////////////////////////////
	void Init();

	////////////////////////////////////////////////////////
	/// Next Init step that is performed after parsing
	/// the database file. The rtp paths should be added
	/// to the FileFinder here.
	////////////////////////////////////////////////////////
	void InitRtpPaths();

	////////////////////////////////////////////////////////
	/// Quit FileFinder.
	////////////////////////////////////////////////////////
	void Quit();
	
	///////////////////////////////////////////////////////
	/// Find an image file.
	/// @param name : the image path and name
	/// @return path to file
	///////////////////////////////////////////////////////
	std::string FindImage(const std::string& dir, const std::string& name);
	
	///////////////////////////////////////////////////////
	/// Find a file.
	/// @param name : the path and name
	/// @return path to file
	///////////////////////////////////////////////////////
	std::string FindDefault(const std::string& dir, const std::string& name);

	///////////////////////////////////////////////////////
	/// Find a music file.
	/// @param name : the music path and name
	/// @return path to file
	///////////////////////////////////////////////////////
	std::string FindMusic(const std::string& name);

	///////////////////////////////////////////////////////
	/// Find a sound file.
	/// @param name : the sound path and name
	/// @return path to file
	///////////////////////////////////////////////////////
	std::string FindSound(const std::string& name);

	///////////////////////////////////////////////////////
	/// Find a font file.
	/// @param name : the font name
	/// @return path to file
	///////////////////////////////////////////////////////
	std::string FindFont(const std::string& name);

	///////////////////////////////////////////////////////
	/// Get a default font.
	/// @return path to file
	///////////////////////////////////////////////////////
	std::string DefaultFont();

	///////////////////////////////////////////////////////
	/// Open a file specified by a UTF-8 string
	/// @param : filename in UTF-8
	/// @param : mode ("r", "w", etc)
	/// @return: FILE*
	///////////////////////////////////////////////////////
	FILE* fopenUTF8(const std::string& name_utf8, const std::string& mode);

	/// Available image extension types
	const char* const IMG_TYPES[] = {
#ifdef SUPPORT_BMP
		".bmp",
#endif
#ifdef SUPPORT_GIF
		".gif",
#endif
#ifdef SUPPORT_JPG
		".jpg",
		".jpeg",
#endif
#ifdef SUPPORT_PNG
		".png",
#endif
#ifdef SUPPORT_XYZ
		".xyz",
#endif
		NULL
	};

	/// Available audio music extension types
	const char* const MUSIC_TYPES[] = {
#ifdef SUPPORT_WAV
		".wav",
#endif
#ifdef SUPPORT_MID
		".mid",
		".midi",
#endif
#ifdef SUPPORT_OGG
		".ogg",
#endif
#ifdef SUPPORT_MP3
		".mp3",
#endif
		NULL
	};

	/// Available audio music extension types
	const char* const SOUND_TYPES[] = {
#ifdef SUPPORT_WAV
		".wav",
#endif
#ifdef SUPPORT_OGG
		".ogg",
#endif
#ifdef SUPPORT_MP3
		".mp3",
#endif
		NULL
	};

	/// Available fonts types
	const char* const FONTS_TYPES[] = {
#ifdef SUPPORT_TTF
		".ttf",
		".ttc",
#endif
#ifdef SUPPORT_FON
		".fon",
#endif
		NULL
	};
}

#endif
