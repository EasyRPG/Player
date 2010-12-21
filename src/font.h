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
	Font(std::string _name);
	Font(int _size);
	Font(std::string _name, int _size);
	~Font();

	TTF_Font* GetTTF() const;

	static void Dispose();
	
	std::string name;
	int size;
	bool bold;
	bool italic;
	int color;
	
	static const std::string default_name;
	static const int default_size;
	static const bool default_bold;
	static const bool default_italic;
	static const int default_color;
	
	static bool Exists(std::string name);
	
	enum SystemColor {
		ColorDefault = 0,
		ColorDisabled = 3,
		ColorCritical = 4,
		ColorKnockout = 5
	};

private:
	// TODO Where's the clean up for this?
	static std::map<std::string, std::map<int, TTF_Font*> > fonts;
};

#endif
