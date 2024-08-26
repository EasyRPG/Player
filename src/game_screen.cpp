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
#include <cmath>
#include "bitmap.h"
#include <lcf/data.h>
#include "player.h"
#include "game_battle.h"
#include "game_battler.h"
#include "game_screen.h"
#include "game_system.h"
#include "game_variables.h"
#include "game_map.h"
#include "output.h"
#include "utils.h"
#include "options.h"
#include <lcf/reader_util.h>
#include "scene.h"
#include "weather.h"
#include "flash.h"
#include "shake.h"
#include "rand.h"

Game_Screen::Game_Screen()
{
}

Game_Screen::~Game_Screen() {
}

void Game_Screen::SetSaveData(lcf::rpg::SaveScreen screen)
{
	CancelBattleAnimation();

	data = std::move(screen);
}

void Game_Screen::InitGraphics() {
	weather = std::make_unique<Weather>();
	OnWeatherChanged();

	if (data.battleanim_active) {
		ShowBattleAnimation(data.battleanim_id,
				data.battleanim_target,
				data.battleanim_global,
				data.battleanim_frame);
	}
}

void Game_Screen::OnMapChange() {
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

	data.battleanim_active = false;
	animation.reset();
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
	FlashOnce(31, 10, 10, 20, 6);
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
	data.shake_time_left = Shake::kShakeContinuousTimeStart;
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

void Game_Screen::PlayMovie(std::string filename,
							int pos_x, int pos_y, int res_x, int res_y) {
	movie_filename = std::move(filename);
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
	int num_particles = Weather::GetMaxNumParticles(data.weather);

	InitParticles(num_particles);

	if (weather) {
		weather->OnWeatherChanged();
	}
}

void Game_Screen::InitParticles(int num_particles) {
	// RPG_RT initializes all particles on new game / load game.
	// We do it lazily instead. That way for games which don't use
	// weather effects, we never consume memory for those effects.
	auto sz = static_cast<int>(particles.size());

	if (num_particles <= sz) {
		return;
	}

	particles.resize(num_particles);

	for (int i = sz; i < num_particles; ++i) {
		auto& p = particles[i];
		// RPG_RT always initializes all particles to these values on startup.
		// This can cause minor visual glitches for the first few frames the
		// first time you start the sandstorm effect. We're bug compatible with RPG_RT.
		p.t = Rand::GetRandomNumber(0, 39);
		p.x = Rand::GetRandomNumber(0, GetPanLimitX() / 16 - 1);
		p.y = Rand::GetRandomNumber(0, GetPanLimitY() / 16 - 1);
	}
}

void Game_Screen::UpdateRain() {
	for (auto& p: particles) {
		if (p.t > 0) {
			--p.t;
			p.y += 4;
			p.x -= 1;
		} else if (Rand::PercentChance(10)) {
			p.t = 12;
			p.x = Rand::GetRandomNumber(0, GetPanLimitX() / 16 - 1);
			p.y = Rand::GetRandomNumber(0, GetPanLimitY() / 16 - 1);
		}
	}
}

void Game_Screen::UpdateSnow() {
	for (auto& p: particles) {
		if (p.t > 0) {
			--p.t;
			p.x -= Rand::GetRandomNumber(0, 1);
			p.y += Rand::GetRandomNumber(2, 3);
		} else if (Rand::PercentChance(5)) {
			p.t = 30;
			p.x = Rand::GetRandomNumber(0, GetPanLimitX() / 16 - 1);
			p.y = Rand::GetRandomNumber(0, GetPanLimitY() / 16 - 1);
		}
	}
}

void Game_Screen::UpdateFog() {
	++particles[0].x;
	++particles[1].x;
}

void Game_Screen::UpdateSandstorm() {
	// RPG_RT takes random numbers in the inclusive range [1, 127] and has a function
	// which takes [0, 255) -> [0, 2 * M_PI) and computes sin or cos. This epsilson
	// accounts for the range starting at 1 (not 0) and ending at 127 (not 128).

	constexpr auto epsilon = 1.0f / 128.0f;
	auto& rng = Rand::GetRNG();
	auto dist = std::uniform_real_distribution<float>(epsilon, M_PI - epsilon);

	UpdateFog();

	for (size_t i = 2; i < particles.size(); ++i) {
		auto& p = particles[i];
		if (p.t > 0) {
			--p.t;
			p.alpha += 2;
			p.x += static_cast<int>(p.vx);
			p.y += static_cast<int>(p.vy);
			p.vx += p.ax;
			p.vy += p.ay;
		} else if (Rand::PercentChance(10)) {
			p.t = 80;

			auto c = std::cos(dist(rng));
			auto s = std::sin(dist(rng));
			auto d = Rand::GetRandomNumber(16, 95);

			p.x = static_cast<int>(d * c * 2.0f) * Player::screen_width / 320 + Player::screen_width / 2;
			p.y = static_cast<int>(d * s) * Player::screen_height / 240;

			p.alpha = 180;
			p.vx = 0.0;
			p.vy = 0.0;
			p.ax = c * 2.0f * Player::screen_width / 320;
			p.ay = s * 2.0f * Player::screen_height / 240;
		}
	}
}

void Game_Screen::OnMapScrolled(int dx, int dy) {
	auto pan_limit_x = GetPanLimitX();
	auto pan_limit_y = GetPanLimitY();

	data.pan_x = (data.pan_x - dx + pan_limit_x) % pan_limit_x;
	data.pan_y = (data.pan_y - dy + pan_limit_y) % pan_limit_y;
}

void Game_Screen::UpdateScreenEffects() {
	if (data.tint_time_left > 0) {
		data.tint_current_red = interpolate(data.tint_time_left, data.tint_current_red, data.tint_finish_red);
		data.tint_current_green = interpolate(data.tint_time_left, data.tint_current_green, data.tint_finish_green);
		data.tint_current_blue = interpolate(data.tint_time_left, data.tint_current_blue, data.tint_finish_blue);
		data.tint_current_sat = interpolate(data.tint_time_left, data.tint_current_sat, data.tint_finish_sat);
		data.tint_time_left = data.tint_time_left - 1;
	}

	Flash::Update(data.flash_current_level,
			data.flash_time_left,
			data.flash_continuous,
			flash_period,
			flash_sat);

	Shake::Update(data.shake_position,
			data.shake_time_left,
			data.shake_strength,
			data.shake_speed,
			data.shake_continuous);
}

void Game_Screen::UpdateMovie() {
	if (!movie_filename.empty()) {
		/* update movie */
	}
}

void Game_Screen::UpdateWeather() {
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
			UpdateFog();
			break;
		case Weather_Sandstorm:
			UpdateSandstorm();
			break;
	}
}

