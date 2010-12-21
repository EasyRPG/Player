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
#include <list>
#include "color.h"
#include "font.h"
#include "rect.h"
#include "tone.h"

class BitmapScreen;

////////////////////////////////////////////////////////////
/// Base Bitmap class.
////////////////////////////////////////////////////////////
class Bitmap {
public:
	////////////////////////////////////////////////////////
	/// Creates bitmap with empty surface.
	/// @param width : surface width
	/// @param height : surface height
	/// @param transparent : allow transparency on bitmap
	////////////////////////////////////////////////////////
	static Bitmap* CreateBitmap(int width, int height, bool transparent = true);

	////////////////////////////////////////////////////////
	/// Loads a bitmap from an image file.
	/// @param filename : image file to load
	/// @param transparent : allow transparency on bitmap
	////////////////////////////////////////////////////////
	static Bitmap* CreateBitmap(const std::string filename, bool transparent = true);

	////////////////////////////////////////////////////////
	/// Loads a bitmap from memory.
	/// @param data : image data
	/// @param bytes : size of data
	/// @param transparent : allow transparency on bitmap
	////////////////////////////////////////////////////////
	static Bitmap* CreateBitmap(const uint8* data, uint bytes, bool transparent = true);

	////////////////////////////////////////////////////////
	/// Creates a bitmap from another.
	/// @param source : source bitmap
	/// @param src_rect : rect to copy from source bitmap
	/// @param transparent : allow transparency on bitmap
	////////////////////////////////////////////////////////
	static Bitmap* CreateBitmap(Bitmap* source, Rect src_rect, bool transparent = true);

	////////////////////////////////////////////////////////
	/// Destructor.
	////////////////////////////////////////////////////////
	virtual ~Bitmap() {}
	
	////////////////////////////////////////////////////////
	/// Blit source bitmap to this one.
	/// @param x : x position
	/// @param y : y position
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	/// @param opacity : opacity for blending with bitmap
	////////////////////////////////////////////////////////
	virtual void Blit(int x, int y, Bitmap* src, Rect src_rect, int opacity);

	////////////////////////////////////////////////////////
	/// Blit source bitmap in tiles to this one.
	/// @param src_rect : source bitmap rect
	/// @param src : source bitmap
	/// @param dst_rect : destination rect
	/// @param opacity : opacity for blending with bitmap
	////////////////////////////////////////////////////////
	virtual void TiledBlit(Rect src_rect, Bitmap* src, Rect dst_rect, int opacity);

	////////////////////////////////////////////////////////
	/// Blit source bitmap in tiles to this one.
	/// @param ox : tile start x offset
	/// @param ox : tile start y offset
	/// @param src_rect : source bitmap rect
	/// @param src : source bitmap
	/// @param dst_rect : destination rect
	/// @param opacity : opacity for blending with bitmap
	////////////////////////////////////////////////////////
	virtual void TiledBlit(int ox, int oy, Rect src_rect, Bitmap* src, Rect dst_rect, int opacity);

	////////////////////////////////////////////////////////
	/// Blit source bitmap stretched to this one.
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	/// @param opacity : opacity for blending with bitmap
	////////////////////////////////////////////////////////
	virtual void StretchBlit(Bitmap* src, Rect src_rect, int opacity);

	////////////////////////////////////////////////////////
	/// Blit source bitmap stretched to this one.
	/// @param dst_rect : destination rect
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	/// @param opacity : opacity for blending with bitmap
	////////////////////////////////////////////////////////
	virtual void StretchBlit(Rect dst_rect, Bitmap* src, Rect src_rect, int opacity);

	////////////////////////////////////////////////////////
	/// Fill entire bitmap with color.
	/// @param color : color for filling
	////////////////////////////////////////////////////////
	virtual void Fill(const Color &color);

	////////////////////////////////////////////////////////
	/// Fill bitmap rect with color.
	/// @param dst_rect : destination rect
	/// @param color : color for filling
	////////////////////////////////////////////////////////
	virtual void FillRect(Rect dst_rect, const Color &color);

	////////////////////////////////////////////////////////
	/// Clears the bitmap with transparent pixels.
	////////////////////////////////////////////////////////
	virtual void Clear();

	////////////////////////////////////////////////////////
	/// Clears the bitmap rect with transparent pixels.
	/// @param dst_rect : destination rect
	////////////////////////////////////////////////////////
	virtual void ClearRect(Rect dst_rect);
	
	////////////////////////////////////////////////////////
	/// Get a pixel color.
	/// @param x : pixel x
	/// @param y : pixel y
	/// @return pixel color
	////////////////////////////////////////////////////////
	virtual Color GetPixel(int x, int y);

