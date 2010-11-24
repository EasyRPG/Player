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

#ifndef _SPRITE_H_
#define _SPRITE_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "bitmap.h"
#include "color.h"
#include "tone.h"
#include "rect.h"
#include "drawable.h"

////////////////////////////////////////////////////////////
/// Sprite class
////////////////////////////////////////////////////////////
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

	Bitmap* GetBitmap() const;
	void SetBitmap(Bitmap* nbitmap);
	Rect GetSrcRect() const;
	void SetSrcRect(Rect& nsrc_rect);
	void SetSpriteRect(Rect& nsprite_rect);
	bool GetVisible() const;
	void SetVisible(bool nvisible);
	int GetX() const;
	void SetX(int nx);
	int GetY() const;
	void SetY(int ny);
	int GetZ() const;
	void SetZ(int nz);
	int GetOx() const;
	void SetOx(int nox);
	int GetOy() const;
	void SetOy(int noy);
	double GetZoomX() const;
	void SetZoomX(double nzoom_x);
	double GetZoomY() const;
	void SetZoomY(double nzoom_y);
	double GetAngle() const;
	void SetAngle(double nangle);
	bool GetFlipX() const;
	void SetFlipX(bool nflipx);
	bool GetFlipY() const;
	void SetFlipY(bool nflipy);
	int GetBushDepth() const;
	void SetBushDepth(int nbush_depth);
	int GetOpacity() const;
	void SetOpacity(int nopacity);
	int GetBlendType() const;
	void SetBlendType(int nblend_type);
	Color GetColor() const;
	void SetColor(Color ncolor);
	Tone GetTone() const;
	void SetTone(Tone ntone);

	unsigned long GetId() const;
	DrawableType GetType() const;

private:
	DrawableType type;
	unsigned long ID;
	Bitmap* bitmap;
	Rect src_rect;
	bool visible;
	int x;
	int y;
	int z;
	int ox;
	int oy;
	double zoom_x;
	double zoom_y;
	double angle;
	bool flipx;
	bool flipy;
	int bush_depth;
	int opacity;
	int blend_type;
	Color color;
	Tone tone;
	
	Color flash_color;
	int flash_duration;
	int flash_frame;
	Bitmap* sprite;
	Rect sprite_rect;
	bool needs_refresh;

	void Refresh();
};

#endif
