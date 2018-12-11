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
#include "game_battle.h"
#include "game_battler.h"
#include "game_screen.h"
#include "game_system.h"
#include "game_variables.h"
#include "main_data.h"
#include "output.h"
#include "utils.h"
#include <cmath>

static constexpr int kShakeContinuousTimeStart = 65535;

Game_Screen::Game_Screen() :
	data(Main_Data::game_data.screen)
{
	Reset(false);
}

void Game_Screen::CreatePicturesFromSave() {
	std::vector<RPG::SavePicture>& save_pics = Main_Data::game_data.pictures;

	pictures.resize(save_pics.size());

	for (int id = 1; id <= (int)save_pics.size(); ++id) {
		if (!save_pics[id - 1].name.empty()) {
			pictures[id - 1].reset(new Game_Picture(id));
		}
	}
}

void Game_Screen::Reset(bool is_load_savegame) {
	if (Main_Data::game_data.pictures.size() < pictures.size()) {
		pictures.resize(Main_Data::game_data.pictures.size());
	}
	for (auto& p : pictures) {
		if (p) {
			p->Erase(false);
		}
	}

	if (!is_load_savegame) {
		data.flash_red = 0;
		data.flash_green = 0;
		data.flash_blue = 0;
		data.flash_time_left = 0;
		data.flash_current_level = 0;
	}
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
}

Game_Picture* Game_Screen::GetPicture(int id) {
	if (id <= 0) {
		return NULL;
	}

	if (id > (int)pictures.size()) {
		// Some games use more pictures then RPG_RT officially supported
		Main_Data::game_data.pictures.resize(id);

		for (size_t i = 0; i < Main_Data::game_data.pictures.size(); ++i) {
			Main_Data::game_data.pictures[i].ID = i + 1;
		}

		pictures.resize(id);
	}
	std::unique_ptr<Game_Picture>& p = pictures[id - 1];
	if (!p)
		p.reset(new Game_Picture(id));
	return p.get();
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

void Game_Screen::FlashOnce(int r, int g, int b, int s, int tenths) {
	data.flash_red = r;
	data.flash_green = g;
	data.flash_blue = b;
	flash_sat = s;
	data.flash_current_level = s;

	if (tenths == 0) {
		// 0.0 duration case
		tenths = 1;
	}

	data.flash_time_left = tenths;
	flash_period = 0;
}

void Game_Screen::FlashBegin(int r, int g, int b, int s, int tenths) {
	FlashOnce(r, g, b, s, tenths);

	flash_period = data.flash_time_left;
	data.flash_continuous = true;
}

void Game_Screen::FlashEnd() {
	data.flash_time_left = 0;
	flash_period = 0;
	data.flash_continuous = false;
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
	data.shake_continuous = false;
}

void Game_Screen::SetWeatherEffect(int type, int strength) {
	// Some games call weather effects in a parallel process
	// This causes issues in the rendering (weather rendered too fast)
	if (data.weather != type ||
		data.weather_strength != strength) {
		StopWeather();
		data.weather = type;
		data.weather_strength = strength;
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
	snowflakes.clear();
}

void Game_Screen::InitSnowRain() {
	if (!snowflakes.empty())
		return;

	static const int num_snowflakes[3] = {50, 100, 150};

	for (int i = 0; i < num_snowflakes[data.weather_strength]; i++) {
		Snowflake f;
		f.x = (short) Utils::GetRandomNumber(0, 440);
		f.y = (uint8_t) Utils::GetRandomNumber(0, 255);
		f.life = (uint8_t) Utils::GetRandomNumber(0, 255);
		snowflakes.push_back(f);
	}
}

void Game_Screen::UpdateSnowRain(int speed) {
	std::vector<Snowflake>::iterator it;

	for (it = snowflakes.begin(); it != snowflakes.end(); ++it) {
		Snowflake& f = *it;
		f.y += (uint8_t)speed;
		f.life -= 5;
		if (f.life < 10)
			f.life = 255;
	}
}

void Game_Screen::Update() {
	if (data.tint_time_left > 0) {
		data.tint_current_red = interpolate(data.tint_time_left, data.tint_current_red, data.tint_finish_red);
		data.tint_current_green = interpolate(data.tint_time_left, data.tint_current_green, data.tint_finish_green);
		data.tint_current_blue = interpolate(data.tint_time_left, data.tint_current_blue, data.tint_finish_blue);
		data.tint_current_sat = interpolate(data.tint_time_left, data.tint_current_sat, data.tint_finish_sat);
		data.tint_time_left = data.tint_time_left - 1;
	}

	if (data.flash_time_left > 0) {
		data.flash_current_level = data.flash_current_level - (data.flash_current_level / data.flash_time_left);
		data.flash_time_left = data.flash_time_left - 1;
		if (data.flash_time_left <= 0) {
			data.flash_time_left = 0;
			if (data.flash_continuous) {
				data.flash_time_left = flash_period;
				data.flash_current_level = flash_sat;
			}
		}
	}

	if (data.shake_continuous || data.shake_time_left > 0) {
		--data.shake_time_left;
		if (data.shake_continuous || data.shake_time_left > 0) {
			if (data.shake_time_left < 0 && data.shake_continuous) {
				data.shake_time_left = kShakeContinuousTimeStart;
			}
			int amplitude = 1 + 2 * data.shake_strength;
			int newpos = amplitude * sin((data.shake_time_left * 4 * (data.shake_speed + 2)) % 256 * M_PI / 128);
			int cutoff = (data.shake_speed * amplitude / 8) + 1;

			data.shake_position = Utils::Clamp<int>(newpos, data.shake_position - cutoff, data.shake_position + cutoff);
		} else {
			data.shake_position = 0;
			data.shake_time_left = 0;
		}
	}

	for (const auto& picture : pictures) {
		if (picture) {
			picture->Update();
		}
	}

	if (!movie_filename.empty()) {
		/* update movie */
	}

	switch (data.weather) {
		case Weather_None:
			break;
		case Weather_Rain:
			InitSnowRain();
			UpdateSnowRain(4);
			break;
		case Weather_Snow:
			InitSnowRain();
			UpdateSnowRain(2);
			break;
		case Weather_Fog:
			break;
		case Weather_Sandstorm:
			break;
	}
}

Tone Game_Screen::GetTone() {
	return Tone((int) ((data.tint_current_red) * 128 / 100),
		(int) ((data.tint_current_green) * 128 / 100),
		(int) ((data.tint_current_blue) * 128 / 100),
		(int) ((data.tint_current_sat) * 128 / 100));
}

Color Game_Screen::GetFlash(int& current_level, int& time_left) {
	time_left = data.flash_time_left;
	current_level = data.flash_current_level / 31 * 255;
	return Color(data.flash_red * 255 / 31, data.flash_green * 255 / 31, data.flash_blue * 255 / 31, 255);
}

int Game_Screen::GetWeatherType() {
	return data.weather;
}

int Game_Screen::GetWeatherStrength() {
	return data.weather_strength;
}

const std::vector<Game_Screen::Snowflake>& Game_Screen::GetSnowflakes() {
	return snowflakes;
}
