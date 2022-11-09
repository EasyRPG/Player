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
#include "memory_management.h"
#include "rect.h"
#include "tone.h"

/**
 * Sprite class.
 */
class Sprite : public Drawable {
public:
	explicit Sprite(Drawable::Flags flags = Drawable::Flags::Default);

	void Draw(Bitmap& dst) override;

	virtual int GetWidth() const;
	virtual int GetHeight() const;

	BitmapRef& GetBitmap();
	BitmapRef const& GetBitmap() const;
	void SetBitmap(BitmapRef const& bitmap);
	Rect const& GetSrcRect() const;
	void SetSrcRect(Rect const& src_rect);
	void SetSpriteRect(Rect const& sprite_rect);
	int GetX() const;
	void SetX(int x);
	int GetY() const;
	void SetY(int y);
	int GetOx() const;
	void SetOx(int ox);
	int GetOy() const;
	void SetOy(int oy);
	double GetZoomX() const;
	void SetZoomX(double zoom_x);
	double GetZoomY() const;
	void SetZoomY(double zoom_y);

	/** @return rotation angle in radians */
	double GetAngle() const;
	/**
	 * Set rotation angle.
	 *
	 * @param angle the angle in radians
	 */
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

	/** @return wave depth in pixels */
	int GetWaverDepth() const;

	/** Set wave effect depth
	 *
	 * @param depth in pixels
	 */
	void SetWaverDepth(int depth);

	/** @return waver phase in radians */
	double GetWaverPhase() const;
	/**
	 * Set wave effect phase.
	 *
	 * @param angle the phase in radians
	 */
	void SetWaverPhase(double phase);

	/**
	 * Set the flash effect color
	 */
	void SetFlashEffect(const Color &color);

private:
	BitmapRef bitmap;

	Rect src_rect;
	int x = 0;
	int y = 0;
	int ox = 0;
	int oy = 0;

	Rect src_rect_effect;
	int opacity_top_effect = 255;
	int opacity_bottom_effect = 128;
	int bush_effect = 0;
	Tone tone_effect;
	double zoom_x_effect = 1.0;
	double zoom_y_effect = 1.0;
	double angle_effect = 0.0;
	int blend_type_effect = 0;
	Color blend_color_effect;
	int waver_effect_depth = 0;
	double waver_effect_phase = 0.0;
	Color flash_effect;

	BitmapRef bitmap_effects;

	Rect bitmap_effects_src_rect;

	Tone current_tone;
	Color current_flash;
	bool flipx_effect = false;
	bool flipy_effect = false;
	bool current_flip_x = false;
	bool current_flip_y = false;
	bool bitmap_changed = true;

	void BlitScreen(Bitmap& dst);
	void BlitScreenIntern(Bitmap& dst, Bitmap const& draw_bitmap,
							Rect const& src_rect) const;
	BitmapRef Refresh(Rect& rect);
};

inline int Sprite::GetWidth() const {
	return src_rect.width;
}

inline int Sprite::GetHeight() const {
	return src_rect.height;
}

inline BitmapRef& Sprite::GetBitmap() {
	return bitmap;
}

inline BitmapRef const& Sprite::GetBitmap() const {
	return bitmap;
}

inline Rect const& Sprite::GetSrcRect() const {
	return src_rect;
}

inline void Sprite::SetSrcRect(Rect const& nsrc_rect) {
	src_rect = nsrc_rect;
}

inline int Sprite::GetX() const {
	return x;
}

inline void Sprite::SetX(int nx) {
	x = nx;
}

inline int Sprite::GetY() const {
	return y;
}

inline void Sprite::SetY(int ny) {
	y = ny;
}

inline int Sprite::GetOx() const {
	return ox;
}

inline void Sprite::SetOx(int nox) {
	ox = nox;
}

inline int Sprite::GetOy() const {
	return oy;
}

inline void Sprite::SetOy(int noy) {
	oy = noy;
}

inline double Sprite::GetZoomX() const {
	return zoom_x_effect;
}

inline double Sprite::GetZoomY() const {
	return zoom_y_effect;
}

inline void Sprite::SetZoomX(double zoom_x) {
	zoom_x_effect = zoom_x;
}

inline void Sprite::SetZoomY(double zoom_y) {
	zoom_y_effect = zoom_y;
}

inline double Sprite::GetAngle() const {
	return angle_effect;
}

inline void Sprite::SetAngle(double angle) {
	angle_effect = angle;
}

inline bool Sprite::GetFlipX() const {
	return flipx_effect;
}

inline bool Sprite::GetFlipY() const {
	return flipy_effect;
}

inline int Sprite::GetBushDepth() const {
	return bush_effect;
}

inline int Sprite::GetOpacity(int which) const {
	return which > 0 ? opacity_bottom_effect : opacity_top_effect;
}

inline int Sprite::GetBlendType() const {
	return blend_type_effect;
}

inline void Sprite::SetBlendType(int blend_type) {
	blend_type_effect = blend_type;
}

inline Color Sprite::GetBlendColor() const {
	return blend_color_effect;
}

inline void Sprite::SetBlendColor(Color blend_color) {
	blend_color_effect = blend_color;
}

inline Tone Sprite::GetTone() const {
	return tone_effect;
}

inline void Sprite::SetTone(Tone tone) {
	tone_effect = tone;
}

inline int Sprite::GetWaverDepth() const {
	return waver_effect_depth;
}

inline double Sprite::GetWaverPhase() const {
	return waver_effect_phase;
}

inline void Sprite::SetWaverDepth(int depth) {
	waver_effect_depth = depth;
}

inline void Sprite::SetWaverPhase(double phase) {
	waver_effect_phase = phase;
}

inline void Sprite::SetSpriteRect(Rect const& nsprite_rect) {
	src_rect_effect = nsprite_rect;
}

inline void Sprite::SetFlipX(bool flipx) {
	flipx_effect = flipx;
}

inline void Sprite::SetFlipY(bool flipy) {
	flipy_effect = flipy;
}

inline void Sprite::SetBushDepth(int bush_depth) {
	bush_effect = bush_depth;
}

inline void Sprite::SetFlashEffect(const Color &color) {
	flash_effect = color;
}

#endif
