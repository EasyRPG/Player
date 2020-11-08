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
#include "tilemap.h"
#include "tilemap_layer.h"
#include "bitmap.h"

// Constructor
Tilemap::Tilemap(): layer_down(0), layer_up(1) {
}

// Properties
BitmapRef const& Tilemap::GetChipset() const {
	return layer_down.GetChipset();
}
void Tilemap::SetChipset(BitmapRef const& chipset) {
	layer_down.SetChipset(chipset);
	layer_up.SetChipset(chipset);
}

bool Tilemap::IsVisible() const {
	return layer_down.IsVisible();
}
void Tilemap::SetVisible(bool nvisible) {
	layer_down.SetVisible(nvisible);
	layer_up.SetVisible(nvisible);
}
int Tilemap::GetOx() const {
	return layer_down.GetOx();
}
void Tilemap::SetOx(int nox) {
	layer_down.SetOx(nox);
	layer_up.SetOx(nox);
}
int Tilemap::GetOy() const {
	return layer_down.GetOy();
}
void Tilemap::SetOy(int noy) {
	layer_down.SetOy(noy);
	layer_up.SetOy(noy);
}
int Tilemap::GetWidth() const {
	return layer_down.GetWidth();
}
void Tilemap::SetWidth(int nwidth) {
	layer_down.SetWidth(nwidth);
	layer_up.SetWidth(nwidth);
}
int Tilemap::GetHeight() const {
	return layer_down.GetHeight();
}
void Tilemap::SetHeight(int nheight) {
	layer_down.SetHeight(nheight);
	layer_up.SetHeight(nheight);
}
int Tilemap::GetAnimationSpeed() const {
	return layer_down.GetAnimationSpeed();
}
void Tilemap::SetAnimationSpeed(int speed) {
	layer_down.SetAnimationSpeed(speed);
}
int Tilemap::GetAnimationType() const {
	return layer_down.GetAnimationType();
}
void Tilemap::SetAnimationType(int type) {
	layer_down.SetAnimationType(type);
}
void Tilemap::OnSubstituteDown() {
	layer_down.OnSubstitute();
}
void Tilemap::OnSubstituteUp() {
	layer_up.OnSubstitute();
}

void Tilemap::SetFastBlitDown(bool fast) {
	layer_down.SetFastBlit(fast);
}

void Tilemap::SetTone(Tone tone) {
	layer_down.SetTone(tone);
	layer_up.SetTone(tone);
}
