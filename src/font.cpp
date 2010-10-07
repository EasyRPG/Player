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
#include "font.h"
#include "filefinder.h"
#include "output.h"

////////////////////////////////////////////////////////////
/// Static Variables
////////////////////////////////////////////////////////////
std::string Font::default_name = "Font/DejaVuLGCSansMono";
int Font::default_size = 12;
bool Font::default_bold = false;
bool Font::default_italic = false;
Color Font::default_color(255, 255, 255, 255);
std::map<std::string, std::map<int, TTF_Font*> > Font::fonts;

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
Font::Font() {
	name = default_name;
	size = default_size;
	bold = default_bold;
	italic = default_italic;
	color = default_color;
}
Font::Font(std::string name) {
	this->name = name;
	size = default_size;
	bold = default_bold;
	italic = default_italic;
	color = default_color;
}
Font::Font(std::string name, int size) {
	this->name = name;
	this->size = size;
	bold = default_bold;
	italic = default_italic;
	color = default_color;
}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
Font::~Font() {
}

////////////////////////////////////////////////////////////
/// Get TTF_Font*
////////////////////////////////////////////////////////////
TTF_Font* Font::GetTTF() const {
	if (fonts.count(name) > 0 && fonts[name].count(size) > 0) {
		return fonts[name][size];
	} else {
		std::string path = FileFinder::FindFont(name);
		TTF_Font* ttf_font = TTF_OpenFont(path.c_str(), size);
		if (!ttf_font) {
			Output::Error("Couldn't open font %s size %d.\n%s\n", name.c_str(), size, TTF_GetError());
		}
		fonts[name][size] = ttf_font;
		return ttf_font;
	}
}

////////////////////////////////////////////////////////////
/// Class Exists
////////////////////////////////////////////////////////////
bool Font::Exists(std::string name) {
	return FileFinder::FindFont(name) != "";
}
