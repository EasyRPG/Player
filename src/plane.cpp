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

Plane::Plane() :
	type(TypePlane),
	visible(true),
	z(0),
	ox(0),
	oy(0) {

	Graphics::RegisterDrawable(this);
}

Plane::~Plane() {
	Graphics::RemoveDrawable(this);
}

void Plane::Draw() {
	if (!visible || !bitmap) return;

	BitmapRef dst = DisplayUi->GetDisplaySurface();
	Rect dst_rect(0, 0, DisplayUi->GetWidth(), DisplayUi->GetHeight());

	dst->TiledBlit(-ox, -oy, bitmap->GetRect(), *bitmap, dst_rect, 255);
}

BitmapRef const& Plane::GetBitmap() const {
	return bitmap;
}
void Plane::SetBitmap(BitmapRef const& nbitmap) {
	bitmap = nbitmap;
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
	if (z != nz) Graphics::UpdateZCallback();
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

DrawableType Plane::GetType() const {
	return type;
}