	////////////////////////////////////////////////////////
	/// Get a pixel color.
	/// @param x : pixel x
	/// @param y : pixel y
	/// @param color : pixel color
	////////////////////////////////////////////////////////
	virtual void SetPixel(int x, int y, const Color &color);

	////////////////////////////////////////////////////////
	/// Rotate bitmap hue.
	/// @param h : hue
	////////////////////////////////////////////////////////
	virtual void HueChange(double h);

	////////////////////////////////////////////////////////
	/// Adjust bitmap HSL colors.
	/// @param h : hue, default 0.0
	/// @param s : saturation, default 0.0
	/// @param l : luminance, default 1.0
	/// @param dst_rect : destination rect
	////////////////////////////////////////////////////////
	virtual void HSLChange(double h, double s, double l, Rect dst_rect);

	////////////////////////////////////////////////////////
	/// Adjust bitmap tone.
	/// @param tone : tone to apply
	////////////////////////////////////////////////////////
	virtual void ToneChange(const Tone &tone);

	////////////////////////////////////////////////////////
	/// Flips the bitmap pixels.
	/// @param horizontal : flip horizontally (mirror)
	/// @param vertical : flip vertically
	////////////////////////////////////////////////////////
	virtual void Flip(bool horizontal, bool vertical);

	////////////////////////////////////////////////////////
	/// Create a resampled bitmap.
	/// @param scale_w : resampled width
	/// @param scale_h : resampled height
	/// @param src_rect : source rect to resample
	////////////////////////////////////////////////////////
	virtual Bitmap* Resample(int scale_w, int scale_h, Rect src_rect);

	/// TextDraw alignment options
	enum TextAlignment {
		TextAlignLeft,
		TextAlignCenter,
		TextAlignRight
	};

	////////////////////////////////////////////////////////
	/// Draws text to bitmap.
	/// @param dst_rect : destination rect
	/// @param text : text to draw
	/// @param align : text alignment inside dst_rect
	////////////////////////////////////////////////////////
	virtual void TextDraw(Rect dst_rect, std::string text, TextAlignment align = Bitmap::TextAlignLeft) = 0;

	////////////////////////////////////////////////////////
	/// Get space needed to draw some text.
	/// @param text : text to draw
	////////////////////////////////////////////////////////
	virtual Rect GetTextSize(std::string text) const = 0;

	/// @return the bitmap width
	virtual int GetWidth() const;

	/// @return the bitmap height
	virtual int GetHeight() const;

	/// @return bitmap bounds rect
	virtual Rect GetRect() const;

	/// @return text drawing font
	virtual Font* GetFont() const;

	/// @param text drawing font
	virtual void SetFont(Font* font);

	/// @return if bitmap allows transparency
	virtual bool GetTransparent() const;

	/// @return current transparent color
	virtual Color GetTransparentColor() const;

	/// @param color : new transparent color 
	virtual void SetTransparentColor(Color color) = 0;

	////////////////////////////////////////////////////////
	/// Set the bitmap not to update its attached
	/// BitmapScreen objects until EndEditing is called.
	/// This way when multiple operation take place, the
	/// attached BitmapScreen objects will be set dirty
	/// only one time.
	////////////////////////////////////////////////////////
	virtual void BeginEditing();

	////////////////////////////////////////////////////////
	/// Set all attached BitmapScreen objects dirty and
	/// restore normal updates.
	////////////////////////////////////////////////////////
	virtual void EndEditing();

protected:
	friend class BitmapScreen;

#ifdef USE_SDL
	friend class SdlBitmapScreen;
#endif

	Bitmap();

	void* pixels;
	int width;
	int height;
	int bpp;
	int pitch;
	uint32 rmask;
	uint32 gmask;
	uint32 bmask;
	uint32 amask;

	bool transparent;
	uint32 colorkey;

	/// Font for text drawing.
	Font* font;

	virtual Color GetColor(uint32 color) const = 0;
	virtual uint32 GetUint32Color(const Color &color) const = 0;
	virtual uint32 GetUint32Color(uint8 r, uint8  g, uint8 b, uint8 a) const = 0;
	virtual void GetColorComponents(uint32 color, uint8 &r, uint8 &g, uint8 &b, uint8 &a) const = 0;

	virtual void Lock() = 0;
	virtual void Unlock() = 0;

	virtual void AttachBitmapScreen(BitmapScreen* bitmap);
	virtual void DetachBitmapScreen(BitmapScreen* bitmap);

	virtual void RefreshCallback();

	std::list<BitmapScreen*> attached_screen_bitmaps;
	bool editing;
};

#endif
