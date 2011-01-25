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
/// Static Variables
////////////////////////////////////////////////////////////
std::map<std::string, std::map<int, TTF_Font*> > SdlFont::fonts;

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
SdlFont::SdlFont()
	: Font(), ttf_font(NULL) {}

SdlFont::SdlFont(int _size)
	: Font(_size), ttf_font(NULL) {}

SdlFont::SdlFont(std::string _name)
	: Font(_name), ttf_font(NULL) {}

SdlFont::SdlFont(std::string _name, int _size)
	: Font(_name, _size), ttf_font(NULL) {}

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

	if (fonts.count(name) > 0 && fonts[name].count(size) > 0) {
		ttf_font = fonts[name][size];
	} else {
		std::string path = FileFinder::FindFont(name);
		ttf_font = TTF_OpenFont(path.c_str(), size);
		if (!ttf_font) {
			Output::Error("Couldn't open font %s size %d.\n%s\n", name.c_str(), size, TTF_GetError());
		}
		fonts[name][size] = ttf_font;
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

Bitmap* SdlFont::Render(int c) {
	GetTTF();
	SDL_Color color = {255, 255, 255, 255};
	uint16 text[2] = {(uint16) c, 0};
	SDL_Surface* temp = TTF_RenderUNICODE_Solid(ttf_font, text, color);
	SDL_Color colorkey = temp->format->palette->colors[0];
	SDL_SetColorKey(temp, SDL_SRCCOLORKEY, SDL_MapRGB(temp->format, colorkey.r, colorkey.g, colorkey.b));
	SDL_Surface* surf = DisplayFormat(temp);
	SDL_FreeSurface(temp);
	return new SdlBitmap(surf);
}

////////////////////////////////////////////////////////////
/// Cleanup
////////////////////////////////////////////////////////////
void SdlFont::Dispose() {
	std::map<int, TTF_Font*>::iterator it;
	std::map<std::string, std::map<int, TTF_Font*> >::iterator it2;

	for (it2 = fonts.begin(); it2 != fonts.end(); ++it2) {
		for (it = it2->second.begin(); it != it2->second.end(); ++it) {
			TTF_CloseFont(it->second);
		}
	}
}

#endif

