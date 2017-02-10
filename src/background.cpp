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
#include "bitmap.h"
#include "main_data.h"

Background::Background(const std::string& name) :
	visible(true),
	bg_hscroll(0), bg_vscroll(0), bg_x(0), bg_y(0),
	fg_hscroll(0), fg_vscroll(0), fg_x(0), fg_y(0) {

	Graphics::RegisterDrawable(this);

	if (!name.empty()) {
		FileRequestAsync* request = AsyncHandler::RequestFile("Backdrop", name);
		request_id = request->Bind(&Background::OnBackgroundGraphicReady, this);
		request->Start();
	}
}

Background::Background(int terrain_id) :
	visible(true),
	bg_hscroll(0), bg_vscroll(0), bg_x(0), bg_y(0),
	fg_hscroll(0), fg_vscroll(0), fg_x(0), fg_y(0) {

	Graphics::RegisterDrawable(this);

	const RPG::Terrain& terrain = Data::terrains[terrain_id - 1];

	// Either background or frame
	if (terrain.background_type == RPG::Terrain::BGAssociation_background && !terrain.background_name.empty()) {
		FileRequestAsync* request = AsyncHandler::RequestFile("Backdrop", terrain.background_name);
		request_id = request->Bind(&Background::OnBackgroundGraphicReady, this);
		request->Start();
		return;
	}

	// Frame
	if (!terrain.background_a_name.empty()) {
		FileRequestAsync* request = AsyncHandler::RequestFile("Frame", terrain.background_a_name);
		request_id = request->Bind(&Background::OnBackgroundGraphicReady, this);
		request->Start();

		bg_hscroll = terrain.background_a_scrollh ? terrain.background_a_scrollh_speed : 0;
		bg_vscroll = terrain.background_a_scrollv ? terrain.background_a_scrollv_speed : 0;
	}

	if (terrain.background_b && !terrain.background_b_name.empty()) {
		FileRequestAsync* request = AsyncHandler::RequestFile("Frame", terrain.background_b_name);
		request_id = request->Bind(&Background::OnForegroundFrameGraphicReady, this);
		request->Start();

		fg_hscroll = terrain.background_b_scrollh ? terrain.background_b_scrollh_speed : 0;
		fg_vscroll = terrain.background_b_scrollv ? terrain.background_b_scrollv_speed : 0;
	}
}

void Background::OnBackgroundGraphicReady(FileRequestResult* result) {
	if (result->directory == "Backdrop") {
		bg_bitmap = Cache::Backdrop(result->file);
	}
	else if (result->directory == "Frame") {
		bg_bitmap = Cache::Frame(result->file, false);
	}
}

void Background::OnForegroundFrameGraphicReady(FileRequestResult* result) {
	fg_bitmap = Cache::Frame(result->file);
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

	BitmapRef dst = DisplayUi->GetDisplaySurface();
	Rect dst_rect = dst->GetRect();

	int shake_pos = Main_Data::game_data.screen.shake_position;
	dst_rect.x += shake_pos;

	if (bg_bitmap)
		dst->TiledBlit(-Scale(bg_x), -Scale(bg_y), bg_bitmap->GetRect(), *bg_bitmap, dst_rect, 255);

	if (fg_bitmap)
		dst->TiledBlit(-Scale(fg_x), -Scale(fg_y), fg_bitmap->GetRect(), *fg_bitmap, dst_rect, 255);
}
