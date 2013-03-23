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

#ifndef _SPRITE_H_
#define _SPRITE_H_

// Headers
#include "color.h"
#include "drawable.h"
#include "rect.h"
#include "tone.h"
#include "zobj.h"

/**
 * Sprite class.
 */
class Sprite : public Drawable {
public:
	Sprite();
	virtual ~Sprite();

	void Draw(int z_order);

	void Flash(int duration);
	void Flash(Color color, int duration);
	void Update();

	int GetWidth() const;
	int GetHeight() const;

	BitmapRef const& GetBitmap() const;
	void SetBitmap(BitmapRef const& bitmap);
	Rect const& GetSrcRect() const;
	void SetSrcRect(Rect const& src_rect);
	void SetSpriteRect(Rect const& sprite_rect);
	bool GetVisible() const;
	void SetVisible(bool visible);
	int GetX() const;
	void SetX(int x);
	int GetY() const;
	void SetY(int y);
	int GetZ() const;
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
	int GetOpacity() const;
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

	unsigned long GetId() const;
	DrawableType GetType() const;

private:
	DrawableType type;
	unsigned long ID;
	ZObj* zobj;

	BitmapRef bitmap;
	BitmapScreenRef bitmap_screen;

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
};

#endif
