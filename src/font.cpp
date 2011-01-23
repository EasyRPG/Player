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
#include "filefinder.h"
#include "output.h"
#include "system.h"
#include "font.h"
#ifdef USE_SDL_TTF
#include "sdl_font.h"
#else
#include "ftfont.h"
#endif

////////////////////////////////////////////////////////////
/// Static Variables
////////////////////////////////////////////////////////////
const std::string Font::default_name = FileFinder::DefaultFont();
const int Font::default_size = 9;
const bool Font::default_bold = false;
const bool Font::default_italic = false;
const int Font::default_color = 0;

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
Font::Font():
	name(default_name),
	size(default_size),
	bold(default_bold),
	italic(default_italic),
	color(default_color) {
}

Font::Font(int _size):
	name(default_name),
	size(_size),
	bold(default_bold),
	italic(default_italic),
	color(default_color) {

}

Font::Font(std::string _name):
	name(_name),
	size(default_size),
	bold(default_bold),
	italic(default_italic),
	color(default_color) {
}
Font::Font(std::string _name, int _size):
	name(_name),
	size(_size),
	bold(default_bold),
	italic(default_italic),
	color(default_color) {
}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
Font::~Font() {
}

////////////////////////////////////////////////////////////
/// Class Exists
////////////////////////////////////////////////////////////
bool Font::Exists(std::string name) {
	return FileFinder::FindFont(name) != "";
}

////////////////////////////////////////////////////////////
/// Factory
////////////////////////////////////////////////////////////
Font* Font::CreateFont() {
#ifdef USE_SDL_TTF
	return new SdlFont();
#else
	return new FTFont();
#endif
}

////////////////////////////////////////////////////////////
/// Cleanup
////////////////////////////////////////////////////////////
void Font::Dispose() {
#ifdef USE_SDL_TTF
	SdlFont::Dispose();
#else
	FTFont::Dispose();
#endif
}

