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

#ifndef _FONT_H_
#define _FONT_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <map>
#include "SDL_ttf.h"
#include "color.h"

////////////////////////////////////////////////////////////
/// Font class
////////////////////////////////////////////////////////////
class Font {
public:
	Font();
	Font(std::string name);
	Font(std::string name, int size);
	~Font();

	TTF_Font* GetTTF() const;
	
	std::string name;
	int size;
	bool bold;
	bool italic;
	Color color;
	
	static std::string default_name;
	static int default_size;
	static bool default_bold;
	static bool default_italic;
	static Color default_color;
	
	static bool Exists(std::string name);
	
private:
	static std::map<std::string, std::map<int, TTF_Font*> > fonts;
};

#endif
