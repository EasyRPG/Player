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

#ifndef _RECT_H_
#define _RECT_H_

#include "system.h"

////////////////////////////////////////////////////////////
/// Rect.
////////////////////////////////////////////////////////////
class Rect {
public:
	///////////////////////////////////////////////////////
	/// Constructor. All values are set to 0.
	///////////////////////////////////////////////////////
	Rect();

	///////////////////////////////////////////////////////
	/// Constructor.
	/// @param x : initial x
	/// @param x : initial y
	/// @param width : initial width
	/// @param height : initial height
	///////////////////////////////////////////////////////
	Rect(int x, int y, int width, int height);

	///////////////////////////////////////////////////////
	/// Equality operator.
	///////////////////////////////////////////////////////
	bool operator==(const Rect& other) const;

	///////////////////////////////////////////////////////
	/// Inequality operator.
	///////////////////////////////////////////////////////
	bool operator!=(const Rect& other) const;

	///////////////////////////////////////////////////////
	/// Set all rect values simultaneously.
	/// @param x : new x
	/// @param x : new y
	/// @param width : new width
	/// @param height : new height
	///////////////////////////////////////////////////////
	void Set(int x, int y, int width, int height);

	///////////////////////////////////////////////////////
	/// Adjusts rect bounds, trimming it so it is contained
	/// inside the given size.
	/// @param width : max width
	/// @param height : max height
	///////////////////////////////////////////////////////
	void Adjust(int width, int height);

	///////////////////////////////////////////////////////
	/// Gets if rect is totally outside the given size.
	/// @param width : max width
	/// @param height : max height
	/// @return whether the rect is outside of the scope
	///////////////////////////////////////////////////////
	bool IsOutOfBounds(int width, int height) const;

	/// X coordinate.
	int x;

	/// Y coordinate.
	int y;

	/// Width.
	int width;

	/// Height.
	int height;
};

#endif
