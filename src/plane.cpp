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

////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include <math.h>
#include "plane.h"
#include "graphics.h"
#include "rect.h"
#include "player.h"

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
Plane::Plane() {
    bitmap = NULL;
    visible = true;
    z = 0;
    ox = 0;
    oy = 0;
    zoom_x = 1.0;
    zoom_y = 1.0;
    opacity = 255;
    blend_type = 0;
    color = Color();
    tone = Tone();

    plane = NULL;

    _id = Graphics::id++;
    Graphics::drawable_list.push_back(this);
    Graphics::drawable_list.sort(Graphics::SortDrawable);
}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
Plane::~Plane() {
    Graphics::RemoveDrawable(_id);
    delete plane;
}

////////////////////////////////////////////////////////////
/// Draw
////////////////////////////////////////////////////////////
void Plane::Draw() {
    if (!visible) return;
    if (opacity <= 0) return;
    if (zoom_x <= 0 || zoom_y <= 0) return;
    if (!bitmap) return;

    Refresh();

    double bmpw = bitmap->GetWidth() * zoom_x;
    double bmph = bitmap->GetHeight() * zoom_y;
    double tilesx = ceil(Player::GetWidth() / bmpw);
    double tilesy = ceil(Player::GetHeight() / bmph);
    int screen_ox = ox % Player::GetWidth();
    int screen_oy = oy % Player::GetHeight();
    for (double i = 0; i < tilesx; i++) {
        for (double j = 0; j < tilesy; j++) {
            plane->BlitScreen(i * bmpw - screen_ox, j * bmph - screen_oy);
        }
    }
}

////////////////////////////////////////////////////////////
/// Refresh
////////////////////////////////////////////////////////////
void Plane::Refresh() {
    if (!needs_refresh) return;

    needs_refresh = false;

    delete plane;
    
    plane = new Bitmap(bitmap, bitmap->GetRect());
        
    plane->ToneChange(tone);
    plane->OpacityChange(opacity, 0);
    plane->Zoom(zoom_x, zoom_y);
}

////////////////////////////////////////////////////////////
/// Properties
////////////////////////////////////////////////////////////
Bitmap* Plane::GetBitmap() {
    return bitmap;
}
void Plane::SetBitmap(Bitmap* nbitmap) {
    needs_refresh = true;
    bitmap = nbitmap;
}
bool Plane::GetVisible() {
    return visible;
}
void Plane::SetVisible(bool nvisible) {
    visible = nvisible;
}
int Plane::GetZ() {
    return z;
}
void Plane::SetZ(int nz) {
    if (z != nz) Graphics::drawable_list.sort(Graphics::SortDrawable);
    z = nz;
}
int Plane::GetOx() {
    return ox;
}
void Plane::SetOx(int nox) {
    ox = nox;
}
int Plane::GetOy() {
    return oy;
}
void Plane::SetOy(int noy) {
    oy = noy;
}
float Plane::GetZoomX() {
    return zoom_x;
}
void Plane::SetZoomX(float nzoom_x) {
    if (zoom_x != nzoom_x) needs_refresh = true;
    zoom_x = nzoom_x;
}
float Plane::GetZoomY() {
    return zoom_y;
}
void Plane::SetZoomY(float nzoom_y) {
    if (zoom_y != nzoom_y) needs_refresh = true;
    zoom_y = nzoom_y;
}
int Plane::GetOpacity() {
    return opacity;
}
void Plane::SetOpacity(int nopacity) {
    if (opacity != nopacity) needs_refresh = true;
    opacity = nopacity;
}
int Plane::GetBlendType() {
    return blend_type;
}
void Plane::SetBlendType(int nblend_type) {
    blend_type = nblend_type;
}
Color Plane::GetColor() {
    return color;
}
void Plane::SetColor(Color ncolor) {
    color = ncolor;
}
Tone Plane::GetTone() {
    return tone;
}
void Plane::SetTone(Tone ntone) {
    if (tone != ntone) needs_refresh = true;
    tone = ntone;
}

////////////////////////////////////////////////////////////
/// Get id
////////////////////////////////////////////////////////////
unsigned long Plane::GetId() {
    return _id;
}
