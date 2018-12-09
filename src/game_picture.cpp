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

#include "bitmap.h"
#include "options.h"
#include "cache.h"
#include "game_map.h"
#include "game_picture.h"
#include "player.h"
#include "main_data.h"
#include "reader_util.h"

// Applied to ensure that all pictures are above "normal" objects on this layer
constexpr int z_mask = (1 << 16);

Game_Picture::Game_Picture(int ID) :
	id(ID),
	old_map_x(Game_Map::GetDisplayX()),
	old_map_y(Game_Map::GetDisplayY())
{
	RequestPictureSprite();
}

Game_Picture::~Game_Picture() {
	if (id <= 0 || id -1 >= static_cast<int>(Main_Data::game_data.pictures.size())) {
		// Prevent crash on game load when old pictures are destroyed
		// but the new pictures array is smaller then the old one
		return;
	}

	GetData().name.clear();
}

void Game_Picture::UpdateSprite() {
	RPG::SavePicture& data = GetData();

	if (!sprite)
		return;
	if (data.name.empty())
		return;

	// RPG Maker 2k3 1.12: Spritesheets
	if (HasSpritesheet() && (data.spritesheet_frame != last_spritesheet_frame || !sheet_bitmap)) {
		// Usage of an additional bitmap instead of Subrect is necessary because the Subrect
		// approach will fail while the bitmap is rotated because the outer parts will be
		// visible for degrees != 90 * n
		if (!sheet_bitmap) {
			int frame_width = whole_bitmap->GetWidth() / data.spritesheet_cols;
			int frame_height = whole_bitmap->GetHeight() / data.spritesheet_rows;

			sheet_bitmap = Bitmap::Create(frame_width, frame_height);
		}

		last_spritesheet_frame = data.spritesheet_frame;

		int sx = sheet_bitmap->GetWidth() * ((last_spritesheet_frame - 1) % data.spritesheet_cols);
		int sy = sheet_bitmap->GetHeight() * ((last_spritesheet_frame - 1) / data.spritesheet_cols % data.spritesheet_rows);
		Rect r(sx, sy, sheet_bitmap->GetWidth(), sheet_bitmap->GetHeight());

		sheet_bitmap->Clear();

		if (last_spritesheet_frame > 0 && last_spritesheet_frame <= data.spritesheet_cols * data.spritesheet_rows) {
			sheet_bitmap->Blit(0, 0, *whole_bitmap, r, Opacity::opaque);
		}

		sprite->SetBitmap(sheet_bitmap);
	}

	int x = data.current_x;
	int y = data.current_y;
	if (data.flags.affected_by_shake) {
		x -= Main_Data::game_data.screen.shake_position;
		y += Main_Data::game_data.screen.shake_position_y;
	}

	sprite->SetX(x);
	sprite->SetY(y);
	if (Player::IsMajorUpdatedVersion()) {
		// Battle Animations are above pictures
		int priority = Drawable::GetPriorityForMapLayer(data.map_layer);
		if (priority > 0) {
			sprite->SetZ(priority + z_mask + data.ID);
		}
	} else {
		// Battle Animations are below pictures
		sprite->SetZ(Priority_PictureOld + data.ID);
	}
	sprite->SetZoomX(data.current_magnify / 100.0);
	sprite->SetZoomY(data.current_magnify / 100.0);
	sprite->SetOx(sprite->GetBitmap()->GetWidth() / 2);
	sprite->SetOy(sprite->GetBitmap()->GetHeight() / 2);

	sprite->SetAngle(data.effect_mode != 2 ? data.current_rotation * 360 / 256 : 0.0);
	sprite->SetWaverPhase(data.effect_mode == 2 ? data.current_waver : 0.0);
	sprite->SetWaverDepth(data.effect_mode == 2 ? data.current_effect * 2 : 0);
	sprite->SetOpacity(
		(int)(255 * (100 - data.current_top_trans) / 100),
		(int)(255 * (100 - data.current_bot_trans) / 100));
	if (data.current_bot_trans != data.current_top_trans)
		sprite->SetBushDepth(sprite->GetHeight() / 2);

	auto tone = Tone((int) (data.current_red * 128 / 100),
			(int) (data.current_green * 128 / 100),
			(int) (data.current_blue * 128 / 100),
			(int) (data.current_sat * 128 / 100));
	if (data.flags.affected_by_tint) {
		auto screen_tone = Main_Data::game_screen->GetTone();
		tone = Blend(tone, screen_tone);
	}
	sprite->SetTone(tone);

	if (data.flags.affected_by_flash) {
		int flash_level = 0;
		int flash_time = 0;
		auto flash = Main_Data::game_screen->GetFlash(flash_level, flash_time);
		if (flash_time > 0) {
			flash.alpha = flash_level;
			sprite->Flash(flash, flash_time);
		}
	}
}

