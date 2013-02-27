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
#include <cmath>

#include "system.h"
#include "bitmap_screen.h"
#include "baseui.h"
#include "util_macro.h"
#include "bitmap.h"

BitmapScreenRef BitmapScreen::Create(BitmapRef const& source) {
	return EASYRPG_MAKE_SHARED<BitmapScreen>(source);
}

BitmapScreenRef BitmapScreen::Create() {
	return Create(BitmapRef());
}

BitmapScreen::BitmapScreen(BitmapRef const& bitmap) :
	bitmap(bitmap),
	bitmap_effects_valid(false),
	bitmap_scale_valid(false) {

	ClearEffects();
	bitmap_changed = true;

	current_tone = tone_effect;
	current_zoom_x = zoom_x_effect;
	current_zoom_y = zoom_y_effect;
	current_flip_x = flipx_effect;
	current_flip_y = flipy_effect;
	current_flash = flash_effect;

	bitmap_effects_src_rect = Rect();
	bitmap_scale_src_rect = Rect();

	if (bitmap != NULL) {
		src_rect_effect = bitmap->GetRect();
		bitmap->AttachBitmapScreen(this);
	}
}

BitmapScreen::~BitmapScreen() {
	if (bitmap) {
		bitmap->DetachBitmapScreen(this);
	}
}

void BitmapScreen::BlitScreen(int x, int y) {
	BlitScreen(x, y, Rect(0, 0, src_rect_effect.width, src_rect_effect.height));
}

void BitmapScreen::BlitScreen(int x, int y, Rect const& src_rect) {
	if (bitmap == NULL || (opacity_top_effect <= 0 && opacity_bottom_effect <= 0))
		return;

	Rect rect = src_rect_effect.GetSubRect(src_rect);

	Rect bush_rect = src_rect_effect;
	bush_rect.height -= bush_effect;
	bush_rect = bush_rect.GetSubRect(src_rect);
	int bush_y = (rect.y + rect.height) - (bush_rect.y + bush_rect.height);

	bool need_scale = false;
	BitmapRef draw_bitmap = Refresh(rect, need_scale, bush_y);

	bitmap_changed = false;
	needs_refresh = false;

	if(draw_bitmap) {
		BlitScreenIntern(*draw_bitmap, x, y, rect, need_scale, bush_y);
	}
}

void BitmapScreen::BlitScreenTiled(Rect const& src_rect, Rect const& dst_rect, int ox, int oy) {
	if (bitmap == NULL || (opacity_top_effect <= 0 && opacity_bottom_effect <= 0))
		return;

	Rect rect = src_rect_effect.GetSubRect(src_rect);
	int bush_y = 0;

	bool need_scale = false;
	BitmapRef draw_bitmap = Refresh(rect, need_scale, bush_y);

	bitmap_changed = false;
	needs_refresh = false;

	int width = rect.width;
	int height = rect.height;

	if (need_scale) {
		width  = (int)(width  * zoom_x_effect);
		height = (int)(height * zoom_y_effect);
	}

	if (ox > 0)
		ox -= width * ((ox + width - 1) / width);
	else if (ox < 0)
		ox += width * (ox / width);

	if (oy > 0)
		oy -= height * ((oy + height - 1) / height);
	else if (oy < 0)
		oy += height * (oy / height);

	int x0 = dst_rect.x + ox;
	int y0 = dst_rect.y + oy;
	int x1 = dst_rect.x + dst_rect.width;
	int y1 = dst_rect.y + dst_rect.height;
	for (int y = y0; y < y1; y += height) {
		for (int x = x0; x < x1; x += width) {
			Rect blit_rect = rect;
			if (y + blit_rect.height > y1)
				blit_rect.height = y1 - y;
			if (x + blit_rect.width > x1)
				blit_rect.width = x1 - x;
			BlitScreenIntern(*draw_bitmap, x, y, blit_rect, need_scale, 0);
		}
	}
}

void BitmapScreen::SetDirty() {
	needs_refresh = true;
	bitmap_changed = true;
}

void BitmapScreen::SetBitmap(BitmapRef const& source) {
    if (bitmap)
		bitmap->DetachBitmapScreen(this);

	bitmap = source;
	needs_refresh = true;
	bitmap_changed = true;

	if (bitmap) {
		bitmap->AttachBitmapScreen(this);
		src_rect_effect = bitmap->GetRect();
	} else {
		src_rect_effect = Rect();
	}
}

BitmapRef const& BitmapScreen::GetBitmap() {
	return bitmap;
}

