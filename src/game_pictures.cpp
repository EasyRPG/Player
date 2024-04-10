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

#include <cmath>
#include "bitmap.h"
#include "options.h"
#include "cache.h"
#include "output.h"
#include "game_map.h"
#include "game_pictures.h"
#include "game_screen.h"
#include "game_windows.h"
#include "player.h"
#include "main_data.h"
#include "scene.h"
#include "drawable_mgr.h"
#include "sprite_picture.h"

static bool IsEmpty(const lcf::rpg::SavePicture& data, int frames) {
	lcf::rpg::SavePicture empty;
	empty.ID = data.ID;
	empty.frames = frames;

	return data == empty;
}

static bool IsEmpty(const lcf::rpg::SavePicture& data) {
	return IsEmpty(data, data.frames);
}

template <bool do_effect>
void SyncCurrentToFinish(lcf::rpg::SavePicture& data) {
	data.current_x = data.finish_x;
	data.current_y = data.finish_y;
	data.current_red = data.finish_red;
	data.current_green = data.finish_green;
	data.current_blue = data.finish_blue;
	data.current_sat = data.finish_sat;
	data.current_magnify = data.finish_magnify;
	data.current_top_trans = data.finish_top_trans;
	data.current_bot_trans = data.finish_bot_trans;
	if (do_effect) {
		data.current_effect_power = data.finish_effect_power;
	}
}

Game_Pictures::Picture::Picture(lcf::rpg::SavePicture save)
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

void Game_Pictures::SetSaveData(std::vector<lcf::rpg::SavePicture> save)
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

