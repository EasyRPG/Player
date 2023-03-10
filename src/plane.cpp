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
#include "player.h"
#include "bitmap.h"
#include "main_data.h"
#include "game_map.h"
#include "drawable_mgr.h"
#include "game_screen.h"

Plane::Plane() : Drawable(0)
{
	DrawableMgr::Register(this);
}

void Plane::Draw(Bitmap& dst) {
	if (!bitmap) return;

	if (needs_refresh) {
		needs_refresh = false;

		if (!tone_bitmap ||
			bitmap->GetWidth() != tone_bitmap->GetWidth() ||
			bitmap->GetHeight() != tone_bitmap->GetHeight()) {
			tone_bitmap = Bitmap::Create(bitmap->GetWidth(), bitmap->GetHeight());
		}
		tone_bitmap->Clear();
		tone_bitmap->ToneBlit(0, 0, *bitmap, bitmap->GetRect(), tone_effect, Opacity::Opaque());
	}

	BitmapRef source = tone_effect == Tone() ? bitmap : tone_bitmap;

	Rect dst_rect = dst.GetRect();
	int src_x = -ox;
	int src_y = -oy;

	// Apply screen shaking
	const int shake_x = Main_Data::game_screen->GetShakeOffsetX();
	const int shake_y = Main_Data::game_screen->GetShakeOffsetY();
	if (Game_Map::LoopHorizontal()) {
		src_x += shake_x;
	} else {
		// The panorama occupies the same rectangle as the whole map.
		// Using coordinates where the top-left of the screen is the origin...
		// Minimal width is a 20 tile wide map by default, smaller maps are hacked
		int bg_x = -Game_Map::GetDisplayX() / TILE_SIZE + shake_x;
		int bg_width = std::max(Game_Map::GetWidth() * TILE_SIZE, Player::screen_width);

		// Clip the panorama to the screen
		if (bg_x < 0) {
			bg_width += bg_x;
			bg_x = 0;
		}
		if (dst_rect.width < bg_x + bg_width) {
			bg_width = dst_rect.width - bg_x;
		}

		bool off_screen =
			bg_x >= dst_rect.width ||
			bg_x + bg_width <= 0;
		if (off_screen) {
			// This probably won't happen...
			return;
		}

		dst_rect.x = bg_x;
		dst_rect.width = bg_width;

		// Correct the offset if the top-left corner moved.
		src_x += shake_x + bg_x;
	}
	src_y += shake_y;

	dst.TiledBlit(src_x, src_y, source->GetRect(), *source, dst_rect, 255);
}

