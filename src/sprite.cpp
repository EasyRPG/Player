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
	flash_frame(0),

	needs_refresh(true),
	bitmap_changed(true),
	src_rect_effect(Rect()),

	opacity_top_effect(255),
	opacity_bottom_effect(128),
	bush_effect(0),
	tone_effect(Tone()),
	flipx_effect(false),
	flipy_effect(false),
	zoom_x_effect(1.0),
	zoom_y_effect(1.0),
	angle_effect(0.0),
	waver_effect_depth(0),
	waver_effect_phase(0.0),
	flash_effect(Color(0,0,0,0)),
	bitmap_effects_src_rect(Rect()),
	bitmap_effects_valid(false),

	current_tone(Tone()),
	current_flash(Color(0,0,0,0)),
	current_flip_x(false),
	current_flip_y(false) {

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

	BlitScreen();
}

void Sprite::BlitScreen() {
	if (!bitmap || (opacity_top_effect <= 0 && opacity_bottom_effect <= 0))
		return;

	Rect rect = src_rect_effect.GetSubRect(src_rect);

	BitmapRef draw_bitmap = Refresh(rect);

	bitmap_changed = false;
	needs_refresh = false;

	if (draw_bitmap) {
		BlitScreenIntern(*draw_bitmap, rect, bush_effect);
	}
}

void Sprite::BlitScreenIntern(Bitmap const& draw_bitmap,
								Rect const& src_rect, int opacity_split) const {
	BitmapRef dst = DisplayUi->GetDisplaySurface();

	double zoom_x = zoom_x_effect;
	double zoom_y = zoom_y_effect;

	dst->EffectsBlit(x, y, ox, oy, draw_bitmap, src_rect,
					 Opacity(opacity_top_effect, opacity_bottom_effect, opacity_split),
					 zoom_x, zoom_y, angle_effect != 0.0 ? angle_effect * 3.14159 / 180 : 0.0,
					 waver_effect_depth, waver_effect_phase);
}

BitmapRef Sprite::Refresh(Rect& rect) {
	if (zoom_x_effect != 1.0 && zoom_y_effect != 1.0 && angle_effect != 0.0 && waver_effect_depth != 0) {
		// TODO: Out of bounds check adjustments for zoom, angle and waver
		// but even without this will catch most of the cases
		if (Rect(x - ox, y - oy, GetWidth(), GetHeight()).IsOutOfBounds(Rect(0, 0, SCREEN_TARGET_WIDTH, SCREEN_TARGET_HEIGHT))) {
			return BitmapRef();
		};
	}

	rect.Adjust(bitmap->GetWidth(), bitmap->GetHeight());

	bool no_tone = tone_effect == Tone();
	bool no_flash = flash_effect.alpha == 0;
	bool no_flip = !flipx_effect && !flipy_effect;
	bool no_effects = no_tone && no_flash && no_flip;
	bool effects_changed = tone_effect != current_tone ||
		flash_effect != current_flash ||
		flipx_effect != current_flip_x ||
		flipy_effect != current_flip_y;
	bool effects_rect_changed = rect != bitmap_effects_src_rect;

	if (effects_changed || effects_rect_changed || bitmap_changed) {
		bitmap_effects_valid = false;
	}

	if (no_effects)
		return bitmap;

	if (bitmap_effects && bitmap_effects_valid)
		return bitmap_effects;

	BitmapRef src_bitmap;

	if (no_effects)
		src_bitmap = bitmap;
	else if (bitmap_effects_valid)
		src_bitmap = bitmap_effects;
	else {
		current_tone = tone_effect;
		current_flash = flash_effect;
		current_flip_x = flipx_effect;
		current_flip_y = flipy_effect;

		if (bitmap_effects &&
			bitmap_effects->GetWidth() < rect.x + rect.width &&
			bitmap_effects->GetHeight() < rect.y + rect.height) {
		bitmap_effects.reset();
		}

		if (!bitmap_effects)
			bitmap_effects = Bitmap::Create(bitmap->GetWidth(), bitmap->GetHeight(), true);

		bitmap_effects->Clear();
		if (no_tone && no_flash)
			bitmap_effects->FlipBlit(rect.x, rect.y, *bitmap, rect, flipx_effect, flipy_effect, Opacity::opaque);
		else if (no_flip && no_flash)
			bitmap_effects->ToneBlit(rect.x, rect.y, *bitmap, rect, tone_effect, Opacity::opaque);
		else if (no_flip && no_tone)
			bitmap_effects->BlendBlit(rect.x, rect.y, *bitmap, rect, flash_effect, Opacity::opaque);
		else if (no_flash) {
			bitmap_effects->ToneBlit(rect.x, rect.y, *bitmap, rect, tone_effect, Opacity::opaque);
			bitmap_effects->Flip(rect, flipx_effect, flipy_effect);
		}
		else if (no_tone) {
			bitmap_effects->BlendBlit(rect.x, rect.y, *bitmap, rect, flash_effect, Opacity::opaque);
			bitmap_effects->Flip(rect, flipx_effect, flipy_effect);
		}
		else if (no_flip) {
			bitmap_effects->BlendBlit(rect.x, rect.y, *bitmap, rect, flash_effect, Opacity::opaque);
			bitmap_effects->ToneBlit(rect.x, rect.y, *bitmap_effects, rect, tone_effect, Opacity::opaque);
		}
		else {
			bitmap_effects->BlendBlit(rect.x, rect.y, *bitmap, rect, flash_effect, Opacity::opaque);
			bitmap_effects->ToneBlit(rect.x, rect.y, *bitmap_effects, rect, tone_effect, Opacity::opaque);
			bitmap_effects->Flip(rect, flipx_effect, flipy_effect);
		}

		bitmap_effects_src_rect = rect;
		bitmap_effects_valid = true;

		src_bitmap = bitmap_effects;
	}

	return src_bitmap;
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
			SetFlashEffect(Color());
		} else {
			Color flash_effect = flash_color;
			flash_effect.alpha = flash_duration == 0 || flash_frame >= flash_duration
				? 0
				: flash_effect.alpha * (flash_duration - flash_frame) / flash_duration;
			SetFlashEffect(flash_effect);
		}
	}
}

