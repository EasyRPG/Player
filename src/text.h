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

#ifndef _EASYRPG_TEXT_H_
#define _EASYRPG_TEXT_H_

#include "system.h"
#include <string>

#ifdef _WIN32
#  define utf(x) L##x
#else
#  define utf(x) x
#endif

namespace Text {
	/// TextDraw alignment options
	enum Alignment {
		AlignLeft,
		AlignCenter,
		AlignRight
	};

	void Draw(Bitmap& dest, int x, int y, int color, std::string const& text, Text::Alignment align = Text::AlignLeft);

	/**
	 * Draws text directly dest without memory allocation (used to render errors)
	 */
	void DirectDraw(Bitmap& dest, int x, int y, Color color, std::string const& text);
}; // namespace Text

#endif
