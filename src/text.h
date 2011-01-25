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

#include "ftfont.h"
#include "surface.h"

class Text {
public:
	static void Draw(Surface* dest, int x, int y, std::string text, Surface::TextAlignment align = Surface::TextAlignLeft);
	static void Draw(Surface* dest, int x, int y, std::wstring text, Surface::TextAlignment align = Surface::TextAlignLeft);
};

#endif
