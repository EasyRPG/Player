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

#ifndef _EASYRPG_SDL_FONT_H_
#define _EASYRPG_SDL_FONT_H_

#include "system.h"
#ifdef USE_SDL_TTF

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <map>
#include <SDL_ttf.h>
#include "font.h"
#include "sdl_bitmap.h"

////////////////////////////////////////////////////////////
/// SdlFont class
////////////////////////////////////////////////////////////
class SdlFont : public Font {
public:
	SdlFont(const std::string& name, int size, bool bold, bool italic);
	~SdlFont();

	int GetHeight();
	Bitmap* Render(int glyph);

private:
	TTF_Font* ttf_font;
	void GetTTF();
};

#endif

#endif

