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

#ifndef _SDL_BITMAP_H_
#define _SDL_BITMAP_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "bitmap.h"
#include "SDL.h"

////////////////////////////////////////////////////////////
/// SdlBitmap class.
////////////////////////////////////////////////////////////
class SdlBitmap : public Bitmap {
public:
	SdlBitmap(int width, int height, bool transparent = true);
	SdlBitmap(const std::string filename, bool transparent = true);
	SdlBitmap(const uint8* data, uint bytes, bool transparent = true);
	SdlBitmap(Bitmap* source, Rect src_rect, bool transparent = true);

	~SdlBitmap();

	void Blit(int x, int y, Bitmap* src, Rect src_rect, int opacity);
	void StretchBlit(Rect dst_rect, Bitmap* src, Rect src_rect, int opacity);
	void Fill(const Color &color);
	void FillRect(Rect dst_rect, const Color &color);
	void TextDraw(Rect dst_rect, std::string text, TextAlignment align = Bitmap::TextAlignLeft);
	Rect GetTextSize(std::string text) const;
	void SetTransparentColor(Color color);

	void* pixels();
	int width() const;
	int height() const;
	uint8 bpp() const;
	uint16 pitch() const;
	uint32 rmask() const;
	uint32 gmask() const;
	uint32 bmask() const;
	uint32 amask() const;
	uint32 colorkey() const;

protected:
	friend class SdlBitmapScreen;

	SdlBitmap(SDL_Surface* bitmap, bool transparent = true);

	//void SetupBitmapData();

	void RemovePalleteColorkeyDuplicates(SDL_Surface* src, SDL_Color* color);

	/// Bitmap data.
	SDL_Surface* bitmap;

	Color GetColor(uint32 color) const;
	uint32 GetUint32Color(const Color &color) const;
	uint32 GetUint32Color(uint8 r, uint8  g, uint8 b, uint8 a) const;
	void GetColorComponents(uint32 color, uint8 &r, uint8 &g, uint8 &b, uint8 &a) const;

	void Lock();
	void Unlock();
};

#endif
