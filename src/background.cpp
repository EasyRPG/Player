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
#include "data.h"
#include "rpg_terrain.h"
#include "baseui.h"
#include "graphics.h"
#include "cache.h"
#include "background.h"
#include "bitmap_screen.h"
#include "bitmap.h"

Background::Background(const std::string& name) :
	visible(true),
	bg_hscroll(0), bg_vscroll(0), bg_x(0), bg_y(0),
	fg_hscroll(0), fg_vscroll(0), fg_x(0), fg_y(0) {

	Graphics::RegisterDrawable(this);

	bg_screen = BitmapScreen::Create(Cache::Backdrop(name));
}

Background::Background(int terrain_id) :
	visible(true),
	bg_hscroll(0), bg_vscroll(0), bg_x(0), bg_y(0),
	fg_hscroll(0), fg_vscroll(0), fg_x(0), fg_y(0) {

	Graphics::RegisterDrawable(this);

	const RPG::Terrain& terrain = Data::terrains[terrain_id - 1];

	if (terrain.background_type == 0) {
		bg_screen = BitmapScreen::Create(Cache::Backdrop(terrain.background_name));
		return;
	}

	bg_screen = BitmapScreen::Create(Cache::Frame(terrain.background_a_name));
	bg_hscroll = terrain.background_a_scrollh ? terrain.background_a_scrollh_speed : 0;
	bg_vscroll = terrain.background_a_scrollv ? terrain.background_a_scrollv_speed : 0;

	if (terrain.background_b) {
		fg_screen = BitmapScreen::Create(Cache::Frame(terrain.background_b_name));
		fg_hscroll = terrain.background_b_scrollh ? terrain.background_b_scrollh_speed : 0;
		fg_vscroll = terrain.background_b_scrollv ? terrain.background_b_scrollv_speed : 0;
	}
}

Background::~Background() {
	Graphics::RemoveDrawable(this);
}

int Background::GetZ() const {
	return z;
}

DrawableType Background::GetType() const {
	return type;
}

void Background::Update(int& rate, int& value) {
	int step =
		(rate > 0) ? 1 << rate :
		(rate < 0) ? 1 << -rate :
		0;
	value += step;
}

void Background::Update() {
	Update(bg_hscroll, bg_x);
	Update(bg_vscroll, bg_y);
	Update(fg_hscroll, fg_x);
	Update(fg_vscroll, fg_y);
}

int Background::Scale(int x) {
	return x > 0 ? x / 64 : -(-x / 64);
}

void Background::Draw() {
	if (!visible)
		return;

	if (bg_screen)
		bg_screen->BlitScreenTiled(bg_screen->GetBitmap()->GetRect(),
								   DisplayUi->GetDisplaySurface()->GetRect(),
								   Scale(bg_x), Scale(bg_y));

	if (fg_screen)
		fg_screen->BlitScreenTiled(bg_screen->GetBitmap()->GetRect(),
								   DisplayUi->GetDisplaySurface()->GetRect(),
								   Scale(fg_x), Scale(fg_y));
}
