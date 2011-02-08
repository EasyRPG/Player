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

#ifndef _SOFT_BITMAP_H_
#define _SOFT_BITMAP_H_

#include "system.h"
#ifdef USE_SOFT_BITMAP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>

#include "surface.h"
#include "pixel_format.h"
#include "bitmap_utils.h"

////////////////////////////////////////////////////////////
/// SoftBitmap class.
////////////////////////////////////////////////////////////
class SoftBitmap : public Surface {
public:
	SoftBitmap(int width, int height, bool transparent);
	SoftBitmap(void *pixels, int width, int height, int pitch, const DynamicFormat& format);
	SoftBitmap(const std::string& filename, bool transparent, uint32 flags);
	SoftBitmap(const uint8* data, uint bytes, bool transparent, uint32 flags);
	SoftBitmap(Bitmap* source, Rect src_rect, bool transparent);
	~SoftBitmap();

	static DynamicFormat ChooseFormat(const DynamicFormat& format);
	static void SetFormat(const DynamicFormat& format);

protected:
	friend class SoftBitmapScreen;

	/// Bitmap data.
	int w, h;
	int _pitch;
	void* bitmap;
	bool destroy;

	void* pixels();
	int width() const;
	int height() const;
	uint16 pitch() const;

	void Init(int width, int height, void* data, int pitch = 0, bool destroy = true);
	void ConvertImage(int& width, int& height, void*& pixels, bool transparent);

	static bool initialized;
	static DynamicFormat pixel_format;
	static DynamicFormat opaque_pixel_format;
	static DynamicFormat image_format;
	static DynamicFormat opaque_image_format;
};

#endif

#endif
