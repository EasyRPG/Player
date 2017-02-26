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
#include "main_data.h"
#include "game_map.h"

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

	if (needs_refresh) {
		needs_refresh = false;

		if (!tone_bitmap ||
			bitmap->GetWidth() != tone_bitmap->GetWidth() ||
			bitmap->GetHeight() != tone_bitmap->GetHeight()) {
			tone_bitmap = Bitmap::Create(bitmap->GetWidth(), bitmap->GetHeight());
		}
		tone_bitmap->Clear();
		tone_bitmap->ToneBlit(0, 0, *bitmap, bitmap->GetRect(), tone_effect, Opacity::opaque);
	}

	BitmapRef source = tone_effect == Tone() ? bitmap : tone_bitmap;

	BitmapRef dst = DisplayUi->GetDisplaySurface();
	Rect dst_rect(0, 0, DisplayUi->GetWidth(), DisplayUi->GetHeight());
	int src_x = -ox;
	int src_y = -oy;

	// Apply screen shaking
	int shake_pos = Main_Data::game_data.screen.shake_position;
	if (Game_Map::LoopHorizontal()) {
		src_x += shake_pos;
	} else {
		// The panorama occupies the same rectangle as the whole map.
		int map_width = Game_Map::GetWidth() * TILE_SIZE;
		int map_height = Game_Map::GetHeight() * TILE_SIZE;
		Rect panorama_rect(0, 0, map_width, map_height);

		// Change into coordinate where the top-left of the screen is
		// the origin.
		panorama_rect.x -= Game_Map::GetDisplayX() / TILE_SIZE - shake_pos;
		panorama_rect.y -= Game_Map::GetDisplayY() / TILE_SIZE;

		panorama_rect.Adjust(dst_rect);
		dst_rect = panorama_rect;

		if (panorama_rect.IsEmpty()) {
			// The map is completely off-screen (probably won't happen)
			return;
		}

		// Correct the offset if the top-left corner moved.
		src_x += shake_pos + dst_rect.x;
	}

	dst->TiledBlit(src_x, src_y, source->GetRect(), *source, dst_rect, 255);
}

BitmapRef const& Plane::GetBitmap() const {
	return bitmap;
}
void Plane::SetBitmap(BitmapRef const& nbitmap) {
	bitmap = nbitmap;

	needs_refresh = true;
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

Tone Plane::GetTone() const {
	return tone_effect;
}

void Plane::SetTone(Tone tone) {
	if (tone_effect != tone) {
		tone_effect = tone;
		needs_refresh = true;
	}
}

DrawableType Plane::GetType() const {
	return type;
}
