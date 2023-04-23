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
#include <lcf/data.h>
#include <lcf/rpg/terrain.h>
#include "cache.h"
#include "background.h"
#include "bitmap.h"
#include "main_data.h"
#include <lcf/reader_util.h>
#include "output.h"
#include "drawable_mgr.h"
#include "game_screen.h"
#include "player.h"

Background::Background(const std::string& name) : Drawable(Priority_Background)
{
	DrawableMgr::Register(this);

	if (!name.empty()) {
		FileRequestAsync* request = AsyncHandler::RequestFile("Backdrop", name);
		request->SetGraphicFile(true);
		bg_request_id = request->Bind(&Background::OnBackgroundGraphicReady, this);
		request->Start();
	}
}

Background::Background(int terrain_id) : Drawable(Priority_Background)
{
	DrawableMgr::Register(this);

	const lcf::rpg::Terrain* terrain = lcf::ReaderUtil::GetElement(lcf::Data::terrains, terrain_id);

	if (!terrain) {
		Output::Warning("Background: Invalid terrain ID {}", terrain_id);
		return;
	}

	// Either background or frame
	if (terrain->background_type == lcf::rpg::Terrain::BGAssociation_background && !terrain->background_name.empty()) {
		FileRequestAsync* request = AsyncHandler::RequestFile("Backdrop", terrain->background_name);
		request->SetGraphicFile(true);
		bg_request_id = request->Bind(&Background::OnBackgroundGraphicReady, this);
		request->Start();
		return;
	}

	// Frame
	if (!terrain->background_a_name.empty()) {
		FileRequestAsync* request = AsyncHandler::RequestFile("Frame", terrain->background_a_name);
		request->SetGraphicFile(true);
		bg_request_id = request->Bind(&Background::OnBackgroundGraphicReady, this);
		request->Start();

		bg_hscroll = terrain->background_a_scrollh ? terrain->background_a_scrollh_speed : 0;
		bg_vscroll = terrain->background_a_scrollv ? terrain->background_a_scrollv_speed : 0;
	}

	if (terrain->background_b && !terrain->background_b_name.empty()) {
		FileRequestAsync* request = AsyncHandler::RequestFile("Frame", terrain->background_b_name);
		request->SetGraphicFile(true);
		fg_request_id = request->Bind(&Background::OnForegroundFrameGraphicReady, this);
		request->Start();

		fg_hscroll = terrain->background_b_scrollh ? terrain->background_b_scrollh_speed : 0;
		fg_vscroll = terrain->background_b_scrollv ? terrain->background_b_scrollv_speed : 0;
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

void Background::Update(int& rate, int& value) {
	int step =
		(rate > 0) ? 2 << rate :
		(rate < 0) ? 2 << -rate :
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

void Background::Draw(Bitmap& dst) {
	Rect dst_rect = dst.GetRect();

	// If the background doesn't fill the screen, center it to support custom resolutions
	BitmapRef center_bitmap = bg_bitmap ? bg_bitmap : fg_bitmap;
	if (center_bitmap) {
		if (center_bitmap->GetWidth() < Player::screen_width) {
			dst_rect.x += Player::menu_offset_x;
			dst_rect.width = MENU_WIDTH;
		}
		if (center_bitmap->GetHeight() < Player::screen_height) {
			dst_rect.y += Player::menu_offset_y;
			dst_rect.height = MENU_HEIGHT;
		}
	}

	dst_rect.x += Main_Data::game_screen->GetShakeOffsetX();
	dst_rect.y += Main_Data::game_screen->GetShakeOffsetY();

	if (bg_bitmap)
		dst.TiledBlit(-Scale(bg_x), -Scale(bg_y), bg_bitmap->GetRect(), *bg_bitmap, dst_rect, 255);

	if (fg_bitmap)
		dst.TiledBlit(-Scale(fg_x), -Scale(fg_y), fg_bitmap->GetRect(), *fg_bitmap, dst_rect, 255);

	if (tone_effect != Tone()) {
		dst.ToneBlit(0, 0, dst, dst.GetRect(), tone_effect, Opacity::Opaque());
	}
}
