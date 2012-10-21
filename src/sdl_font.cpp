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

#include "system.h"
#ifdef USE_SDL_TTF

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SDL.h>
#include <SDL_ttf.h>
#include "font.h"
#include "filefinder.h"
#include "output.h"
#include "utils.h"
#include "sdl_bitmap.h"
#include "sdl_font.h"

////////////////////////////////////////////////////////////

#ifdef USE_ALPHA
	#define DisplayFormat(surface) SDL_DisplayFormatAlpha(surface)
#else
	#define DisplayFormat(surface) SDL_DisplayFormat(surface)
#endif

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
SdlFont::SdlFont(const std::string& name, int size, bool bold, bool italic)
	: Font(name, size, bold, italic), ttf_font(NULL) {}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
SdlFont::~SdlFont() {
}

////////////////////////////////////////////////////////////
/// Get TTF_Font*
////////////////////////////////////////////////////////////
void SdlFont::GetTTF() {
	if (ttf_font != NULL)
		return;

	std::string path = FileFinder::FindFont(name);
	ttf_font = TTF_OpenFont(path.c_str(), size);
	if (!ttf_font) {
		Output::Error("Couldn't open font %s size %d.\n%s\n", name.c_str(), size, TTF_GetError());
	}

	int style = 0;
	if (bold) style |= TTF_STYLE_BOLD;
	if (italic) style |= TTF_STYLE_ITALIC;
	TTF_SetFontStyle(ttf_font, style);
}

////////////////////////////////////////////////////////////
/// Public methods
////////////////////////////////////////////////////////////
int SdlFont::GetHeight() {
	GetTTF();
	return TTF_FontHeight(ttf_font);
}

BitmapRef SdlFont::Render(int c) {
	GetTTF();
	SDL_Color color = {255, 255, 255, 255};
	uint16_t text[2] = {(uint16_t) c, 0};
	SDL_BitmapRef temp = TTF_RenderUNICODE_Solid(ttf_font, text, color);
	SDL_Color colorkey = temp->format->palette->colors[0];
	SDL_SetColorKey(temp, SDL_SRCCOLORKEY, SDL_MapRGB(temp->format, colorkey.r, colorkey.g, colorkey.b));
	SDL_BitmapRef surf = DisplayFormat(temp);
	SDL_FreeSurface(temp);
	return new SdlBitmap(surf);
}

#endif

