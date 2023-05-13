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
#include "util_macro.h"
#include "bitmap.h"
#include "cache.h"
#include "drawable_mgr.h"

// Constructor
Sprite::Sprite(Drawable::Flags flags) : Drawable(0, flags)
{
	DrawableMgr::Register(this);
}

// Draw
void Sprite::Draw(Bitmap& dst) {
	if (GetWidth() <= 0 || GetHeight() <= 0) return;

	BlitScreen(dst);
}

void Sprite::BlitScreen(Bitmap& dst) {
	if (!bitmap || (opacity_top_effect <= 0 && opacity_bottom_effect <= 0))
		return;

	BitmapRef draw_bitmap = Refresh(src_rect_effect);
	if (!draw_bitmap) {
		return;
	}

	bitmap_changed = false;

	Rect rect = src_rect_effect.GetSubRect(src_rect);
	if (draw_bitmap == bitmap_effects) {
		// When a "sprite rect" (src_rect_effect) is used bitmap_effects
		// only has the size of this subrect instead of the whole bitmap
		rect.x %= bitmap_effects->GetWidth();
		rect.y %= bitmap_effects->GetHeight();

		if (flipx_effect) {
			rect.x = bitmap_effects->GetWidth() - rect.x - rect.width;
		}

		if (flipy_effect) {
			rect.y = bitmap_effects->GetHeight() - rect.y - rect.height;
		}
	}

	BlitScreenIntern(dst, *draw_bitmap, rect);
}

void Sprite::BlitScreenIntern(Bitmap& dst, Bitmap const& draw_bitmap, Rect const& src_rect) const
{
	double zoom_x = zoom_x_effect;
	double zoom_y = zoom_y_effect;

	dst.EffectsBlit(x, y, ox, oy, draw_bitmap, src_rect,
		Opacity(opacity_top_effect, opacity_bottom_effect, bush_effect),
		zoom_x, zoom_y, angle_effect,
		waver_effect_depth, waver_effect_phase, static_cast<Bitmap::BlendMode>(blend_type_effect));
}

BitmapRef Sprite::Refresh(Rect& rect) {
	if (zoom_x_effect == 1.0 && zoom_y_effect == 1.0 && angle_effect == 0.0 && waver_effect_depth == 0) {
		// Prevent effect sprite creation when not in the viewport
		// TODO: Out of bounds math adjustments for zoom, angle and waver
		// but even without this will catch most of the cases
		if (Rect(x - ox, y - oy, GetWidth(), GetHeight()).IsOutOfBounds(Rect(0, 0, Player::screen_width, Player::screen_height))) {
			return BitmapRef();
		}
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

	if (no_effects || effects_changed || effects_rect_changed || bitmap_changed) {
		bitmap_effects.reset();
	}

	if (no_effects) {
		return bitmap;
	} else if (bitmap_effects) {
		return bitmap_effects;
	} else {
		current_tone = tone_effect;
		current_flash = flash_effect;
		current_flip_x = flipx_effect;
		current_flip_y = flipy_effect;

		bitmap_effects = Cache::SpriteEffect(bitmap, rect, flipx_effect, flipy_effect, current_tone, current_flash);
		bitmap_effects_src_rect = rect;

		return bitmap_effects;
	}
}

void Sprite::SetBitmap(BitmapRef const& nbitmap) {
	bitmap = nbitmap;
	if (!bitmap) {
		src_rect = Rect();
	} else {
		src_rect = bitmap->GetRect();
	}

	src_rect_effect = src_rect;

	bitmap_changed = true;
}

void Sprite::SetOpacity(int opacity_top, int opacity_bottom) {
	if (opacity_top_effect != opacity_top) {
		opacity_top_effect = opacity_top;
	}
	if (opacity_bottom == -1)
		opacity_bottom = (opacity_top + 1) / 2;
	if (opacity_bottom_effect != opacity_bottom) {
		opacity_bottom_effect = opacity_bottom;
	}
}