void Game_Picture::Show(const ShowParams& params) {
	RPG::SavePicture& data = GetData();

	data.name = params.name;
	data.transparency = params.transparency;
	data.fixed_to_map = params.fixed_to_map;
	SetNonEffectParams(params);
	data.effect_mode = params.effect_mode;
	if (data.effect_mode == 0) {
		// params.effect_power seems to contain garbage here
		data.finish_effect = 0.0;
	} else {
		data.finish_effect = params.effect_power;
	}

	SyncCurrentToFinish();
	data.current_rotation = 0.0;
	data.current_waver = 0;
	data.time_left = 0;

	// RPG Maker 2k3 1.12
	data.frames = 0;
	data.spritesheet_rows = params.spritesheet_rows;
	data.spritesheet_cols = params.spritesheet_cols;
	data.spritesheet_play_once = !params.spritesheet_loop;
	data.spritesheet_frame = params.spritesheet_frame;
	data.spritesheet_speed = params.spritesheet_speed;
	data.map_layer = params.map_layer;
	data.battle_layer = params.battle_layer;
	data.flags.erase_on_map_change = (params.flags & 1) == 1;
	data.flags.erase_on_battle_end = (params.flags & 2) == 2;
	data.flags.affected_by_tint = (params.flags & 16) == 16;
	data.flags.affected_by_flash = (params.flags & 32) == 32;
	data.flags.affected_by_shake = (params.flags & 64) == 64;
	last_spritesheet_frame = 0;
	sheet_bitmap.reset();

	RequestPictureSprite();
	UpdateSprite();

	old_map_x = Game_Map::GetDisplayX();
	old_map_y = Game_Map::GetDisplayY();
}

void Game_Picture::Move(const MoveParams& params) {
	RPG::SavePicture& data = GetData();

	SetNonEffectParams(params);
	data.time_left = params.duration * DEFAULT_FPS / 10;

	// Note that data.effect_mode doesn't necessarily reflect the
	// last effect set. Possible states are:
	//
	// * effect_mode == 0 && finish_effect == 0
	//   Picture has not had an effect set since Show.
	// * effect_mode == 0 && finish_effect != 0
	//   Picture was set to no effect; previously, it was rotating.
	// * effect_mode == 2 && finish_effect == 0
	//   Picture was set to no effect; previously, it was wavering.
	// * effect_mode == 1
	//   Picture was set to rotate.
	// * effect_mode == 2 && finish_effect != 0
	//   Picture was set to waver.

	bool started_with_no_effect =
		data.effect_mode == 0 && data.finish_effect == 0.0;
	if (Player::IsRPG2k() && started_with_no_effect) {
		// Possibly a bug(?) in RM2k: if Show Picture command has no
		// effect, a Move Picture command cannot add one
		return;
	}

	if (data.effect_mode == 0 && params.effect_mode == 0) {
		// Nothing to do
	} else if (data.effect_mode == params.effect_mode) {
		data.finish_effect = params.effect_power;
	} else if (data.effect_mode == 1 && params.effect_mode == 0) {
		data.effect_mode = 0;
	} else if (data.effect_mode == 2 && params.effect_mode == 0) {
		data.finish_effect = 0;
	} else {
		data.effect_mode = params.effect_mode;
		data.current_effect = params.effect_power;
		data.finish_effect = params.effect_power;
	}
}

void Game_Picture::Erase(bool force_erase) {
	RPG::SavePicture& data = GetData();

	if (!(force_erase || data.flags.erase_on_map_change)) {
		return;
	}

	request_id = FileRequestBinding();

	data.name.clear();
	sprite.reset();
	whole_bitmap.reset();
	sheet_bitmap.reset();
}

void Game_Picture::RequestPictureSprite() {
	const std::string& name = GetData().name;
	if (name.empty()) return;

	FileRequestAsync* request = AsyncHandler::RequestFile("Picture", name);
	request_id = request->Bind(&Game_Picture::OnPictureSpriteReady, this);
	request->Start();
}

void Game_Picture::OnPictureSpriteReady(FileRequestResult*) {
	RPG::SavePicture& data = GetData();

	whole_bitmap = Cache::Picture(data.name, data.transparency);

	sprite.reset(new Sprite());
	sprite->SetBitmap(whole_bitmap);

	UpdateSprite();
}

bool Game_Picture::HasSpritesheet() const {
	RPG::SavePicture& data = GetData();

	if (data.spritesheet_rows < 1 || data.spritesheet_cols < 1) {
		return false;
	}

	return data.spritesheet_rows > 1 || data.spritesheet_cols > 1;
}

