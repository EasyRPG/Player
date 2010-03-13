//////////////////////////////////////////////////////////////////////////////////
/// This file is part of EasyRPG Player.
/// 
/// EasyRPG Player is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
/// 
/// EasyRPG Player is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
/// 
/// You should have received a copy of the GNU General Public License
/// along with EasyRPG Player.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////////////////

#ifndef _SPRITE_H_
#define _SPRITE_H_

////////////////////////////////////////////////////////////
/// Headers
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
    ~Sprite();
    
    void Draw();

    void Flash(int duration);
    void Flash(Color color, int duration);
    void Update();

    int GetWidth();
    int GetHeight();

    Bitmap* GetBitmap();
    void SetBitmap(Bitmap* nbitmap);
    Rect GetSrcRect();
    void SetSrcRect(Rect nsrc_rect);
    bool GetVisible();
    void SetVisible(bool nvisible);
    int GetX();
    void SetX(int nx);
    int GetY();
    void SetY(int ny);
    int GetZ();
    void SetZ(int nz);
    int GetOx();
    void SetOx(int nox);
    int GetOy();
    void SetOy(int noy);
    double GetZoomX();
    void SetZoomX(double nzoom_x);
    double GetZoomY();
    void SetZoomY(double nzoom_y);
    double GetAngle();
    void SetAngle(double nangle);
    bool GetFlipX();
    void SetFlipX(bool nflipx);
    bool GetFlipY();
    void SetFlipY(bool nflipy);
    int GetBushDepth();
    void SetBushDepth(int nbush_depth);
    int GetOpacity();
    void SetOpacity(int nopacity);
    int GetBlendType();
    void SetBlendType(int nblend_type);
    Color GetColor();
    void SetColor(Color ncolor);
    Tone GetTone();
    void SetTone(Tone ntone);

    unsigned long GetId();

private:
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
    bool needs_refresh;

    void Refresh();
};

#endif
