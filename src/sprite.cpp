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
#include <string>
#include "sprite.h"
#include "player.h"
#include "graphics.h"
#include "util_macro.h"
#include "bitmap.h"
#include "bitmap_screen.h"

// Constructor
Sprite::Sprite() :
	type(TypeSprite),
	visible(true),
	x(0),
	y(0),
	z(0),
	ox(0),
	oy(0),
	flash_duration(0),
	flash_frame(0) {

	bitmap_screen = BitmapScreen::Create();

	Graphics::RegisterDrawable(this);
}

// Destructor
Sprite::~Sprite() {
	Graphics::RemoveDrawable(this);
}

// Draw
void Sprite::Draw() {
	if (!visible) return;
	if (GetWidth() <= 0 || GetHeight() <= 0) return;

	bitmap_screen->BlitScreen(x - ox, y - oy, src_rect);
}

int Sprite::GetWidth() const {
	return src_rect.width;
}

int Sprite::GetHeight() const {
	return src_rect.height;
}

void Sprite::Update() {
	if (flash_duration != 0) {
		flash_frame += 1;
		if (flash_duration == flash_frame) {
			flash_duration = 0;
			bitmap_screen->SetFlashEffect(Color());
		} else {
			Color flash_effect = flash_color;
			flash_effect.alpha = flash_duration == 0 || flash_frame >= flash_duration
				? 0
				: flash_effect.alpha * (flash_duration - flash_frame) / flash_duration;
			bitmap_screen->SetFlashEffect(flash_effect);
		}
	}
}

void Sprite::Flash(int duration){
	bitmap_screen->SetFlashEffect(flash_color);
	flash_duration = duration;
	flash_frame = 0;
}
void Sprite::Flash(Color color, int duration){
	flash_color = color;
	flash_duration = duration;
	flash_frame = 0;
	bitmap_screen->SetFlashEffect(color);
}

BitmapRef const& Sprite::GetBitmap() const {
	return bitmap;
}

void Sprite::SetBitmap(BitmapRef const& nbitmap) {
	bitmap = nbitmap;
	if (!bitmap) {
		src_rect = Rect();
	} else {
		src_rect = bitmap->GetRect();
	}
	bitmap_screen->SetBitmap(bitmap);
	bitmap_screen->SetSrcRect(src_rect);
}
Rect const& Sprite::GetSrcRect() const {
	return src_rect;
}

void Sprite::SetSrcRect(Rect const& nsrc_rect) {
	src_rect = nsrc_rect;
}
void Sprite::SetSpriteRect(Rect const& nsprite_rect) {
	bitmap_screen->SetSrcRect(nsprite_rect);
}

bool Sprite::GetVisible() const {
	return visible;
}
void Sprite::SetVisible(bool nvisible) {
	visible = nvisible;
}

int Sprite::GetX() const {
	return x;
}
void Sprite::SetX(int nx) {
	x = nx;
}

int Sprite::GetY() const {
	return y;
}
void Sprite::SetY(int ny) {
	y = ny;
}

int Sprite::GetZ() const {
	return z;
}
void Sprite::SetZ(int nz) {
	if (z != nz) Graphics::UpdateZCallback();
	z = nz;
}

int Sprite::GetOx() const {
	return ox;
}
void Sprite::SetOx(int nox) {
	ox = nox;
}

int Sprite::GetOy() const {
	return oy;
}
void Sprite::SetOy(int noy) {
	oy = noy;
}

double Sprite::GetZoomX() const {
	return bitmap_screen->GetZoomXEffect();
}
void Sprite::SetZoomX(double zoom_x) {
	bitmap_screen->SetZoomXEffect(zoom_x);
}

double Sprite::GetZoomY() const {
	return bitmap_screen->GetZoomYEffect();
}
void Sprite::SetZoomY(double zoom_y) {
	bitmap_screen->SetZoomYEffect(zoom_y);
}

double Sprite::GetAngle() const {
	return bitmap_screen->GetAngleEffect();
}
void Sprite::SetAngle(double angle) {
	bitmap_screen->SetAngleEffect(angle);
}

bool Sprite::GetFlipX() const {
	return bitmap_screen->GetFlipXEffect();
}
void Sprite::SetFlipX(bool flipx) {
	bitmap_screen->SetFlipXEffect(flipx);
}

bool Sprite::GetFlipY() const {
	return bitmap_screen->GetFlipYEffect();
}
void Sprite::SetFlipY(bool flipy) {
	bitmap_screen->SetFlipYEffect(flipy);
}

int Sprite::GetBushDepth() const {
	return bitmap_screen->GetBushDepthEffect();
}
void Sprite::SetBushDepth(int bush_depth) {
	bitmap_screen->SetBushDepthEffect(bush_depth);
}

int Sprite::GetOpacity() const {
	return bitmap_screen->GetOpacityEffect();
}
void Sprite::SetOpacity(int top_opacity, int bottom_opacity) {
	bitmap_screen->SetOpacityEffect(top_opacity, bottom_opacity);
}

int Sprite::GetBlendType() const {
	return bitmap_screen->GetBlendType();
}
void Sprite::SetBlendType(int blend_type) {
	bitmap_screen->SetBlendType(blend_type);
}

Color Sprite::GetBlendColor() const {
	return bitmap_screen->GetBlendColor();
}
void Sprite::SetBlendColor(Color color) {
	bitmap_screen->SetBlendColor(color);
}

Tone Sprite::GetTone() const {
	return bitmap_screen->GetToneEffect();
}
void Sprite::SetTone(Tone tone) {
	bitmap_screen->SetToneEffect(tone);
}

int Sprite::GetWaverDepth() const {
	return bitmap_screen->GetWaverEffectDepth();
}
void Sprite::SetWaverDepth(int depth) {
	bitmap_screen->SetWaverEffectDepth(depth);
}

double Sprite::GetWaverPhase() const {
	return bitmap_screen->GetWaverEffectPhase();
}
void Sprite::SetWaverPhase(double phase) {
	bitmap_screen->SetWaverEffectPhase(phase);
}

DrawableType Sprite::GetType() const {
	return type;
}
