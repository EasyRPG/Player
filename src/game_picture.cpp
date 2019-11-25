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

#define _USE_MATH_DEFINES
#include "bitmap.h"
#include "options.h"
#include "cache.h"
#include "output.h"
#include "game_map.h"
#include "game_picture.h"
#include "player.h"
#include "main_data.h"
#include "reader_util.h"

// Applied to ensure that all pictures are above "normal" objects on this layer
constexpr int z_mask = (1 << 16);

Game_Picture::Game_Picture(int ID) :
	id(ID)
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

		int sx = sheet_bitmap->GetWidth() * ((last_spritesheet_frame) % data.spritesheet_cols);
		int sy = sheet_bitmap->GetHeight() * ((last_spritesheet_frame) / data.spritesheet_cols % data.spritesheet_rows);
		Rect r(sx, sy, sheet_bitmap->GetWidth(), sheet_bitmap->GetHeight());

		sheet_bitmap->Clear();

		if (last_spritesheet_frame >= 0 && last_spritesheet_frame < data.spritesheet_cols * data.spritesheet_rows) {
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

	sprite->SetAngle(data.effect_mode != RPG::SavePicture::Effect_wave ? data.current_rotation * (2 * M_PI) / 256 : 0.0);
	sprite->SetWaverPhase(data.effect_mode == RPG::SavePicture::Effect_wave ? data.current_waver * (2 * M_PI) / 256 : 0.0);
	sprite->SetWaverDepth(data.effect_mode == RPG::SavePicture::Effect_wave ? data.current_effect_power * 2 : 0);

	// Only older versions of RPG_RT apply the effects of current_bot_trans chunk.
	const bool use_bottom_trans = (Player::IsRPG2k3() && !Player::IsRPG2k3E());
	const auto top_trans = data.current_top_trans;
	const auto bottom_trans = use_bottom_trans ? data.current_bot_trans : top_trans;

	sprite->SetOpacity(
		(int)(255 * (100 - top_trans) / 100),
		(int)(255 * (100 - bottom_trans) / 100));

	if (bottom_trans != top_trans) {
		sprite->SetBushDepth(sprite->GetHeight() / 2);
	} else {
		sprite->SetBushDepth(0);
	}

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
		auto flash_color = Main_Data::game_screen->GetFlashColor();
		if (flash_color.alpha > 0) {
			sprite->Flash(flash_color, 0);
		} else {
			sprite->Flash(Color(), 0);
		}
	}
}

void Game_Picture::Show(const ShowParams& params) {
	RPG::SavePicture& data = GetData();

	data.name = params.name;
	data.use_transparent_color = params.use_transparent_color;
	data.fixed_to_map = params.fixed_to_map;
	SetNonEffectParams(params);

	data.effect_mode = params.effect_mode;
	if (data.effect_mode == RPG::SavePicture::Effect_none) {
		// params.effect_power seems to contain garbage here
		data.finish_effect_power = 0.0;
	} else {
		data.finish_effect_power = params.effect_power;
	}

	SyncCurrentToFinish();
	data.start_x = data.current_x;
	data.start_y = data.current_y;
	data.current_rotation = 0.0;
	data.current_waver = 0;
	data.time_left = 0;

	// RPG Maker 2k3 1.12
	data.frames = 0;
	data.spritesheet_rows = params.spritesheet_rows;
	data.spritesheet_cols = params.spritesheet_cols;
	data.spritesheet_play_once = params.spritesheet_play_once;
	data.spritesheet_frame = params.spritesheet_frame;
	data.spritesheet_speed = params.spritesheet_speed;
	data.map_layer = params.map_layer;
	data.battle_layer = params.battle_layer;
	data.flags.erase_on_map_change = (params.flags & 1) == 1;
	data.flags.erase_on_battle_end = (params.flags & 2) == 2;
	data.flags.affected_by_tint = (params.flags & 16) == 16;
	data.flags.affected_by_flash = (params.flags & 32) == 32;
	data.flags.affected_by_shake = (params.flags & 64) == 64;
	last_spritesheet_frame = -1;
	sheet_bitmap.reset();

	RequestPictureSprite();
	UpdateSprite();
}

