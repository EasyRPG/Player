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
#include <cmath>
#include "bitmap.h"
#include "options.h"
#include "cache.h"
#include "output.h"
#include "game_map.h"
#include "game_pictures.h"
#include "game_screen.h"
#include "player.h"
#include "main_data.h"
#include "scene.h"
#include "drawable_mgr.h"

// Applied to ensure that all pictures are above "normal" objects on this layer
constexpr int z_mask = (1 << 16);

static bool IsEmpty(const RPG::SavePicture& data, int frames) {
	RPG::SavePicture empty;
	empty.ID = data.ID;
	empty.frames = frames;

	return data == empty;
}

static bool IsEmpty(const RPG::SavePicture& data) {
	return IsEmpty(data, data.frames);
}

Game_Pictures::Picture::Picture(RPG::SavePicture save)
	: data(std::move(save))
{
	// FIXME: Make this more accurate by checking all animating chunks values to see if they all will remain stable.
	// Write unit tests to ensure it's correct.
	// Then add it to ErasePicture()
	needs_update = !IsEmpty(data);
}

void Game_Pictures::InitGraphics() {
	for (auto& pic: pictures) {
		RequestPictureSprite(pic);
	}
}

void Game_Pictures::SetSaveData(std::vector<RPG::SavePicture> save)
{
	pictures.clear();

	frame_counter = save.empty() ? 0 : save.back().frames;

	// Don't create pictures for empty save picture data at the end of the vector.
	int num_pictures = static_cast<int>(save.size());
	while (num_pictures > 0) {
		if (!IsEmpty(save[num_pictures - 1], frame_counter)) {
			break;
		}
		--num_pictures;
	}

	pictures.reserve(num_pictures);
	for (int i = 0; i < num_pictures; ++i) {
		pictures.emplace_back(std::move(save[i]));
	}
}

std::vector<RPG::SavePicture> Game_Pictures::GetSaveData() const {
	std::vector<RPG::SavePicture> save;

	auto data_size = std::max(static_cast<int>(pictures.size()), GetDefaultNumberOfPictures());
	save.reserve(data_size);

	for (auto& pic: pictures) {
		save.push_back(pic.data);
	}

	// RPG_RT Save game data always has a constant number of pictures
	// depending on the engine version. We replicate this, unless we have even
	// more pictures than that.
	while (data_size > static_cast<int>(save.size())) {
		RPG::SavePicture data;
		data.ID = static_cast<int>(save.size());
		if (Player::IsRPG2k3E()) {
			data.frames = frame_counter;
		}
		save.push_back(std::move(data));
	}

	return save;
}

int Game_Pictures::GetDefaultNumberOfPictures() {
	if (Player::IsEnglish()) {
		return 1000;
	}
	else if (Player::IsMajorUpdatedVersion()) {
		return 50;
	}
	else if (Player::IsRPG2k3Legacy()) {
		return 40;
	}
	else if (Player::IsRPG2kLegacy()) {
		return 20;
	}
	return 0;
}

Game_Pictures::Picture& Game_Pictures::GetPicture(int id) {
	if (EP_UNLIKELY(id > static_cast<int>(pictures.size()))) {
		pictures.reserve(id);
		while (static_cast<int>(pictures.size()) < id) {
			pictures.emplace_back(pictures.size() + 1);
		}
	}
	return pictures[id - 1];
}

Game_Pictures::Picture* Game_Pictures::GetPicturePtr(int id) {
	return id <= static_cast<int>(pictures.size())
		? &pictures[id - 1] : nullptr;
}

void Game_Pictures::Picture::UpdateGraphics(bool is_battle) {
	if (!sprite || !sprite->GetBitmap() || data.name.empty()) {
		return;
	}

	// RPG Maker 2k3 1.12: Spritesheets
	if (Player::IsRPG2k3E()
			&& NumSpriteSheetFrames() > 1
			&& (data.spritesheet_frame != last_spritesheet_frame))
	{
		last_spritesheet_frame = data.spritesheet_frame;

		const int sw = sprite->GetBitmap()->GetWidth() / data.spritesheet_cols;
		const int sh = sprite->GetBitmap()->GetHeight() / data.spritesheet_rows;
		const int sx = sw * ((last_spritesheet_frame) % data.spritesheet_cols);
		const int sy = sh * ((last_spritesheet_frame) / data.spritesheet_cols % data.spritesheet_rows);

		sprite->SetSrcRect(Rect{ sx, sy, sw, sh });
	}

	int x = data.current_x;
	int y = data.current_y;
	if (data.flags.affected_by_shake) {
		x -= Main_Data::game_screen->GetShakeOffsetX();
		y -= Main_Data::game_screen->GetShakeOffsetY();
	}

	sprite->SetX(x);
	sprite->SetY(y);
	if (Player::IsMajorUpdatedVersion()) {
		// Battle Animations are above pictures
		int priority = 0;
		if (is_battle) {
			priority = Drawable::GetPriorityForBattleLayer(data.battle_layer);
		} else {
			priority = Drawable::GetPriorityForMapLayer(data.map_layer);
		}
		if (priority > 0) {
			sprite->SetZ(priority + z_mask + data.ID);
		}
	} else {
		// Battle Animations are below pictures
		sprite->SetZ(Priority_PictureOld + data.ID);
	}
	sprite->SetVisible(is_battle ? IsOnBattle() : IsOnMap());
	sprite->SetZoomX(data.current_magnify / 100.0);
	sprite->SetZoomY(data.current_magnify / 100.0);

	auto sr = sprite->GetSrcRect();
	sprite->SetOx(sr.width / 2);
	sprite->SetOy(sr.height / 2);

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
		sprite->SetFlashEffect(Main_Data::game_screen->GetFlashColor());
	}
}

