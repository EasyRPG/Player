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

#ifndef _COLOR_H_
#define _COLOR_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SDL.h"

////////////////////////////////////////////////////////////
/// Color class
////////////////////////////////////////////////////////////
class Color {

public:
	Color();
	Color(int ired, int igreen, int iblue, int ialpha);
	Color(Uint32 color, SDL_PixelFormat* format);
	~Color();

	bool operator!=(const Color& other) const;

	void Set(int nred, int ngreen, int nblue, int nalpha);

	enum ColorTypes {
		Default = 0,
		Disabled = 3
	};

	int red;
	int green;
	int blue;
	int alpha;

	Uint32 GetUint32(SDL_PixelFormat* format) const;
	SDL_Color Get() const;
};

#endif
