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

#ifndef _SDL_BITMAP_SCREEN_H_
#define _SDL_BITMAP_SCREEN_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "bitmap_screen.h"
#include "SDL.h"

////////////////////////////////////////////////////////////
/// SdlBitmapScreen class.
////////////////////////////////////////////////////////////
class SdlBitmapScreen : public BitmapScreen {
public:
	SdlBitmapScreen(Bitmap* source);
	SdlBitmapScreen(bool delete_bitmap);
	~SdlBitmapScreen();

	void SetDirty();

	void SetBitmap(Bitmap* bitmap);
	Bitmap* GetBitmap();

	void BlitScreen(int x, int y);
	void BlitScreen(int x, int y, Rect src_rect);
	void BlitScreenTiled(Rect src_rect, Rect dst_rect);

	void ClearEffects();
	void SetFlashEffect(const Color &color, int duration);
	void UpdateFlashEffect(int frame);

	Rect GetSrcRect() const;
	void SetSrcRect(Rect src_rect);
	int GetOpacityEffect() const;
	void SetOpacityEffect(int opacity);
	int GetBushDepthEffect() const;
	void SetBushDepthEffect(int bush_depth);
	Tone GetToneEffect() const;
	void SetToneEffect(Tone tone);
	bool GetFlipXEffect() const;
	void SetFlipXEffect(bool flipx);
	bool GetFlipYEffect() const;
	void SetFlipYEffect(bool flipy);
	double GetZoomXEffect() const;
	void SetZoomXEffect(double zoom_x);
	double GetZoomYEffect() const;
	void SetZoomYEffect(double zoom_y);
	double GetAngleEffect() const;
	void SetAngleEffect(double angle);
	int GetBlendType() const;
	void SetBlendType(int blend_type);
	Color GetBlendColor() const;
	void SetBlendColor(Color blend_color);

protected:
	static void BlitScreenIntern(SDL_Surface* surface, int x, int y, Rect src_rect, int opacity);

	void Refresh();

	void CalcRotatedSize(int &width, int &height);
	void CalcZoomedSize(int &width, int &height);

	bool delete_bitmap;

	Bitmap* bitmap;
	Bitmap* bitmap_effects;

	bool needs_refresh;

	Rect src_rect_effect;
	bool src_rect_effect_applied;
	int opacity_effect;
	int bush_effect;
	Tone tone_effect;
	bool flipx_effect;
	bool flipy_effect;
	double zoom_x_effect;
	double zoom_y_effect;
	double angle_effect;
	int blend_type_effect;
	Color blend_color_effect;
};

#endif
