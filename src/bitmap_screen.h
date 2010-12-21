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

#ifndef _BITMAP_SCREEN_H_
#define _BITMAP_SCREEN_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "bitmap.h"
#include "tone.h"

////////////////////////////////////////////////////////////
/// Base BitmapScreen class.
/// It includes blitting to screen methods.
/// This class must not be used by the Engine, it may only
/// be used by the backend itself.
////////////////////////////////////////////////////////////
class BitmapScreen {
public:
	////////////////////////////////////////////////////////
	/// Creates a BitmapScreen object.
	/// @param source : source bitmap, if different from
	///		NULL it will be deleted together with the
	///		BitmapScreen object.
	////////////////////////////////////////////////////////
	static BitmapScreen* CreateBitmapScreen(Bitmap* source);

	////////////////////////////////////////////////////////
	/// Creates a BitmapScreen object.
	/// @param delete_bitmap : if true the bitmaps set to
	///     this object will be deleted together with the
	///		BitmapScreen object.
	////////////////////////////////////////////////////////
	static BitmapScreen* CreateBitmapScreen(bool delete_bitmap);

	////////////////////////////////////////////////////////
	/// Destructor.
	////////////////////////////////////////////////////////
	virtual ~BitmapScreen() {}

	////////////////////////////////////////////////////////
	/// Marks the BitmapScreen as dirty.
	////////////////////////////////////////////////////////
	virtual void SetDirty() = 0;

	////////////////////////////////////////////////////////
	/// Set source bitmap.
	/// @param source : source bitmap
	////////////////////////////////////////////////////////
	virtual void SetBitmap(Bitmap* source) = 0;

	////////////////////////////////////////////////////////
	/// Get source bitmap.
	/// @return source bitmap
	////////////////////////////////////////////////////////
	virtual Bitmap* GetBitmap() = 0;

	////////////////////////////////////////////////////////
	/// Blit the bitmap to the screen.
	/// @param x : x position
	/// @param y : y position
	////////////////////////////////////////////////////////
	virtual void BlitScreen(int x, int y) = 0;

	////////////////////////////////////////////////////////
	/// Blit the bitmap to the screen.
	/// @param x : x position
	/// @param y : y position
	/// @param src_rect : source rect
	////////////////////////////////////////////////////////
	virtual void BlitScreen(int x, int y, Rect src_rect) = 0;

	////////////////////////////////////////////////////////
	/// Blit the bitmap to the screen.
	/// @param src_rect : source bitmap rect
	/// @param dst_rect : screen destination rect
	////////////////////////////////////////////////////////
	virtual void BlitScreenTiled(Rect src_rect, Rect dst_rect) = 0;

	////////////////////////////////////////////////////////
	/// Clear all effects data.
	////////////////////////////////////////////////////////
	virtual void ClearEffects() = 0;

	////////////////////////////////////////////////////////
	/// Flash effect.
	/// @param color : flash color
	/// @param duration : flash duration
	////////////////////////////////////////////////////////
	virtual void SetFlashEffect(const Color &color, int duration) = 0;

	////////////////////////////////////////////////////////
	/// Flash effect update.
	/// @param frame : frame of flash animation
	////////////////////////////////////////////////////////
	virtual void UpdateFlashEffect(int frame) = 0;

	/// @return source rect
	virtual Rect GetSrcRect() const = 0;

	/// @param src_rect : source rect
	virtual void SetSrcRect(Rect src_rect) = 0;

	/// @return bitmap opacity
	virtual int GetOpacityEffect() const = 0;

	/// @param opacity : bitmap opacity
	virtual void SetOpacityEffect(int opacity) = 0;

	/// @return bush depth effect
	virtual int GetBushDepthEffect() const = 0;

	/// @param bush_depth : bush depth effect
	virtual void SetBushDepthEffect(int bush_depth) = 0;

	/// @return tone effect
	virtual Tone GetToneEffect() const = 0;

	/// @param tone : tone effect
	virtual void SetToneEffect(Tone tone) = 0;

	/// @return horizontal flip
	virtual bool GetFlipXEffect() const = 0;

	/// @param flipx : horizontal flip
	virtual void SetFlipXEffect(bool flipx) = 0;

	/// @return vertical flip
	virtual bool GetFlipYEffect() const = 0;

	/// @param flipy : vertical flip
	virtual void SetFlipYEffect(bool flipy) = 0;

	/// @return horizontal zoom
	virtual double GetZoomXEffect() const = 0;

	/// @param zoom_x : horizontal zoom
	virtual void SetZoomXEffect(double zoom_x) = 0;

	/// @return vertical zoom
	virtual double GetZoomYEffect() const = 0;

	/// @param zoom_y : vertical zoom
	virtual void SetZoomYEffect(double zoom_y) = 0;

	/// @return angle of rotation in degrees
	virtual double GetAngleEffect() const = 0;

	/// @param angle : angle of rotation in degrees
	virtual void SetAngleEffect(double angle) = 0;

	/// @return blend type
	virtual int GetBlendType() const = 0;

	/// @param blend_type : blend type
	virtual void SetBlendType(int blend_type) = 0;

	/// @return blend color
	virtual Color GetBlendColor() const = 0;

	/// @param blend_color : blend color
	virtual void SetBlendColor(Color blend_color) = 0;

protected:
	BitmapScreen() {}
};

#endif