void Game_Picture::Move(const MoveParams& params) {
	RPG::SavePicture& data = GetData();

	SetNonEffectParams(params);
	data.time_left = params.duration * DEFAULT_FPS / 10;

	// Note that data.effect_mode doesn't necessarily reflect the
	// last effect set. Possible states are:
	//
	// * effect_mode == RPG::SavePicture::Effect_none && finish_effect_power == 0
	//   Picture has not had an effect set since Show.
	// * effect_mode == RPG::SavePicture::Effect_none && finish_effect_power != 0
	//   Picture was set to no effect; previously, it was rotating.
	// * effect_mode == RPG::SavePicture::Effect_wave && finish_effect_power == 0
	//   Picture was set to no effect; previously, it was wavering.
	// * effect_mode == RPG::SavePicture::Effect_rotation
	//   Picture was set to rotate.
	// * effect_mode == RPG::SavePicture::Effect_wave && finish_effect_power != 0
	//   Picture was set to waver.

	bool started_with_no_effect =
		data.effect_mode == RPG::SavePicture::Effect_none && data.finish_effect_power == 0.0;
	if (Player::IsRPG2k() && started_with_no_effect) {
		// Possibly a bug(?) in RM2k: if Show Picture command has no
		// effect, a Move Picture command cannot add one
		return;
	}

	if (data.effect_mode == RPG::SavePicture::Effect_none && params.effect_mode == RPG::SavePicture::Effect_none) {
		// Nothing to do
	} else if (data.effect_mode == params.effect_mode) {
		data.finish_effect_power = params.effect_power;
	} else if (data.effect_mode == RPG::SavePicture::Effect_rotation && params.effect_mode == RPG::SavePicture::Effect_none) {
		data.effect_mode = RPG::SavePicture::Effect_none;
	} else if (data.effect_mode == RPG::SavePicture::Effect_wave && params.effect_mode == RPG::SavePicture::Effect_none) {
		data.finish_effect_power = 0;
	} else {
		data.effect_mode = params.effect_mode;
		data.current_effect_power = params.effect_power;
		data.finish_effect_power = params.effect_power;
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
	request->SetGraphicFile(true);
	request_id = request->Bind(&Game_Picture::OnPictureSpriteReady, this);
	request->Start();
}

void Game_Picture::OnPictureSpriteReady(FileRequestResult*) {
	RPG::SavePicture& data = GetData();

	whole_bitmap = Cache::Picture(data.name, data.use_transparent_color);

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

	if (data.fixed_to_map) {
		// Instead of modifying the Ox/Oy offset the real position is altered
		// based on map scroll because of savegame compatibility with RPG_RT

		double dx = Game_Map::GetScrolledRight() / TILE_SIZE;

		data.finish_x = data.finish_x - dx;
		data.current_x = data.current_x - dx;
		data.start_x = data.start_x - dx;

		double dy = Game_Map::GetScrolledDown() / TILE_SIZE;

		data.finish_y = data.finish_y - dy;
		data.current_y = data.current_y - dy;
		data.start_y = data.start_y - dy;
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
		if (data.effect_mode != RPG::SavePicture::Effect_none) {
			data.current_effect_power = interpolate(data.current_effect_power, data.finish_effect_power);
		}

		data.time_left = data.time_left - 1;
	}

	// Update rotation
	// When a move picture disables rotation effect, we continue rotating
	// until one full revolution is done. There is a bug in RPG_RT where this
	// only happens when the current rotation and power is positive. We emulate this for now.
	if (data.effect_mode == RPG::SavePicture::Effect_rotation ||
			(data.effect_mode == RPG::SavePicture::Effect_none
			 && data.current_rotation > 0
			 && data.current_effect_power > 0)
			)
	{

		// RPG_RT always scales the rotation down to [0, 256] when this case is triggered.
		if (data.effect_mode == RPG::SavePicture::Effect_none && data.current_rotation >= 256) {
			data.current_rotation = std::remainder(data.current_rotation, 256.0);
		}

		data.current_rotation = data.current_rotation + data.current_effect_power;

		// Rotation finally ends after full revolution.
		if (data.effect_mode == RPG::SavePicture::Effect_none && data.current_rotation >= 256) {
			data.current_rotation = 0;
		}
	}

	// Update waver phase
	if (data.effect_mode == RPG::SavePicture::Effect_wave) {
		data.current_waver = data.current_waver + 8;
	}

	// RPG Maker 2k3 1.12: Animated spritesheets
	if (Player::IsRPG2k3E()) {
		data.frames = data.frames + 1;

		if (data.spritesheet_speed > 0) {
			if (data.frames > data.spritesheet_speed) {
				data.frames = 1;
				data.spritesheet_frame = data.spritesheet_frame + 1;

				if (data.spritesheet_frame >= data.spritesheet_rows * data.spritesheet_cols) {
					data.spritesheet_frame = 0;
					if (data.spritesheet_play_once && !data.name.empty()) {
						Erase(true);
					}
				}
			}
		}
	}

	if (data.name.empty()) {
		return;
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
	data.current_effect_power = data.finish_effect_power;
}

RPG::SavePicture& Game_Picture::GetData() const {
	// Save: Picture array is guaranteed to be of correct size
	return *ReaderUtil::GetElement(Main_Data::game_data.pictures, id);
}
