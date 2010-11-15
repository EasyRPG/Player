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

	///////////////////////////////////////////////////////
	/// Find an image file.
	/// @param name : the image path and name
	///////////////////////////////////////////////////////
	std::string FindImage(std::string name);

	///////////////////////////////////////////////////////
	/// Find a music file.
	/// @param name : the music path and name
	///////////////////////////////////////////////////////
	std::string FindMusic(std::string name);

	///////////////////////////////////////////////////////
	/// Find a sound file.
	/// @param name : the sound path and name
	///////////////////////////////////////////////////////
	std::string FindSound(std::string name);

	///////////////////////////////////////////////////////
	/// Find a font file.
	/// @param name : the font name
	///////////////////////////////////////////////////////
	std::string FindFont(std::string name);

	/// Available image extension types
	const std::string IMG_TYPES[] = {
#ifdef HAVE_BMP
		".bmp",
#endif
#ifdef HAVE_GIF
		".gif",
#endif
#ifdef HAVE_JPG
		".jpg",
		".jpeg",
#endif
#ifdef HAVE_PNG
		".png",
#endif
#ifdef HAVE_XYZ
		".xyz",
#endif
		""
	};

	/// Available audio music extension types
	const std::string MUSIC_TYPES[] = {
#ifdef HAVE_WAV
		".wav",
#endif
#ifdef HAVE_MID
		".mid",
		".midi",
#endif
#ifdef HAVE_OGG
		".ogg",
#endif
#ifdef HAVE_MP3
		".mp3",
#endif
		""
	};

	/// Available audio music extension types
	const std::string SOUND_TYPES[] = {
#ifdef HAVE_WAV
		".wav",
#endif
#ifdef HAVE_OGG
		".ogg",
#endif
#ifdef HAVE_MP3
		".mp3",
#endif
		""
	};

	/// Available fonts types
	const std::string FONTS_TYPES[] = {
#ifdef HAVE_TTF
		".ttf",
#endif
		""
	};
}