void Game_Picture::Update() {
	RPG::SavePicture& data = GetData();

	if (data.name.empty())
		return;

	if (data.fixed_to_map) {
		// Instead of modifying the Ox/Oy offset the real position is altered
		// based on map scroll because of savegame compatibility with RPG_RT

		if (old_map_x != Game_Map::GetDisplayX()) {
			double mx = (old_map_x - Game_Map::GetDisplayX()) / (double)TILE_SIZE;

			data.finish_x = data.finish_x + mx;
			data.current_x = data.current_x + mx;
		}
		if (old_map_y != Game_Map::GetDisplayY()) {
			double my = (old_map_y - Game_Map::GetDisplayY()) / (double)TILE_SIZE;

			data.finish_y = data.finish_y + my;
			data.current_y = data.current_y + my;
		}

		old_map_x = Game_Map::GetDisplayX();
		old_map_y = Game_Map::GetDisplayY();
	}

	if (data.time_left == 0) {
		SyncCurrentToFinish();
	} else {
		auto interpolate = [=](double current, double finish) {
			double d = data.time_left;
			return (current * (d - 1) + finish) / d;
		};

		data.current_x = interpolate(data.current_x, data.finish_x);
		data.current_y = interpolate(data.current_y, data.finish_y);
		data.current_red = interpolate(data.current_red, data.finish_red);
		data.current_green = interpolate(data.current_green, data.finish_green);
		data.current_blue = interpolate(data.current_blue, data.finish_blue);
		data.current_sat = interpolate(data.current_sat, data.finish_sat);
		data.current_magnify = interpolate(data.current_magnify, data.finish_magnify);
		data.current_top_trans = interpolate(data.current_top_trans, data.finish_top_trans);
		data.current_bot_trans = interpolate(data.current_bot_trans, data.finish_bot_trans);
		if (data.effect_mode != 0) {
			data.current_effect = interpolate(data.current_effect, data.finish_effect);
		}

		data.time_left = data.time_left - 1;
	}

	// Update rotation
	if (data.current_rotation >= 256.0) {
		data.current_rotation = data.current_rotation - 256.0;
	}
	bool is_rotating_but_stopping =
		data.effect_mode == 0 && (
			data.current_rotation != 0.0 ||
			data.current_effect * data.time_left >= 256.0
		);
	bool is_rotating =
		data.effect_mode == 1 ||
		is_rotating_but_stopping;
	if (is_rotating) {
		data.current_rotation = data.current_rotation + data.current_effect;
		if (is_rotating_but_stopping && data.current_rotation >= 256.0) {
			data.current_rotation = 0.0;
		}
	}

	// Update waver phase
	if (data.effect_mode == 2) {
		data.current_waver = data.current_waver + 10;
	}

	// RPG Maker 2k3 1.12: Spritesheets
	if (HasSpritesheet()) {
		if (data.spritesheet_speed > 0) {
			if (data.frames % data.spritesheet_speed == 0) {
				data.spritesheet_frame = data.spritesheet_frame + 1;

				if (data.spritesheet_frame > data.spritesheet_rows * data.spritesheet_cols) {
					if (data.spritesheet_play_once) {
						Erase(true);
						return;
					}

					data.spritesheet_frame = 1;
				}
			}
		}

		data.frames = data.frames + 1;
	}

	UpdateSprite();
}

void Game_Picture::SetNonEffectParams(const Params& params) {
	RPG::SavePicture& data = GetData();

	data.finish_x = params.position_x;
	data.finish_y = params.position_y;
	data.finish_magnify = params.magnify;
	data.finish_top_trans = params.top_trans;
	data.finish_bot_trans = params.bottom_trans;
	data.finish_red = params.red;
	data.finish_green = params.green;
	data.finish_blue = params.blue;
	data.finish_sat = params.saturation;
}

void Game_Picture::SyncCurrentToFinish() {
	RPG::SavePicture& data = GetData();

	data.current_x = data.finish_x;
	data.current_y = data.finish_y;
	data.current_red = data.finish_red;
	data.current_green = data.finish_green;
	data.current_blue = data.finish_blue;
	data.current_sat = data.finish_sat;
	data.current_magnify = data.finish_magnify;
	data.current_top_trans = data.finish_top_trans;
	data.current_bot_trans = data.finish_bot_trans;
	data.current_effect = data.finish_effect;
}

RPG::SavePicture& Game_Picture::GetData() const {
	// Save: Picture array is guaranteed to be of correct size
	return *ReaderUtil::GetElement(Main_Data::game_data.pictures, id);
}