void Game_Pictures::UpdateGraphics(bool is_battle) {
	for (auto& pic: pictures) {
		pic.UpdateGraphics(is_battle);
	}
}

void Game_Pictures::OnMapChange() {
	for (auto& pic: pictures) {
		if (pic.data.flags.erase_on_map_change) {
			pic.Erase();
		}
	}
}

void Game_Pictures::OnBattleEnd() {
	for (auto& pic: pictures) {
		if (pic.data.flags.erase_on_battle_end) {
			pic.Erase();
		}
	}
}

bool Game_Pictures::Picture::Show(const ShowParams& params) {
	needs_update = true;

	data.name = params.name;
	data.use_transparent_color = params.use_transparent_color;
	data.fixed_to_map = params.fixed_to_map;
	SetNonEffectParams(params, true);

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

	const auto num_frames = NumSpriteSheetFrames();

	// If an invalid frame is specified and no animation, skip loading picture data.
	if (num_frames > 0
			&& data.spritesheet_speed == 0
			&& (data.spritesheet_frame < 0 || data.spritesheet_frame >= num_frames))
	{
		if (sprite) {
			sprite->SetBitmap(nullptr);
		}
		return false;
	}

	return true;
}

void Game_Pictures::Show(int id, const ShowParams& params) {
	auto& pic = GetPicture(id);
	if (pic.Show(params)) {
		RequestPictureSprite(pic);
	}
}

void Game_Pictures::Picture::Move(const MoveParams& params) {
	const bool ignore_position = Player::IsLegacy() && data.fixed_to_map;

	SetNonEffectParams(params, !ignore_position);
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

void Game_Pictures::Move(int id, const MoveParams& params) {
	auto& pic = GetPicture(id);
	pic.Move(params);
}

void Game_Pictures::Picture::Erase() {
	request_id = {};
	data.name.clear();
	sprite.reset();
}

void Game_Pictures::Erase(int id) {
	auto* pic = GetPicturePtr(id);
	if (EP_LIKELY(pic)) {
		pic->Erase();
	}
}

void Game_Pictures::RequestPictureSprite(Picture& pic) {
	const auto& name = pic.data.name;
	if (name.empty()) return;

	FileRequestAsync* request = AsyncHandler::RequestFile("Picture", name);
	request->SetGraphicFile(true);

	int pic_id = pic.data.ID;

	pic.request_id = request->Bind([this, pic_id](FileRequestResult*) {
			OnPictureSpriteReady(pic_id);
			});
	request->Start();
}


void Game_Pictures::Picture::OnPictureSpriteReady() {
	auto bitmap = Cache::Picture(data.name, data.use_transparent_color);

	if (!sprite) {
		sprite.reset(new Sprite(Drawable::Flags::Shared));
	}
	sprite->SetBitmap(bitmap);
}

void Game_Pictures::OnPictureSpriteReady(int id) {
	auto* pic = GetPicturePtr(id);
	if (EP_LIKELY(pic)) {
		pic->OnPictureSpriteReady();
	}
}

void Game_Pictures::Picture::Update(bool is_battle) {
	if ((is_battle && !IsOnBattle()) || (!is_battle && !IsOnMap())) {
		return;
	}

	if (Player::IsRPG2k3E()) {
		++data.frames;
	}

	if (!needs_update) {
		return;
	}

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
	if (Player::IsRPG2k3E()
			&& data.spritesheet_speed > 0
			&& data.frames > data.spritesheet_speed)
	{
		data.frames = 1;
		data.spritesheet_frame = data.spritesheet_frame + 1;

		if (data.spritesheet_frame >= data.spritesheet_rows * data.spritesheet_cols) {
			data.spritesheet_frame = 0;
			if (data.spritesheet_play_once && !data.name.empty()) {
				Erase();
			}
		}
	}
}

void Game_Pictures::Update(bool is_battle) {
	++frame_counter;
	for (auto& pic: pictures) {
		pic.Update(is_battle);
	}
}

void Game_Pictures::Picture::SetNonEffectParams(const Params& params, bool set_positions) {
	if (set_positions) {
		data.finish_x = params.position_x;
		data.finish_y = params.position_y;
	}
	data.finish_magnify = params.magnify;
	data.finish_top_trans = params.top_trans;
	data.finish_bot_trans = params.bottom_trans;
	data.finish_red = params.red;
	data.finish_green = params.green;
	data.finish_blue = params.blue;
	data.finish_sat = params.saturation;
}

void Game_Pictures::Picture::SyncCurrentToFinish() {
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

inline int Game_Pictures::Picture::NumSpriteSheetFrames() const {
	return data.spritesheet_cols * data.spritesheet_rows;
}
