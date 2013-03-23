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

// Headers
#include "plane.h"
#include "graphics.h"
#include "player.h"
#include "bitmap.h"
#include "bitmap_screen.h"

Plane::Plane() :
	type(TypePlane),
	ID(Graphics::drawable_id++),
	visible(true),
	z(0),
	ox(0),
	oy(0) {

	bitmap_screen = BitmapScreen::Create();

	zobj = Graphics::RegisterZObj(0, ID);
	Graphics::RegisterDrawable(ID, this);
}

Plane::~Plane() {
	Graphics::RemoveZObj(ID);
	Graphics::RemoveDrawable(ID);
}

void Plane::Draw(int /* z_order */) {
	if (!visible || !bitmap) return;

	Rect dst_rect(0, 0, DisplayUi->GetWidth(), DisplayUi->GetHeight());

	bitmap_screen->BlitScreenTiled(bitmap->GetRect(), dst_rect, ox, oy);
}

BitmapRef const& Plane::GetBitmap() const {
	return bitmap;
}
void Plane::SetBitmap(BitmapRef const& nbitmap) {
	bitmap = nbitmap;
	bitmap_screen->SetBitmap(nbitmap);
}

bool Plane::GetVisible() const {
	return visible;
}
void Plane::SetVisible(bool nvisible) {
	visible = nvisible;
}
int Plane::GetZ() const {
	return z;
}
void Plane::SetZ(int nz) {
	if (z != nz) Graphics::UpdateZObj(zobj, nz);
	z = nz;
}
int Plane::GetOx() const {
	return ox;
}
void Plane::SetOx(int nox) {
	ox = nox;
}
int Plane::GetOy() const {
	return oy;
}
void Plane::SetOy(int noy) {
	oy = noy;
}
double Plane::GetZoomX() const {
	return bitmap_screen->GetZoomXEffect();
}
void Plane::SetZoomX(float zoom_x) {
	bitmap_screen->SetZoomXEffect(zoom_x);
}
double Plane::GetZoomY() const {
	return bitmap_screen->GetZoomYEffect();
}
void Plane::SetZoomY(float zoom_y) {
	bitmap_screen->SetZoomYEffect(zoom_y);
}
int Plane::GetOpacity() const {
	return bitmap_screen->GetOpacityEffect();
}
void Plane::SetOpacity(int opacity) {
	bitmap_screen->SetOpacityEffect(opacity);
}
int Plane::GetBlendType() const {
	return bitmap_screen->GetBlendType();
}
void Plane::SetBlendType(int blend_type) {
	bitmap_screen->SetBlendType(blend_type);
}
Color Plane::GetBlendColor() const {
	return bitmap_screen->GetBlendColor();
}
void Plane::SetBlendColor(Color color) {
	bitmap_screen->SetBlendColor(color);
}
Tone Plane::GetTone() const {
	return bitmap_screen->GetToneEffect();
}
void Plane::SetTone(Tone tone) {
	bitmap_screen->SetToneEffect(tone);
}

unsigned long Plane::GetId() const {
	return ID;
}

DrawableType Plane::GetType() const {
	return type;
}
