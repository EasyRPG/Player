/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EP_SPRITE_H
#define EP_SPRITE_H

// Headers
#include "color.h"
#include "drawable.h"
#include "rect.h"
#include "tone.h"

/**
 * Sprite class.
 */
class Sprite : public Drawable {
public:
	Sprite();
	~Sprite() override;

	void Draw() override;

	virtual void Flash(int duration);
	virtual void Flash(Color color, int duration);
	void Update();

	virtual int GetWidth() const;
	virtual int GetHeight() const;

	BitmapRef const& GetBitmap() const;
	void SetBitmap(BitmapRef const& bitmap);
	Rect const& GetSrcRect() const;
	void SetSrcRect(Rect const& src_rect);
	void SetSpriteRect(Rect const& sprite_rect);
	virtual bool GetVisible() const;
	virtual void SetVisible(bool visible);
	int GetX() const;
	void SetX(int x);
	int GetY() const;
	void SetY(int y);
	int GetZ() const override;
	void SetZ(int z);
	int GetOx() const;
	void SetOx(int ox);
	int GetOy() const;
	void SetOy(int oy);
	double GetZoomX() const;
	void SetZoomX(double zoom_x);
	double GetZoomY() const;
	void SetZoomY(double zoom_y);
	double GetAngle() const;
	void SetAngle(double angle);
	bool GetFlipX() const;
	void SetFlipX(bool flipx);
	bool GetFlipY() const;
	void SetFlipY(bool flipy);
	int GetBushDepth() const;
	void SetBushDepth(int bush_depth);
	int GetOpacity(int which = 0) const;
	void SetOpacity(int top_opacity, int bottom_opacity = -1);
	int GetBlendType() const;
	void SetBlendType(int blend_type);
	Color GetBlendColor() const;
	void SetBlendColor(Color color);
	Tone GetTone() const;
	void SetTone(Tone tone);
	int GetWaverDepth() const;
	void SetWaverDepth(int depth);
	double GetWaverPhase() const;
	void SetWaverPhase(double phase);

	DrawableType GetType() const override;

private:
	DrawableType type;

	BitmapRef bitmap;

	Rect src_rect;
	bool visible;
	int x;
	int y;
	int z;
	int ox;
	int oy;

	Color flash_color;
	int flash_duration;
	int flash_frame;

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

	BitmapRef bitmap_effects;

	Rect bitmap_effects_src_rect;
	bool bitmap_effects_valid;

	Tone current_tone;
	Color current_flash;
	bool current_flip_x;
	bool current_flip_y;

	void BlitScreen();
	void BlitScreenIntern(Bitmap const& draw_bitmap,
							Rect const& src_rect, int opacity_split) const;
	BitmapRef Refresh(Rect& rect);
	void SetFlashEffect(const Color &color);
};

#endif
