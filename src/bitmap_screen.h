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
#include "system.h"
#include "tone.h"
#include "rect.h"
#include "color.h"

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
	static BitmapScreenRef Create(BitmapRef const& source);

	////////////////////////////////////////////////////////
	/// Creates a BitmapScreen object with no attached bitmap
	////////////////////////////////////////////////////////
	static BitmapScreenRef Create();

	////////////////////////////////////////////////////////
	/// Destructor.
	////////////////////////////////////////////////////////
	~BitmapScreen();

	////////////////////////////////////////////////////////
	/// Marks the BitmapScreen as dirty.
	////////////////////////////////////////////////////////
	void SetDirty();

	////////////////////////////////////////////////////////
	/// Set source bitmap.
	/// @param source : source bitmap
	/// @param delete_bitmap : if true, the bitmap will be
	///      deleted when it is replaced or when this object
    ///      is deleted.
	////////////////////////////////////////////////////////
	void SetBitmap(BitmapRef const& source);

	////////////////////////////////////////////////////////
	/// Get source bitmap.
	/// @return source bitmap
	////////////////////////////////////////////////////////
	BitmapRef const& GetBitmap();

	////////////////////////////////////////////////////////
	/// Blit the bitmap to the screen.
	/// @param x : x position
	/// @param y : y position
	////////////////////////////////////////////////////////
	void BlitScreen(int x, int y);

	////////////////////////////////////////////////////////
	/// Blit the bitmap to the screen.
	/// @param x : x position
	/// @param y : y position
	/// @param src_rect : source rect
	////////////////////////////////////////////////////////
	void BlitScreen(int x, int y, Rect src_rect);

	////////////////////////////////////////////////////////
	/// Blit the bitmap to the screen.
	/// @param src_rect : source bitmap rect
	/// @param dst_rect : screen destination rect
	////////////////////////////////////////////////////////
	void BlitScreenTiled(Rect src_rect, Rect dst_rect, int ox, int oy);

	////////////////////////////////////////////////////////
	/// Clear all effects data.
	////////////////////////////////////////////////////////
	void ClearEffects();

	/// @return source rect
	Rect GetSrcRect() const;

	/// @param src_rect : source rect
	void SetSrcRect(Rect src_rect);

	/// @return bitmap opacity
	/// @param which : 0 => top, 1 => bottom (below bush depth)
	int GetOpacityEffect(int which = 0) const;

	/// @param opacity_top : bitmap top opacity (above bush depth)
	/// @param opacity_bottom : bitmap bottom opacity (below bush depth)
	///  note: opacity_bottom == -1 => opacity_bottom = (opacity_top + 1) / 2
	void SetOpacityEffect(int opacity_top, int opacity_bottom = -1);

	/// @return bush depth effect
	int GetBushDepthEffect() const;

	/// @param bush_depth : bush depth effect
	void SetBushDepthEffect(int bush_depth);

	/// @return tone effect
	Tone GetToneEffect() const;

	/// @param tone : tone effect
	void SetToneEffect(Tone tone);

	////////////////////////////////////////////////////////
	/// Flash effect.
	/// @param color : flash color
	////////////////////////////////////////////////////////
	void SetFlashEffect(const Color &color);

	/// @return horizontal flip
	bool GetFlipXEffect() const;

	/// @param flipx : horizontal flip
	void SetFlipXEffect(bool flipx);

	/// @return vertical flip
	bool GetFlipYEffect() const;

	/// @param flipy : vertical flip
	void SetFlipYEffect(bool flipy);

	/// @return horizontal zoom
	double GetZoomXEffect() const;

	/// @param zoom_x : horizontal zoom
	void SetZoomXEffect(double zoom_x);

	/// @return vertical zoom
	double GetZoomYEffect() const;

	/// @param zoom_y : vertical zoom
	void SetZoomYEffect(double zoom_y);

	/// @return angle of rotation in degrees
	double GetAngleEffect() const;

	/// @param angle : angle of rotation in degrees
	void SetAngleEffect(double angle);

	/// @return blend type
	int GetBlendType() const;

	/// @param blend_type : blend type
	void SetBlendType(int blend_type);

	/// @return blend color
	Color GetBlendColor() const;

	/// @param blend_color : blend color
	void SetBlendColor(Color blend_color);

	/// @return waver magnitude in pixels
	int GetWaverEffectDepth() const;

	/// @return waver phase in degrees
	double GetWaverEffectPhase() const;

	/// @param waver magnitude in pixels
	void SetWaverEffectDepth(int depth);

	/// @param waver phase in degrees
	void SetWaverEffectPhase(double phase);

	BitmapScreen(BitmapRef const& source);

protected:
	BitmapRef bitmap;

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

	void BlitScreenIntern(Bitmap const& draw_bitmap, int x, int y, Rect src_rect,
								  bool need_scale, int bush_y);

	BitmapRef Refresh(Rect& rect, bool& need_scale, int& bush_y);

	BitmapRef bitmap_effects;
	BitmapRef bitmap_scale;

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
