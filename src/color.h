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
#include "system.h"

////////////////////////////////////////////////////////////
/// Color class.
////////////////////////////////////////////////////////////
class Color {
public:
	////////////////////////////////////////////////////////
	/// Constructor. All components are set to 0, but alpha
	/// that is set to 255.
	////////////////////////////////////////////////////////
	Color();

	////////////////////////////////////////////////////////
	/// Constructor.
	/// @param red : red component
	/// @param green : green component
	/// @param blue : blue component
	/// @param alpha : alpha component
	////////////////////////////////////////////////////////
	Color(uint red, uint green, uint blue, uint alpha);

	////////////////////////////////////////////////////////
	/// Constructor.
	/// @param red : red component
	/// @param green : green component
	/// @param blue : blue component
	/// @param alpha : alpha component
	////////////////////////////////////////////////////////
	Color(int red, int green, int blue, int alpha);

	////////////////////////////////////////////////////////
	/// Equality operator.
	////////////////////////////////////////////////////////
	bool operator==(const Color &other) const;

	////////////////////////////////////////////////////////
	/// Inequality operator.
	////////////////////////////////////////////////////////
	bool operator!=(const Color &other) const;

	////////////////////////////////////////////////////////
	/// Set all color properties.
	/// @param red : red component
	/// @param green : green component
	/// @param blue : blue component
	/// @param alpha : alpha component
	////////////////////////////////////////////////////////
	void Set(uint red, uint green, uint blue, uint alpha);

	/// Red component.
	uint8 red;

	/// Green component.
	uint8 green;

	/// Blue component.
	uint8 blue;

	/// Alpha component.
	uint8 alpha;
};

#endif