void Game_Screen::Update() {
	UpdateScreenEffects();
	UpdateMovie();
	UpdateWeather();
	UpdateBattleAnimation();
}

int Game_Screen::ShowBattleAnimation(int animation_id, int target_id, bool global, int start_frame) {
	const lcf::rpg::Animation* anim = lcf::ReaderUtil::GetElement(lcf::Data::animations, animation_id);
	if (!anim) {
		Output::Warning("ShowBattleAnimation: Invalid battle animation ID {}", animation_id);
		return 0;
	}

	auto* chara = Game_Character::GetCharacter(target_id, target_id);
	if (!chara) {
		Output::Warning("ShowBattleAnimation: Invalid target event ID {}", target_id);
		CancelBattleAnimation();
		return 0;
	}

	data.battleanim_id = animation_id;
	data.battleanim_target = target_id;
	data.battleanim_global = global;
	data.battleanim_active = true;
	data.battleanim_frame = start_frame;

	animation.reset(new BattleAnimationMap(*anim, *chara, global));

	if (start_frame) {
		animation->SetFrame(start_frame);
	}

	return animation->GetFrames();
}

void Game_Screen::UpdateBattleAnimation() {
	if (animation) {
		if (!animation->IsDone()) {
			animation->Update();
			data.battleanim_frame = animation->GetFrame();
		}

		if (animation->IsDone() && !Game_Battle::IsBattleRunning()) {
			// FIXME: Lifetime is flawed but we need the animation in battle for
			// SE and flash. Delay destruction until back on the map.
			CancelBattleAnimation();
		}
	}
}

void Game_Screen::CancelBattleAnimation() {
	data.battleanim_frame = animation ?
		animation->GetFrames() : 0;
	data.battleanim_active = false;
	animation.reset();
}

void Game_Screen::UpdateUnderlyingEventReferences() {
	if (!IsBattleAnimationWaiting()) {
		return;
	}

	auto* chara = Game_Character::GetCharacter(data.battleanim_target, data.battleanim_target);
	if (!chara) {
		// Event was deleted
		CancelBattleAnimation();
	} else {
		animation->SetTarget(*chara);
	}
}
