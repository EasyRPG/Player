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
#include "surface.h"
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
	/// @param source : source bitmap, or NULL.
	/// @param delete_bitmap : if true, the bitmap will be 
	///      deleted when it is replaced or when this object
    ///      is deleted.
	////////////////////////////////////////////////////////
	static BitmapScreen* CreateBitmapScreen(Bitmap* source, bool delete_bitmap = false);

	////////////////////////////////////////////////////////
	/// Creates a BitmapScreen object with no attached bitmap
	////////////////////////////////////////////////////////
	static BitmapScreen* CreateBitmapScreen();

	////////////////////////////////////////////////////////
	/// Destructor.
	////////////////////////////////////////////////////////
	virtual ~BitmapScreen();

	////////////////////////////////////////////////////////
	/// Marks the BitmapScreen as dirty.
	////////////////////////////////////////////////////////
	virtual void SetDirty();

	////////////////////////////////////////////////////////
	/// Set source bitmap.
	/// @param source : source bitmap
	/// @param delete_bitmap : if true, the bitmap will be 
	///      deleted when it is replaced or when this object
    ///      is deleted.
	////////////////////////////////////////////////////////
	virtual void SetBitmap(Bitmap* source, bool delete_bitmap = false);

	////////////////////////////////////////////////////////
	/// Get source bitmap.
	/// @return source bitmap
	////////////////////////////////////////////////////////
	virtual Bitmap* GetBitmap();

	////////////////////////////////////////////////////////
	/// Blit the bitmap to the screen.
	/// @param x : x position
	/// @param y : y position
	////////////////////////////////////////////////////////
	virtual void BlitScreen(int x, int y);

	////////////////////////////////////////////////////////
	/// Blit the bitmap to the screen.
	/// @param x : x position
	/// @param y : y position
	/// @param src_rect : source rect
	////////////////////////////////////////////////////////
	virtual void BlitScreen(int x, int y, Rect src_rect);

	////////////////////////////////////////////////////////
	/// Blit the bitmap to the screen.
	/// @param src_rect : source bitmap rect
	/// @param dst_rect : screen destination rect
	////////////////////////////////////////////////////////
	virtual void BlitScreenTiled(Rect src_rect, Rect dst_rect, int ox, int oy);

	////////////////////////////////////////////////////////
	/// Clear all effects data.
	////////////////////////////////////////////////////////
	virtual void ClearEffects();

	/// @return source rect
	virtual Rect GetSrcRect() const;

	/// @param src_rect : source rect
	virtual void SetSrcRect(Rect src_rect);

	/// @return bitmap opacity
	/// @param which : 0 => top, 1 => bottom (below bush depth)
	virtual int GetOpacityEffect(int which = 0) const;

	/// @param opacity_top : bitmap top opacity (above bush depth)
	/// @param opacity_bottom : bitmap bottom opacity (below bush depth)
	///  note: opacity_bottom == -1 => opacity_bottom = (opacity_top + 1) / 2
	virtual void SetOpacityEffect(int opacity_top, int opacity_bottom = -1);

	/// @return bush depth effect
	virtual int GetBushDepthEffect() const;

	/// @param bush_depth : bush depth effect
	virtual void SetBushDepthEffect(int bush_depth);

	/// @return tone effect
	virtual Tone GetToneEffect() const;

	/// @param tone : tone effect
	virtual void SetToneEffect(Tone tone);

	////////////////////////////////////////////////////////
	/// Flash effect.
	/// @param color : flash color
	////////////////////////////////////////////////////////
	virtual void SetFlashEffect(const Color &color);

	/// @return horizontal flip
	virtual bool GetFlipXEffect() const;

	/// @param flipx : horizontal flip
	virtual void SetFlipXEffect(bool flipx);

	/// @return vertical flip
	virtual bool GetFlipYEffect() const;

	/// @param flipy : vertical flip
	virtual void SetFlipYEffect(bool flipy);

	/// @return horizontal zoom
	virtual double GetZoomXEffect() const;

	/// @param zoom_x : horizontal zoom
	virtual void SetZoomXEffect(double zoom_x);

	/// @return vertical zoom
	virtual double GetZoomYEffect() const;

	/// @param zoom_y : vertical zoom
	virtual void SetZoomYEffect(double zoom_y);

	/// @return angle of rotation in degrees
	virtual double GetAngleEffect() const;

	/// @param angle : angle of rotation in degrees
	virtual void SetAngleEffect(double angle);

	/// @return blend type
	virtual int GetBlendType() const;

	/// @param blend_type : blend type
	virtual void SetBlendType(int blend_type);

	/// @return blend color
	virtual Color GetBlendColor() const;

	/// @param blend_color : blend color
	virtual void SetBlendColor(Color blend_color);

	/// @return waver magnitude in pixels
	virtual int GetWaverEffectDepth() const;

	/// @return waver phase in degrees
	virtual double GetWaverEffectPhase() const;

	/// @param waver magnitude in pixels
	virtual void SetWaverEffectDepth(int depth);

	/// @param waver phase in degrees
	virtual void SetWaverEffectPhase(double phase);

protected:
	BitmapScreen(Bitmap* source, bool delete_bitmap);

	Bitmap* bitmap;

	bool delete_bitmap;

	bool needs_refresh;
	bool bitmap_changed;

	Rect src_rect_effect;
	int opacity_top_effect;
	int opacity_bottom_effect;
	int bush_effect;
	Tone tone_effect;
	bool flipx_effect;
	bool flipy_effect;
	double zoom_x_effect;
	double zoom_y_effect;
	double angle_effect;
	int blend_type_effect;
	Color blend_color_effect;
	int waver_effect_depth;
	double waver_effect_phase;
	Color flash_effect;

	virtual void BlitScreenIntern(Bitmap* draw_bitmap, int x, int y, Rect src_rect,
								  bool need_scale, int bush_y);

	virtual Bitmap* Refresh(Rect& rect, bool& need_scale, int& bush_y);

	Surface* bitmap_effects;
	Bitmap* bitmap_scale;

	Rect bitmap_effects_src_rect;
	Rect bitmap_scale_src_rect;
	bool bitmap_effects_valid;
	bool bitmap_scale_valid;

	Tone current_tone;
	Color current_flash;
	double current_zoom_x;
	double current_zoom_y;
	bool current_flip_x;
	bool current_flip_y;
};

#endif