void BitmapScreen::ClearEffects() {
	needs_refresh = true;

	opacity_top_effect = 255;
	opacity_bottom_effect = 128;
	bush_effect = 0;
	tone_effect = Tone();
	src_rect_effect = Rect(0, 0, 0, 0);
	flipx_effect = false;
	flipy_effect = false;
	zoom_x_effect = 1.0;
	zoom_y_effect = 1.0;
	angle_effect = 0.0;
	waver_effect_depth = 0;
	waver_effect_phase = 0.0;
	flash_effect = Color(0,0,0,0);
}

void BitmapScreen::SetSrcRect(Rect const& src_rect) {
	if (src_rect_effect != src_rect) {
		src_rect_effect = src_rect;
		needs_refresh = true;
	}
}

void BitmapScreen::SetOpacityEffect(int opacity_top, int opacity_bottom) {
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

void BitmapScreen::SetBushDepthEffect(int bush_depth) {
	if (bush_effect != bush_depth) {
		bush_effect = bush_depth;
		needs_refresh = true;
	}
}

void BitmapScreen::SetToneEffect(Tone tone) {
	if (tone_effect != tone) {
		tone_effect = tone;
		needs_refresh = true;
	}
}

void BitmapScreen::SetFlashEffect(const Color &color) {
	if (flash_effect != color) {
		flash_effect = color;
		needs_refresh = true;
	}
}

void BitmapScreen::SetFlipXEffect(bool flipx) {
	if (flipx_effect != flipx) {
		flipx_effect = flipx;
		needs_refresh = true;
	}
}

void BitmapScreen::SetFlipYEffect(bool flipy) {
	if (flipy_effect != flipy) {
		flipy_effect = flipy;
		needs_refresh = true;
	}
}

void BitmapScreen::SetZoomXEffect(double zoom_x) {
	if (zoom_x_effect != zoom_x) {
		zoom_x_effect = zoom_x;
		needs_refresh = true;
	}
}

void BitmapScreen::SetZoomYEffect(double zoom_y) {
	if (zoom_y_effect != zoom_y) {
		zoom_y_effect = zoom_y;
		needs_refresh = true;
	}
}

void BitmapScreen::SetAngleEffect(double angle) {
	if (angle_effect != angle) {
		angle_effect = angle;
		needs_refresh = true;
	}
}

void BitmapScreen::SetWaverEffectDepth(int depth) {
	if (waver_effect_depth != depth) {
		waver_effect_depth = depth;
		needs_refresh = true;
	}
}

void BitmapScreen::SetWaverEffectPhase(double phase) {
	if (waver_effect_phase != phase) {
		waver_effect_phase = phase;
		needs_refresh = true;
	}
}

void BitmapScreen::SetBlendType(int blend_type) {
	blend_type_effect = blend_type;
}

void BitmapScreen::SetBlendColor(Color const& blend_color) {
	blend_color_effect = blend_color;
}

Rect const& BitmapScreen::GetSrcRect() const {
	return src_rect_effect;
}

int BitmapScreen::GetOpacityEffect(int which) const {
	return which > 0 ? opacity_bottom_effect : opacity_top_effect;
}

int BitmapScreen::GetBushDepthEffect() const {
	return bush_effect;
}

Tone BitmapScreen::GetToneEffect() const {
	return tone_effect;
}

bool BitmapScreen::GetFlipXEffect() const {
	return flipx_effect;
}

bool BitmapScreen::GetFlipYEffect() const {
	return flipy_effect;
}

double BitmapScreen::GetZoomXEffect() const {
	return zoom_x_effect;
}

double BitmapScreen::GetZoomYEffect() const {
	return zoom_y_effect;
}

double BitmapScreen::GetAngleEffect() const {
	return angle_effect;
}

int BitmapScreen::GetBlendType() const {
	return blend_type_effect;
}

Color const& BitmapScreen::GetBlendColor() const {
	return blend_color_effect;
}

int BitmapScreen::GetWaverEffectDepth() const {
	return waver_effect_depth;
}

double BitmapScreen::GetWaverEffectPhase() const {
	return waver_effect_phase;
}

void BitmapScreen::BlitScreenIntern(Bitmap const& draw_bitmap, int x, int y,
									Rect const& src_rect, bool need_scale, int bush_y) {
	if (! &draw_bitmap)
		return;

	BitmapRef dst = DisplayUi->GetDisplaySurface();

	int opacity_split = bush_y;

	double zoom_x = need_scale ? zoom_x_effect : 1.0;
	double zoom_y = need_scale ? zoom_y_effect : 1.0;

	dst->EffectsBlit(x, y, draw_bitmap, src_rect,
					 opacity_top_effect, opacity_bottom_effect, opacity_split,
					 Tone(), zoom_x, zoom_y, angle_effect * 3.14159 / 180,
					 waver_effect_depth, waver_effect_phase);
}

BitmapRef BitmapScreen::Refresh(Rect& rect, bool& need_scale, int& bush_y) {
	need_scale = false;

	rect.Adjust(bitmap->GetWidth(), bitmap->GetHeight());

	if (rect.IsOutOfBounds(bitmap->GetWidth(), bitmap->GetHeight()))
	return BitmapRef();

	bool no_tone = tone_effect == Tone();
	bool no_flash = flash_effect.alpha == 0;
	bool no_flip = !flipx_effect && !flipy_effect;
	bool no_effects = no_tone && no_flash && no_flip;
	bool no_zoom = zoom_x_effect == 1.0 && zoom_y_effect == 1.0;
	bool effects_changed = tone_effect != current_tone ||
		flash_effect != current_flash ||
		flipx_effect != current_flip_x ||
		flipy_effect != current_flip_y;
	bool effects_rect_changed = rect != bitmap_effects_src_rect;

	if (effects_changed || effects_rect_changed || bitmap_changed) {
		bitmap_effects_valid = false;
		bitmap_scale_valid = false;
	}

	if (no_effects && no_zoom)
		return bitmap;

	if (bitmap_effects != NULL && bitmap_effects_valid && no_zoom)
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

		if (bitmap_effects != NULL &&
			bitmap_effects->GetWidth() < rect.x + rect.width &&
			bitmap_effects->GetHeight() < rect.y + rect.height) {
		bitmap_effects.reset();
		}

		if (!bitmap_effects)
			bitmap_effects = Bitmap::Create(bitmap->GetWidth(), bitmap->GetHeight(), true);

		bitmap_effects->Clear();
		if (no_tone && no_flash)
			bitmap_effects->FlipBlit(rect.x, rect.y, *bitmap, rect, flipx_effect, flipy_effect);
		else if (no_flip && no_flash)
			bitmap_effects->ToneBlit(rect.x, rect.y, *bitmap, rect, tone_effect);
		else if (no_flip && no_tone)
			bitmap_effects->BlendBlit(rect.x, rect.y, *bitmap, rect, flash_effect);
		else if (no_flash) {
			bitmap_effects->ToneBlit(rect.x, rect.y, *bitmap, rect, tone_effect);
			bitmap_effects->Flip(rect, flipx_effect, flipy_effect);
		}
		else if (no_tone) {
			bitmap_effects->BlendBlit(rect.x, rect.y, *bitmap, rect, flash_effect);
			bitmap_effects->Flip(rect, flipx_effect, flipy_effect);
		}
		else if (no_flip) {
			bitmap_effects->BlendBlit(rect.x, rect.y, *bitmap, rect, flash_effect);
			bitmap_effects->ToneBlit(rect.x, rect.y, *bitmap_effects, rect, tone_effect);
		}
		else {
			bitmap_effects->BlendBlit(rect.x, rect.y, *bitmap, rect, flash_effect);
			bitmap_effects->ToneBlit(rect.x, rect.y, *bitmap_effects, rect, tone_effect);
			bitmap_effects->Flip(rect, flipx_effect, flipy_effect);
		}

		bitmap_effects_src_rect = rect;
		bitmap_effects_valid = true;

		src_bitmap = bitmap_effects;
	}

	if (no_zoom || angle_effect != 0.0)
		return src_bitmap;

	int zoomed_width  = (int)(rect.width  * zoom_x_effect);
	int zoomed_height = (int)(rect.height * zoom_y_effect);

	if (zoomed_width > 640 || zoomed_height > 640) {
		need_scale = true;
		return src_bitmap;
	}

	bool zoom_changed =
		zoom_x_effect != current_zoom_x ||
		zoom_y_effect != current_zoom_y;

	bool scale_rect_changed = rect != bitmap_scale_src_rect;

	if (zoom_changed || scale_rect_changed)
		bitmap_scale_valid = false;

	if (bitmap_scale != NULL && bitmap_scale_valid) {
		bush_y = bush_y * bitmap_scale->GetHeight() / rect.height;
		rect = bitmap_scale->GetRect();
		return bitmap_scale;
	}

	current_zoom_x = zoom_x_effect;
	current_zoom_y = zoom_y_effect;

	bitmap_scale.reset();

	bitmap_scale = src_bitmap->Resample(zoomed_width, zoomed_height, rect);

	bitmap_scale_src_rect = rect;
	bitmap_scale_valid = true;

	bush_y = bush_y * bitmap_scale->GetHeight() / rect.height;
	rect = bitmap_scale->GetRect();
	return bitmap_scale;
}
