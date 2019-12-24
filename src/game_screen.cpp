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
#define _USE_MATH_DEFINES
#include "bitmap.h"
#include "data.h"
#include "player.h"
#include "game_battle.h"
#include "game_battler.h"
#include "game_screen.h"
#include "game_system.h"
#include "game_variables.h"
#include "game_map.h"
#include "main_data.h"
#include "output.h"
#include "utils.h"
#include "options.h"
#include "reader_util.h"
#include "scene.h"
#include "weather.h"
#include <cmath>

static constexpr int kShakeContinuousTimeStart = 65535;

static int GetDefaultNumberOfPictures() {
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

Game_Screen::Game_Screen() :
	data(Main_Data::game_data.screen)
{
	Reset();
}

Game_Screen::~Game_Screen() {}

void Game_Screen::SetupNewGame() {
	Reset();
	weather = std::make_unique<Weather>();
	OnWeatherChanged();

	// Pre-allocate pictures depending on detected game version.
	// This makes our savegames match RPG_RT.
	PreallocatePictureData(GetDefaultNumberOfPictures());
}

void Game_Screen::SetupFromSave() {
	CreatePicturesFromSave();
	weather = std::make_unique<Weather>();
	OnWeatherChanged();

	if (Main_Data::game_data.screen.battleanim_active) {
		ShowBattleAnimation(Main_Data::game_data.screen.battleanim_id,
				Main_Data::game_data.screen.battleanim_target,
				Main_Data::game_data.screen.battleanim_global,
				Main_Data::game_data.screen.battleanim_frame);
	}
}

void Game_Screen::CreatePicturesFromSave() {
	const auto& save_pics = Main_Data::game_data.pictures;

	pictures.clear();
	pictures.reserve(save_pics.size());

	while (pictures.size() < save_pics.size()) {
		pictures.emplace_back(pictures.size() + 1);
	}
}

void Game_Screen::Reset() {
	for (auto& pic : pictures) {
		pic.Erase(false);
	}

	data.flash_red = 0;
	data.flash_green = 0;
	data.flash_blue = 0;
	data.flash_time_left = 0;
	data.flash_current_level = 0;
	flash_sat = 0;
	flash_period = 0;

	if (data.tint_current_red < 0 ||
		data.tint_current_green < 0 ||
		data.tint_current_blue < 0 ||
		data.tint_current_sat < 0) {
		data.tint_current_red = 100;
		data.tint_current_green = 100;
		data.tint_current_blue = 100;
		data.tint_current_sat = 100;
	}

	movie_filename = "";
	movie_pos_x = 0;
	movie_pos_y = 0;
	movie_res_x = 0;
	movie_res_y = 0;

	animation.reset();
}

void Game_Screen::PreallocatePictureData(int id) {
	if (id <= (int)pictures.size()) {
		return;
	}

	const auto old_size = Main_Data::game_data.pictures.size();

	// Some games use more pictures then RPG_RT officially supported
	Main_Data::game_data.pictures.resize(id);

	for (auto i = old_size; i < Main_Data::game_data.pictures.size(); ++i) {
		Main_Data::game_data.pictures[i].ID = i + 1;
	}

	pictures.reserve(id);
	while (pictures.size() < id) {
		pictures.emplace_back(pictures.size() + 1);
	}
}

Game_Picture* Game_Screen::GetPicture(int id) {
	if (id <= 0) {
		return NULL;
	}

	PreallocatePictureData(id);

	return &pictures[id - 1];
}

void Game_Screen::TintScreen(int r, int g, int b, int s, int tenths) {
	data.tint_finish_red = r;
	data.tint_finish_green = g;
	data.tint_finish_blue = b;
	data.tint_finish_sat = s;

	data.tint_time_left = tenths;

	if (data.tint_time_left == 0) {
		data.tint_current_red = data.tint_finish_red;
		data.tint_current_green = data.tint_finish_green;
		data.tint_current_blue = data.tint_finish_blue;
		data.tint_current_sat = data.tint_finish_sat;
	}
}

void Game_Screen::FlashOnce(int r, int g, int b, int s, int frames) {
	data.flash_red = r;
	data.flash_green = g;
	data.flash_blue = b;
	flash_sat = s;
	data.flash_current_level = s;
	data.flash_time_left = frames;
	data.flash_continuous = false;
	flash_period = 0;
}

void Game_Screen::FlashBegin(int r, int g, int b, int s, int frames) {
	FlashOnce(r, g, b, s, frames);

	flash_period = frames;
	data.flash_continuous = true;
}

void Game_Screen::FlashEnd() {
	data.flash_time_left = 0;
	data.flash_current_level = 0;
	flash_period = 0;
	data.flash_continuous = false;
}

void Game_Screen::FlashMapStepDamage() {
	Main_Data::game_screen->FlashOnce(31, 10, 10, 20, 6);
}

void Game_Screen::ShakeOnce(int power, int speed, int tenths) {
	data.shake_strength = power;
	data.shake_speed = speed;
	data.shake_time_left = tenths;
	data.shake_continuous = false;
	// Shake position is not reset in RPG_RT, so that multiple shakes
	// which interrupt each other flow smoothly.
}

void Game_Screen::ShakeBegin(int power, int speed) {
	data.shake_strength = power;
	data.shake_speed = speed;
	data.shake_time_left = kShakeContinuousTimeStart;
	data.shake_continuous = true;
	// Shake position is not reset in RPG_RT, so that multiple shakes
	// which interrupt each other flow smoothly.
}

void Game_Screen::ShakeEnd() {
	data.shake_position = 0;
	data.shake_time_left = 0;
	// RPG_RT does not turn off the continuous shake flag when shake is disabled.
}

void Game_Screen::SetWeatherEffect(int type, int strength) {
	// Some games call weather effects in a parallel process
	// This causes issues in the rendering (weather rendered too fast)
	if (data.weather != type ||
		data.weather_strength != strength) {
		data.weather = type;
		data.weather_strength = strength;
		OnWeatherChanged();
	}
}

void Game_Screen::PlayMovie(const std::string& filename,
							int pos_x, int pos_y, int res_x, int res_y) {
	movie_filename = filename;
	movie_pos_x = pos_x;
	movie_pos_y = pos_y;
	movie_res_x = res_x;
	movie_res_y = res_y;
}

static double interpolate(double d, double x0, double x1)
{
	return (x0 * (d - 1) + x1) / d;
}

void Game_Screen::StopWeather() {
	data.weather = Weather_None;
	OnWeatherChanged();
}

void Game_Screen::OnWeatherChanged() {
	particles.clear();

	switch (data.weather) {
		case Weather_Rain:
			InitRainSnow(80);
			break;
		case Weather_Snow:
			InitRainSnow(255);
			break;
		default:
			break;
	}

	if (weather) {
		weather->OnWeatherChanged();
	}
}

void Game_Screen::InitRainSnow(int lifetime) {
	const auto num_particles = std::min(1 << (data.weather_strength + 4), 128);
	auto rect = GetScreenEffectsRect();

	particles.resize(num_particles);
	for (auto& p: particles) {
		p.x = Utils::GetRandomNumber(0, rect.width);
		p.y = Utils::GetRandomNumber(0, rect.height);
		p.life = Utils::GetRandomNumber(0, lifetime);
	}
}

void Game_Screen::UpdateRain() {
	auto rect = GetScreenEffectsRect();

	for (auto& p: particles) {
		if (p.life > 0) {
			p.y += 4;
			p.x -= 1;
			p.life -= 8;
		} else {
			p.x = Utils::GetRandomNumber(0, rect.width);
			p.y = Utils::GetRandomNumber(0, rect.height);
			p.life = 80;
		}
	}
}

void Game_Screen::UpdateSnow() {
	auto rect = GetScreenEffectsRect();

	for (auto& p: particles) {
		if (p.life > 0) {
			p.y += Utils::GetRandomNumber(2, 3);
			p.x -= Utils::GetRandomNumber(0, 1);
			p.life -= 8;
		} else {
			p.x = Utils::GetRandomNumber(0, rect.width);
			p.y = Utils::GetRandomNumber(0, rect.height);
			p.life = 255;
		}
	}
}

int Game_Screen::AnimateShake(int strength, int speed, int time_left, int position) {
	int amplitude = 1 + 2 * strength;
	int newpos = amplitude * sin((time_left * 4 * (speed + 2)) % 256 * M_PI / 128);
	int cutoff = (speed * amplitude / 8) + 1;

	return Utils::Clamp<int>(newpos, position - cutoff, position + cutoff);
}

void Game_Screen::Update() {
	constexpr auto pan_limit_x = GetPanLimitX();
	constexpr auto pan_limit_y = GetPanLimitY();

	data.pan_x = (data.pan_x - Game_Map::GetScrolledRight() + pan_limit_x) % pan_limit_x;
	data.pan_y = (data.pan_y - Game_Map::GetScrolledDown() + pan_limit_y) % pan_limit_y;

	if (data.tint_time_left > 0) {
		data.tint_current_red = interpolate(data.tint_time_left, data.tint_current_red, data.tint_finish_red);
		data.tint_current_green = interpolate(data.tint_time_left, data.tint_current_green, data.tint_finish_green);
		data.tint_current_blue = interpolate(data.tint_time_left, data.tint_current_blue, data.tint_finish_blue);
		data.tint_current_sat = interpolate(data.tint_time_left, data.tint_current_sat, data.tint_finish_sat);
		data.tint_time_left = data.tint_time_left - 1;
	}

	if (data.flash_current_level > 0 || data.flash_continuous) {
		if (data.flash_time_left > 0) {
			data.flash_current_level = data.flash_current_level - (data.flash_current_level / data.flash_time_left);
			--data.flash_time_left;
		}
		if (data.flash_time_left <= 0) {
			data.flash_time_left = 0;
			data.flash_current_level = 0;
			if (data.flash_continuous) {
				data.flash_time_left = flash_period;
				data.flash_current_level = flash_sat;
			}
		}
	}

	if (data.shake_time_left > 0) {
		--data.shake_time_left;

		// This fixes a bug in RPG_RT where continuous shake would actually stop after
		// 18m12s of gameplay.
		if (data.shake_time_left <= 0 && data.shake_continuous) {
			data.shake_time_left = kShakeContinuousTimeStart;
		}

		if (data.shake_time_left > 0) {
			data.shake_position = AnimateShake(data.shake_strength, data.shake_speed, data.shake_time_left, data.shake_position);
		} else {
			data.shake_position = 0;
			data.shake_time_left = 0;
		}
	}

	for (auto& picture : pictures) {
		picture.Update();
	}

	if (!movie_filename.empty()) {
		/* update movie */
	}

	switch (data.weather) {
		case Weather_None:
			break;
		case Weather_Rain:
			UpdateRain();
			break;
		case Weather_Snow:
			UpdateSnow();
			break;
		case Weather_Fog:
			break;
		case Weather_Sandstorm:
			break;
	}

	UpdateBattleAnimation();
}

int Game_Screen::ShowBattleAnimation(int animation_id, int target_id, bool global, int start_frame) {
	const RPG::Animation* anim = ReaderUtil::GetElement(Data::animations, animation_id);
	if (!anim) {
		Output::Warning("ShowBattleAnimation: Invalid battle animation ID %d", animation_id);
		return 0;
	}

	Main_Data::game_data.screen.battleanim_id = animation_id;
	Main_Data::game_data.screen.battleanim_target = target_id;
	Main_Data::game_data.screen.battleanim_global = global;
	Main_Data::game_data.screen.battleanim_active = true;
	Main_Data::game_data.screen.battleanim_frame = start_frame;

	Game_Character* chara = Game_Character::GetCharacter(target_id, target_id);

	if (chara) {
		animation.reset(new BattleAnimationMap(*anim, *chara, global));
	}

	if (start_frame) {
		animation->SetFrame(start_frame);
	}

	return animation->GetFrames();
}

void Game_Screen::UpdateBattleAnimation() {
	if (animation) {
		animation->Update();
		Main_Data::game_data.screen.battleanim_frame = animation->GetFrame();
		if (animation->IsDone()) {
			CancelBattleAnimation();
		}
	}
}

void Game_Screen::CancelBattleAnimation() {
	Main_Data::game_data.screen.battleanim_frame = animation ?
		animation->GetFrames() : 0;
	Main_Data::game_data.screen.battleanim_active = false;
	animation.reset();
}

void Game_Screen::UpdateGraphics() {
	for (auto& picture: pictures) {
		picture.UpdateSprite();
	}
	weather->SetTone(GetTone());
}