void Sprite::Flash(int duration){
	SetFlashEffect(flash_color);
	flash_duration = duration;
	flash_frame = 0;
}
void Sprite::Flash(Color color, int duration){
	flash_color = color;
	flash_duration = duration;
	flash_frame = 0;
	SetFlashEffect(color);
}

void Sprite::SetFlashEffect(const Color &color) {
	if (flash_effect != color) {
		flash_effect = color;
		needs_refresh = true;
	}
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

	src_rect_effect = src_rect;

	needs_refresh = true;
	bitmap_changed = true;
}

Rect const& Sprite::GetSrcRect() const {
	return src_rect;
}

void Sprite::SetSrcRect(Rect const& nsrc_rect) {
	src_rect = nsrc_rect;
}
void Sprite::SetSpriteRect(Rect const& nsprite_rect) {
	if (src_rect_effect != nsprite_rect) {
		src_rect_effect = nsprite_rect;
		needs_refresh = true;
	}
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
	return zoom_x_effect;
}
void Sprite::SetZoomX(double zoom_x) {
	zoom_x_effect = zoom_x;
}

double Sprite::GetZoomY() const {
	return zoom_y_effect;
}
void Sprite::SetZoomY(double zoom_y) {
	zoom_y_effect = zoom_y;
}

double Sprite::GetAngle() const {
	return angle_effect;
}

void Sprite::SetAngle(double angle) {
	angle_effect = angle;
}

bool Sprite::GetFlipX() const {
	return flipx_effect;
}

void Sprite::SetFlipX(bool flipx) {
	if (flipx_effect != flipx) {
		flipx_effect = flipx;
		needs_refresh = true;
	}
}

bool Sprite::GetFlipY() const {
	return flipy_effect;
}

void Sprite::SetFlipY(bool flipy) {
	if (flipy_effect != flipy) {
		flipy_effect = flipy;
		needs_refresh = true;
	}
}

int Sprite::GetBushDepth() const {
	return bush_effect;
}

void Sprite::SetBushDepth(int bush_depth) {
	if (bush_effect != bush_depth) {
		bush_effect = bush_depth;
		needs_refresh = true;
	}
}

int Sprite::GetOpacity(int which) const {
	return which > 0 ? opacity_bottom_effect : opacity_top_effect;
}

void Sprite::SetOpacity(int opacity_top, int opacity_bottom) {
	if (opacity_top_effect != opacity_top) {
		opacity_top_effect = opacity_top;
		needs_refresh = true;
	}
	if (opacity_bottom == -1)
		opacity_bottom = (opacity_top + 1) / 2;
	if (opacity_bottom_effect != opacity_bottom) {
		opacity_bottom_effect = opacity_bottom;
		needs_refresh = true;
	}
}

int Sprite::GetBlendType() const {
	return blend_type_effect;
}

void Sprite::SetBlendType(int blend_type) {
	blend_type_effect = blend_type;
}

Color Sprite::GetBlendColor() const {
	return blend_color_effect;
}

void Sprite::SetBlendColor(Color blend_color) {
	blend_color_effect = blend_color;
}

Tone Sprite::GetTone() const {
	return tone_effect;
}

void Sprite::SetTone(Tone tone) {
	if (tone_effect != tone) {
		tone_effect = tone;
		needs_refresh = true;
	}
}

int Sprite::GetWaverDepth() const {
	return waver_effect_depth;
}

void Sprite::SetWaverDepth(int depth) {
	if (waver_effect_depth != depth) {
		waver_effect_depth = depth;
		needs_refresh = true;
	}
}

double Sprite::GetWaverPhase() const {
	return waver_effect_phase;
}

void Sprite::SetWaverPhase(double phase) {
	if (waver_effect_phase != phase) {
		waver_effect_phase = phase;
		needs_refresh = true;
	}
}

DrawableType Sprite::GetType() const {
	return type;
}
