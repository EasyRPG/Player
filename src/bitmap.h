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

#ifndef _BITMAP_H_
#define _BITMAP_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>
#include "SDL.h"
#include "rect.h"
#include "color.h"
#include "tone.h"
#include "font.h"

////////////////////////////////////////////////////////////
/// Bitmap class
////////////////////////////////////////////////////////////
class Bitmap {
public:
	Bitmap(int width, int height);
	Bitmap(std::string filename, bool transparent);
	Bitmap(Bitmap* source, Rect& src_rect);
	~Bitmap();

	enum TextAlignment {
		align_left = 0,
		align_center,
		align_right
	};

	void BlitScreen(int x, int y);
	void BlitScreen(int x, int y, int opacity);
	void BlitScreen(int x, int y, Rect& src_rect, int opacity = 255);
	
	int GetWidth() const;
	int GetHeight() const;

	Uint32 GetColorKey() const;
	void SetColorKey(Uint32 color);

	void RemoveColorDuplicates(SDL_Surface* src, SDL_Color* src_color);

	void SetClipRect(const Rect& clip_rect);
	void ClearClipRect();

	void TileBlitX(const Rect& src_rect, Bitmap* src, const Rect& dst_rect);
	void TileBlitY(const Rect& src_rect, Bitmap* src, const Rect& dst_rect);

	void Blit(int x, int y, Bitmap* source, Rect& src_rect, int opacity);
	void StretchBlit(Bitmap* src, Rect& src_rect);
	void StretchBlit(const Rect& dst_rect, Bitmap* src_bitmap, Rect& src_rect, int opacity);
	void FillRect(Rect& rect, Color color);
	void FillofColor(const Rect& rect, Uint32 color);
	void Clear();
	void Clear(Color color);
	Color GetPixel(int x, int y);
	void SetPixel(int x, int y, Color color);
	void HueChange(double hue);
	void SatChange(double saturation);
	void LumChange(double luminance);
	void HSLChange(double h, double s, double l);
	void HSLChange(double h, double s, double l, Rect rect);
	void TextDraw(Rect rect, std::string text, TextAlignment align = Bitmap::align_left);
	Rect GetTextSize(std::string text);
	void GradientFillRect(Rect rect, Color color1, Color color2, bool vertical);
	void ClearRect(Rect rect);
	void Blur();
	void RadialBlur(int angle, int division);

	void SetTransparent(Color col);
	void ToneChange(Tone tone);
	void OpacityChange(int opacity, int bush_depth = 0);
	void Flip(bool flipx, bool flipy);
	void Zoom(double zoom_x, double zoom_y);
	Bitmap* Resample(int scalew, int scaleh, Rect src_rect);
	void Rotate(double angle);
	void Flash(Color color, int frame, int duration);
	
	Rect GetRect() const;
	Font* GetFont();
	void SetFont(Font nfont);

protected:
	SDL_Surface* bitmap;

private:
	static int MaskGetByte(Uint32 mask);

	Font font;
};

#endif