std::vector<lcf::rpg::SavePicture> Game_Pictures::GetSaveData() const {
	std::vector<lcf::rpg::SavePicture> save;

	auto data_size = std::max(static_cast<int>(pictures.size()), GetDefaultNumberOfPictures());
	save.reserve(data_size);

	for (auto& pic: pictures) {
		save.push_back(pic.data);
	}

	// RPG_RT Save game data always has a constant number of pictures
	// depending on the engine version. We replicate this, unless we have even
	// more pictures than that.
	while (data_size > static_cast<int>(save.size())) {
		lcf::rpg::SavePicture data;
		data.ID = static_cast<int>(save.size()) + 1;
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
		if (Player::IsPatchDynRpg() && Player::IsRPG2k3()) {
			// DynRPG only exists for RPG Maker 2003 1.08
			return 2000;
		}
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
	if (data.effect_mode == lcf::rpg::SavePicture::Effect_none) {
		// params.effect_power seems to contain garbage here
		data.finish_effect_power = 0.0;
	} else {
		data.finish_effect_power = params.effect_power;
	}

	SyncCurrentToFinish<true>(data);
	data.start_x = data.current_x;
	data.start_y = data.current_y;
	if (data.effect_mode == lcf::rpg::SavePicture::Effect_maniac_fixed_angle) {
		data.current_rotation = data.finish_effect_power;
	} else {
		data.current_rotation = 0.0;
	}
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

	if (data.map_layer == 0 && data.battle_layer == 0) {
		data.map_layer = 7;
	}

	data.flags.erase_on_map_change = (params.flags & 1) == 1;
	data.flags.erase_on_battle_end = (params.flags & 2) == 2;
	data.flags.affected_by_tint = (params.flags & 16) == 16;
	data.flags.affected_by_flash = (params.flags & 32) == 32;
	data.flags.affected_by_shake = (params.flags & 64) == 64;

	const auto num_frames = NumSpriteSheetFrames();

	bool result = true;

	// If an invalid frame is specified and no animation, skip loading picture data.
	if (num_frames > 0
			&& data.spritesheet_speed == 0
			&& (data.spritesheet_frame < 0 || data.spritesheet_frame >= num_frames))
	{
		if (sprite) {
			sprite->SetBitmap(nullptr);
		}
		result = false;
	}

	// Extensions
	data.easyrpg_flip = params.flip_x ? lcf::rpg::SavePicture::EasyRpgFlip_x : 0;
	data.easyrpg_flip |= params.flip_y ? lcf::rpg::SavePicture::EasyRpgFlip_y : 0;
	data.easyrpg_blend_mode = params.blend_mode;
	data.easyrpg_type = lcf::rpg::SavePicture::EasyRpgType_default;

	// Not saved as the coordinate system is directly transformed to "center"
	origin = params.origin;

	return result;
}

bool Game_Pictures::Show(int id, const ShowParams& params) {
	auto& pic = GetPicture(id);
	if (pic.Show(params)) {
		if (pic.sprite && !pic.data.name.empty()) {
			// When the name is empty the current image buffer is reused by ShowPicture command (Used by Yume2kki)
			// In all other cases hide the current image until replaced while doing an Async load
			pic.sprite->SetVisible(false);
		}
		RequestPictureSprite(pic);
		return true;
	}
	return false;
}

void Game_Pictures::Picture::Move(const MoveParams& params) {
	const bool ignore_position = Player::IsLegacy() && data.fixed_to_map;

	SetNonEffectParams(params, !ignore_position);
	if (params.duration < 0) {
		data.time_left = -params.duration;
	} else {
		data.time_left = params.duration * DEFAULT_FPS / 10;
	}

	// Not saved as the coordinate system is directly transformed to "center"
	origin = params.origin;
	ApplyOrigin(true);

	// Note that data.effect_mode doesn't necessarily reflect the
	// last effect set. Possible states are:
	//
	// * effect_mode == lcf::rpg::SavePicture::Effect_none && finish_effect_power == 0
	//   Picture has not had an effect set since Show.
	// * effect_mode == lcf::rpg::SavePicture::Effect_none && finish_effect_power != 0
	//   Picture was set to no effect; previously, it was rotating.
	// * effect_mode == lcf::rpg::SavePicture::Effect_wave && finish_effect_power == 0
	//   Picture was set to no effect; previously, it was wavering.
	// * effect_mode == lcf::rpg::SavePicture::Effect_rotation
	//   Picture was set to rotate.
	// * effect_mode == lcf::rpg::SavePicture::Effect_wave && finish_effect_power != 0
	//   Picture was set to waver.

	bool started_with_no_effect =
		data.effect_mode == lcf::rpg::SavePicture::Effect_none && data.finish_effect_power == 0.0;
	if (Player::IsRPG2k() && started_with_no_effect) {
		// Possibly a bug(?) in RM2k: if Show Picture command has no
		// effect, a Move Picture command cannot add one
		return;
	}

	if (data.effect_mode == lcf::rpg::SavePicture::Effect_none && params.effect_mode == lcf::rpg::SavePicture::Effect_none) {
		// Nothing to do
	} else if (data.effect_mode == params.effect_mode) {
		data.finish_effect_power = params.effect_power;
	} else if (data.effect_mode == lcf::rpg::SavePicture::Effect_rotation && params.effect_mode == lcf::rpg::SavePicture::Effect_none) {
		data.effect_mode = lcf::rpg::SavePicture::Effect_none;
	} else if (data.effect_mode == lcf::rpg::SavePicture::Effect_wave && params.effect_mode == lcf::rpg::SavePicture::Effect_none) {
		data.finish_effect_power = 0;
	} else {
		data.effect_mode = params.effect_mode;
		data.current_effect_power = params.effect_power;
		data.finish_effect_power = params.effect_power;
	}

	data.easyrpg_flip = params.flip_x ? lcf::rpg::SavePicture::EasyRpgFlip_x : 0;
	data.easyrpg_flip |= params.flip_y ? lcf::rpg::SavePicture::EasyRpgFlip_y : 0;
	data.easyrpg_blend_mode = params.blend_mode;
}

void Game_Pictures::Move(int id, const MoveParams& params) {
	auto& pic = GetPicture(id);
	pic.Move(params);
}

void Game_Pictures::Picture::Erase() {
	request_id = {};
	data.name.clear();
	if (sprite) {
		sprite->SetBitmap(nullptr);
	}
	if (IsWindowAttached()) {
		data.easyrpg_type = lcf::rpg::SavePicture::EasyRpgType_default;
		Main_Data::game_windows->Erase(data.ID);
	}
}

void Game_Pictures::Erase(int id) {
	auto* pic = GetPicturePtr(id);
	if (EP_LIKELY(pic)) {
		pic->Erase();
	}
}

void Game_Pictures::EraseAll() {
	for (auto& pic: pictures) {
		pic.Erase();
	}
}

bool Game_Pictures::Picture::Exists() const {
	// Incompatible with the Yume2kki edge-case that uses empty filenames
	return !data.name.empty();
}

void Game_Pictures::Picture::CreateSprite() {
	if (!sprite) {
		sprite = std::make_unique<Sprite_Picture>(data.ID, Drawable::Flags::Shared);
	}
}

bool Game_Pictures::Picture::IsRequestPending() const {
	return request_id != nullptr;
}

void Game_Pictures::Picture::MakeRequestImportant() const {
	FileRequestAsync* request = AsyncHandler::RequestFile("Picture", data.name);
	request->SetImportantFile(true);
}

void Game_Pictures::RequestPictureSprite(Picture& pic) {
	const auto& name = pic.data.name;
	if (name.empty()) {
		return;
	}

	FileRequestAsync* request = AsyncHandler::RequestFile("Picture", name);
	request->SetGraphicFile(true);
	pic.request_id = request->Bind(&Game_Pictures::OnPictureSpriteReady, this, pic.data.ID);
	request->Start();
}

void Game_Pictures::Picture::OnPictureSpriteReady() {
	auto bitmap = Cache::Picture(data.name, data.use_transparent_color);

	sprite->SetBitmap(bitmap);
	sprite->OnPictureShow();
	sprite->SetVisible(true);

	ApplyOrigin(false);
}

void Game_Pictures::OnPictureSpriteReady(FileRequestResult*, int id) {
	auto* pic = GetPicturePtr(id);
	if (EP_LIKELY(pic)) {
		pic->request_id = nullptr;
		pic->CreateSprite();
		pic->OnPictureSpriteReady();
	}
}

void Game_Pictures::Picture::ApplyOrigin(bool is_move) {
	if (origin == 0 || !sprite || !sprite->GetBitmap()) {
		return;
	}

	double x;
	double y;

	if (is_move) {
		x = data.finish_x;
		y = data.finish_y;
	} else {
		x = data.current_x;
		y = data.current_y;
	}

	double width = sprite->GetFrameWidth();
	double height = sprite->GetFrameHeight();

	switch (origin) {
		case 1:
			// Top-Left
			x += width / 2;
			y += height / 2;
			break;
		case 2:
			// Bottom-Left
			x += (width / 2);
			y -= (height / 2);
			break;
		case 3:
			// Top-Right
			x -= (width / 2);
			y += (height / 2);
			break;
		case 4:
			// Bottom-Right
			x -= (width / 2);
			y -= (height / 2);
			break;
		case 5:
			// Top
			y += (height / 2);
			break;
		case 6:
			// Bottom
			y -= (height / 2);
			break;
		case 7:
			// Left
			x += (width / 2);
			break;
		case 8:
			// Right
			x -= (width / 2);
			break;
	}

	if (!is_move) {
		data.current_x = x;
		data.current_y = y;
		data.start_x = x;
		data.start_y = y;
	}
	data.finish_x = x;
	data.finish_y = y;
}

void Game_Pictures::Picture::OnMapScrolled(int dx16, int dy16) {
	if (data.fixed_to_map && IsOnMap()) {
		// Instead of modifying the Ox/Oy offset the real position is altered
		// based on map scroll because of savegame compatibility with RPG_RT

		auto dx = static_cast<double>(dx16) / TILE_SIZE;

		data.finish_x = data.finish_x - dx;
		data.current_x = data.current_x - dx;
		data.start_x = data.start_x - dx;

		auto dy = static_cast<double>(dy16) / TILE_SIZE;

		data.finish_y = data.finish_y - dy;
		data.current_y = data.current_y - dy;
		data.start_y = data.start_y - dy;
	}
}

void Game_Pictures::OnMapScrolled(int dx, int dy) {
	for (auto& pic: pictures) {
		pic.OnMapScrolled(dx, dy);
	}
}

void Game_Pictures::Picture::AttachWindow(const Window_Base& window) {
	data.easyrpg_type = lcf::rpg::SavePicture::EasyRpgType_window;

	CreateSprite();

	sprite->SetBitmap(std::make_shared<Bitmap>(window.GetWidth(), window.GetHeight(), data.use_transparent_color));
	sprite->OnPictureShow();
	sprite->SetVisible(true);

	ApplyOrigin(false);
}

bool Game_Pictures::Picture::IsWindowAttached() const {
	return data.easyrpg_type == lcf::rpg::SavePicture::EasyRpgType_window;
}

void Game_Pictures::Picture::Update(bool is_battle) {
	if ((is_battle && !IsOnBattle()) || (!is_battle && !IsOnMap())) {
		return;
	}

	if (Player::IsRPG2k3ECommands()) {
		++data.frames;
	}

	if (!needs_update) {
		return;
	}

	if (data.time_left > 0) {
		--data.time_left;
	}

	auto interpolate = [dt=static_cast<double>(data.time_left + 1)](double current, double finish) {
		return (finish - current) / dt + current;
	};

	if (data.time_left <= 0) {
		SyncCurrentToFinish<false>(data);
	} else {
		data.current_x = interpolate(data.current_x, data.finish_x);
		data.current_y = interpolate(data.current_y, data.finish_y);
		data.current_red = interpolate(data.current_red, data.finish_red);
		data.current_green = interpolate(data.current_green, data.finish_green);
		data.current_blue = interpolate(data.current_blue, data.finish_blue);
		data.current_sat = interpolate(data.current_sat, data.finish_sat);
		data.current_magnify = interpolate(data.current_magnify, data.finish_magnify);
		data.current_top_trans = interpolate(data.current_top_trans, data.finish_top_trans);
		data.current_bot_trans = interpolate(data.current_bot_trans, data.finish_bot_trans);
	}

	// When a move picture disables rotation effect, we continue rotating
	// until one full revolution is done. There is a bug in RPG_RT where this
	// only happens when the current rotation and power is positive. We emulate this for now.
	if (data.effect_mode == lcf::rpg::SavePicture::Effect_none && data.current_effect_power > 0) {
		// RPG_RT calculates this and compares it against remaining time.
		const auto et = 256 / static_cast<int>(data.current_effect_power);

		if (et < data.time_left || data.current_rotation > 0.0) {
			data.current_rotation = std::fmod(data.current_rotation, 256.0);
			data.current_rotation += data.current_effect_power;
			if (et >= data.time_left && data.current_rotation >= 256.0) {
				data.current_rotation = 0;
			}
		}
	}

	if (data.effect_mode != lcf::rpg::SavePicture::Effect_none) {
		data.current_effect_power = interpolate(data.current_effect_power, data.finish_effect_power);
	}

	// Update rotation
	if (data.effect_mode == lcf::rpg::SavePicture::Effect_rotation) {
		data.current_rotation += data.current_effect_power;
	}

	// Update waver phase
	if (data.effect_mode == lcf::rpg::SavePicture::Effect_wave) {
		data.current_waver += 8;
	}

	// Update fixed angle
	if (data.effect_mode == lcf::rpg::SavePicture::Effect_maniac_fixed_angle) {
		data.current_rotation = data.current_effect_power;
	}

	// RPG Maker 2k3 1.12: Animated spritesheets
	if (Player::IsRPG2k3ECommands()
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

Game_Pictures::ShowParams Game_Pictures::Picture::GetShowParams() const {
	Game_Pictures::ShowParams params;
	params.position_x = static_cast<int>(data.finish_x);
	params.position_y = static_cast<int>(data.finish_y);
	params.magnify = data.finish_magnify;
	params.top_trans = data.finish_top_trans;
	params.bottom_trans = data.finish_bot_trans;
	params.red = data.finish_red;
	params.green = data.finish_green;
	params.blue = data.finish_blue;
	params.saturation = data.finish_sat;
	params.effect_mode = data.effect_mode;
	params.effect_power = data.finish_effect_power;
	params.name = data.name;
	params.spritesheet_cols = data.spritesheet_cols;
	params.spritesheet_rows = data.spritesheet_rows;
	params.spritesheet_frame = data.spritesheet_frame;
	params.spritesheet_speed = data.spritesheet_speed;
	params.map_layer = data.map_layer;
	params.battle_layer = data.battle_layer;
	for (size_t i = 0; i < data.flags.flags.size(); ++i) {
		params.flags |= data.flags.flags[i] << i;
	}
	params.spritesheet_play_once = data.spritesheet_play_once;
	params.use_transparent_color = data.use_transparent_color;
	params.fixed_to_map = data.fixed_to_map;
	return params;
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


int Game_Pictures::Picture::NumSpriteSheetFrames() const {
	return data.spritesheet_cols * data.spritesheet_rows;
}
