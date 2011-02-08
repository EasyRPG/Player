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
#include "rect.h"
#include "pixel_format.h"

class BitmapScreen;

class BitmapUtils;

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
	static Bitmap* CreateBitmap(int width, int height, const Color& color);

	////////////////////////////////////////////////////////
	/// Loads a bitmap from an image file.
	/// @param filename : image file to load
	/// @param transparent : allow transparency on bitmap
	/// @param read_only : should bitmap be read_only (fast blit)
	////////////////////////////////////////////////////////
	static Bitmap* CreateBitmap(const std::string& filename, bool transparent = true, uint32 flags = 0);

	////////////////////////////////////////////////////////
	/// Loads a bitmap from memory.
	/// @param data : image data
	/// @param bytes : size of data
	/// @param transparent : allow transparency on bitmap
	////////////////////////////////////////////////////////
	static Bitmap* CreateBitmap(const uint8* data, uint bytes, bool transparent = true, uint32 flags = 0);

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
	virtual ~Bitmap();

	/// @return the bitmap width
	virtual int GetWidth() const;

	/// @return the bitmap height
	virtual int GetHeight() const;

	/// @return bitmap bounds rect
	virtual Rect GetRect() const;

	/// @return if bitmap allows transparency
	virtual bool GetTransparent() const;

	/// @return current transparent color
	virtual Color GetTransparentColor() const;

	/// @param color : new transparent color 
	virtual void SetTransparentColor(Color color);

	static const uint32 System  = 0x80000000;
	static const uint32 Chipset = 0x40000000;

	////////////////////////////////////////////////////////
	/// Create a resampled bitmap.
	/// @param scale_w : resampled width
	/// @param scale_h : resampled height
	/// @param src_rect : source rect to resample
	////////////////////////////////////////////////////////
	virtual Bitmap* Resample(int scale_w, int scale_h, const Rect& src_rect);

	enum TileOpacity {
		Opaque,
		Partial,
		Transparent
	};

	TileOpacity GetTileOpacity(int row, int col);

protected:
	friend class Surface;
	friend class BitmapScreen;
	friend class SdlBitmap;
	friend class SdlBitmapScreen;
	friend class GlBitmapScreen;
	friend class BitmapUtils;
	template <class T1, class T2> friend class BitmapUtilsT;

	Bitmap();

	virtual void* pixels() = 0;
	virtual int width() const = 0;
	virtual int height() const = 0;
	virtual uint16 pitch() const = 0;

	virtual uint8 bytes() const;
	virtual uint32 rmask() const;
	virtual uint32 gmask() const;
	virtual uint32 bmask() const;
	virtual uint32 amask() const;
	virtual uint32 colorkey() const;
	virtual uint8* pointer(int x, int y);

	virtual Color GetColor(uint32 color) const;
	virtual uint32 GetUint32Color(const Color &color) const;
	virtual uint32 GetUint32Color(uint8 r, uint8  g, uint8 b, uint8 a) const;
	virtual void GetColorComponents(uint32 color, uint8 &r, uint8 &g, uint8 &b, uint8 &a) const;

	virtual void AttachBitmapScreen(BitmapScreen* bitmap);
	virtual void DetachBitmapScreen(BitmapScreen* bitmap);

	virtual void Lock();
	virtual void Unlock();

	virtual BitmapUtils* Begin();
	virtual void End();

	////////////////////////////////////////////////////////
	/// Get a pixel color.
	/// @param x : pixel x
	/// @param y : pixel y
	/// @return pixel color
	////////////////////////////////////////////////////////
	virtual Color GetPixel(int x, int y);

	virtual TileOpacity CheckOpacity(const Rect& rect);

	virtual void CheckPixels(uint32 flags);

	DynamicFormat format;

	std::list<BitmapScreen*> attached_screen_bitmaps;
	TileOpacity (*opacity)[30];
};

#endif
