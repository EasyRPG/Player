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
#include <cstdlib>
#include <ciso646>
#include "data.h"
#include "options.h"
#include "main_data.h"
#include "game_screen.h"
#include "bitmap.h"
#include "screen.h"

Game_Screen::Game_Screen() :
	data(Main_Data::game_data.screen)
{
	Reset();
}

void Game_Screen::Reset()
{
	pictures.clear();
	pictures.resize(50);

	data.tint_current_red = 100;
	data.tint_current_green = 100;
	data.tint_current_blue = 100;
	data.tint_current_sat = 100;

	data.tint_finish_red = -1;
	data.tint_finish_green = -1;
	data.tint_finish_blue = -1;
	data.tint_finish_sat = -1;
	data.tint_time_left = -1;

	data.flash_red = -1;
	data.flash_green = -1;
	data.flash_blue = -1;
	flash_sat = 0;
	data.flash_time_left = -1;
	flash_period = 0;

	data.shake_strength = -1;
	data.shake_speed = -1;
	data.shake_time_left = -1;
	data.shake_position = 0;
	data.shake_continuous = false;
	shake_direction = 0;

	data.weather = 0;
	data.weather_strength = 0;

	movie_filename = "";
	movie_pos_x = 0;
	movie_pos_y = 0;
	movie_res_x = 0;
	movie_res_y = 0;

	snowflakes.clear();
	StopWeather();

	screen.reset(new Screen(data));
}

Picture* Game_Screen::GetPicture(int id) {
	EASYRPG_SHARED_PTR<Picture>& p = pictures[id - 1];
	if (!p)
		p.reset(new Picture(id));
	return p.get();
}

void Game_Screen::TintScreen(int r, int g, int b, int s, int tenths) {
	data.tint_finish_red = r;
	data.tint_finish_green = g;
	data.tint_finish_blue = b;
	data.tint_finish_sat = s;

	data.tint_time_left = tenths * DEFAULT_FPS / 10;

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

	data.flash_time_left = tenths * DEFAULT_FPS / 10;
	flash_period = 0;
}

void Game_Screen::FlashBegin(int r, int g, int b, int s, int tenths) {
	data.flash_red = r;
	data.flash_green = g;
	data.flash_blue = b;
	flash_sat = s;
	data.flash_current_level = s;

	data.flash_time_left = tenths * DEFAULT_FPS / 10;
	flash_period = data.flash_time_left;
}

void Game_Screen::FlashEnd() {
	data.flash_time_left = 0;
	flash_period = 0;
}

void Game_Screen::ShakeOnce(int power, int speed, int tenths) {
	data.shake_strength = power;
	data.shake_speed = speed;
	data.shake_time_left = tenths * DEFAULT_FPS / 10;
	data.shake_position = 0;
	data.shake_continuous = false;
}

void Game_Screen::ShakeBegin(int power, int speed) {
	data.shake_strength = power;
	data.shake_speed = speed;
	data.shake_time_left = 0;
	data.shake_position = 0;
	data.shake_continuous = true;
}

void Game_Screen::ShakeEnd() {
	data.shake_time_left = 0;
	data.shake_continuous = false;
}

void Game_Screen::Weather(int type, int strength) {
	data.weather = type;
	data.weather_strength = strength;
	StopWeather();
	if (data.weather != Weather_None)
		InitWeather();
}

void Game_Screen::PlayMovie(const std::string& filename,
							int pos_x, int pos_y, int res_x, int res_y) {
	movie_filename = filename;
	movie_pos_x = pos_x;
	movie_pos_y = pos_y;
	movie_res_x = res_x;
	movie_res_y = res_y;
}

void Game_Screen::ShowBattleAnimation(int animation_id, int target_id, bool global) {
	data.battleanim_id = animation_id;
	data.battleanim_target = target_id;
	data.battleanim_global = global;

	Game_Character* target = Game_Character::GetCharacter(target_id, target_id);

	animation.reset(new BattleAnimation(target->GetScreenX(), target->GetScreenY(),
										&Data::animations[animation_id - 1]));
	animation->SetVisible(true);
	// FIXME: target
	// FIXME: global
}

bool Game_Screen::IsBattleAnimationWaiting() const {
	return bool(animation);
}

static double interpolate(double d, double x0, double x1)
{
	return (x0 * (d - 1) + x1) / d;
}

static const uint8_t snow_image[] = {
	0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00,
	0x0d, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00,
	0x00, 0x04, 0x02, 0x03, 0x00, 0x00, 0x00, 0xd4, 0x9f, 0x76, 0xed,
	0x00, 0x00, 0x00, 0x09, 0x50, 0x4c, 0x54, 0x45, 0x00, 0x00, 0x00,
	0xc0, 0xc0, 0xc0, 0xff, 0xff, 0xff, 0x0d, 0x6d, 0xd7, 0xbb, 0x00,
	0x00, 0x00, 0x01, 0x74, 0x52, 0x4e, 0x53, 0x00, 0x40, 0xe6, 0xd8,
	0x66, 0x00, 0x00, 0x00, 0x0e, 0x49, 0x44, 0x41, 0x54, 0x08, 0xd7,
	0x63, 0x10, 0x61, 0xc8, 0x04, 0x42, 0x11, 0x00, 0x03, 0xf0, 0x00,
	0xfb, 0xb6, 0xa8, 0xf1, 0xda, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45,
	0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};

static const uint8_t rain_image[] = {
	0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00,
	0x0d, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00,
	0x00, 0x10, 0x01, 0x03, 0x00, 0x00, 0x00, 0x11, 0x44, 0xac, 0x3e,
	0x00, 0x00, 0x00, 0x06, 0x50, 0x4c, 0x54, 0x45, 0x00, 0x00, 0x00,
	0xc0, 0xc0, 0xc0, 0x64, 0x56, 0x3a, 0x71, 0x00, 0x00, 0x00, 0x01,
	0x74, 0x52, 0x4e, 0x53, 0x00, 0x40, 0xe6, 0xd8, 0x66, 0x00, 0x00,
	0x00, 0x1f, 0x49, 0x44, 0x41, 0x54, 0x08, 0xd7, 0x63, 0x60, 0x64,
	0x60, 0x64, 0x60, 0x02, 0x42, 0x16, 0x20, 0xe4, 0x00, 0x42, 0x01,
	0x20, 0x54, 0x00, 0x42, 0x07, 0x20, 0x6c, 0x60, 0x68, 0x00, 0x00,
	0x0b, 0xd4, 0x01, 0xff, 0xed, 0x11, 0x33, 0x32, 0x00, 0x00, 0x00,
	0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};

void Game_Screen::InitWeather() {
	if (not weather_plane) {
		weather_plane.reset(new Plane());
		weather_surface = Bitmap::Create(320, 240);
		weather_surface->SetTransparentColor(Color(0,0,0,0));
		weather_plane->SetBitmap(weather_surface);
		weather_plane->SetZ(9999);
	}
	weather_surface->Clear();

	if (not rain_bitmap)
		rain_bitmap = Bitmap::Create(rain_image, sizeof(rain_image));

	if (not snow_bitmap)
		snow_bitmap = Bitmap::Create(snow_image, sizeof(snow_image));
}

void Game_Screen::StopWeather() {
	weather_plane.reset();
	snowflakes.clear();
}

void Game_Screen::InitSnowRain() {
	if (!snowflakes.empty())
		return;

	static const int num_snowflakes[3] = {100, 200, 300};

	for (int i = 0; i < num_snowflakes[data.weather_strength]; i++) {
		Snowflake f;
		f.x = (short) (rand() * 440.0 / RAND_MAX);
		f.y = (uint8_t) rand();
		f.life = (uint8_t) rand();
		snowflakes.push_back(f);
	}
}

static const int snowflake_life = 200;
static const int snowflake_visible = 150;

void Game_Screen::UpdateSnowRain(int speed) {
	std::vector<Snowflake>::iterator it;

	for (it = snowflakes.begin(); it != snowflakes.end(); it++) {
		Snowflake& f = *it;
		f.y += (uint8_t)speed;
		f.life++;
		if (f.life > snowflake_life)
			f.life = 0;
	}
}

void Game_Screen::DrawRain() {
	weather_plane->SetOpacity(192);

	Rect rect = rain_bitmap->GetRect();

	std::vector<Snowflake>::iterator it;
	for (it = snowflakes.begin(); it != snowflakes.end(); it++) {
		Snowflake& f = *it;
		if (f.life > snowflake_visible)
			continue;
		weather_surface->Blit(f.x - f.y/2, f.y, *rain_bitmap, rect, 255);
	}
}

void Game_Screen::DrawSnow() {
	static const int wobble[2][18] = {
		{-1,-1, 0, 1, 0, 1, 1, 0,-1,-1, 0, 1, 0, 1, 1, 0,-1, 0},
		{-1,-1, 0, 0, 1, 1, 0,-1,-1, 0, 1, 0, 1, 1, 0,-1, 0, 0}
	};
	weather_plane->SetOpacity(192);

	Rect rect = snow_bitmap->GetRect();

	std::vector<Snowflake>::iterator it;
	for (it = snowflakes.begin(); it != snowflakes.end(); it++) {
		Snowflake& f = *it;
		if (f.life > snowflake_visible)
			continue;
		int x = f.x - f.y/2;
		int y = f.y;
		int i = (y / 2) % 18;
		x += wobble[0][i];
		y += wobble[1][i];
		weather_surface->Blit(x, y, *snow_bitmap, rect, 255);
	}
}

void Game_Screen::DrawFog() {

	weather_surface->Fill(Color(128,128,128,255));
	static const int opacities[3] = {128, 160, 192};
	weather_plane->SetOpacity(opacities[data.weather_strength]);
}

void Game_Screen::DrawSandstorm() {

	weather_surface->Fill(Color(192,160,128,255));
	static const int opacities[3] = {128, 160, 192};
	weather_plane->SetOpacity(opacities[data.weather_strength]);
	// TODO
}

void Game_Screen::Update() {
	if (data.tint_time_left > 0) {
		data.tint_current_red = interpolate(data.tint_time_left, data.tint_current_red, data.tint_finish_red);
		data.tint_current_green = interpolate(data.tint_time_left, data.tint_current_green, data.tint_finish_green);
		data.tint_current_blue = interpolate(data.tint_time_left, data.tint_current_blue, data.tint_finish_blue);
		data.tint_current_sat = interpolate(data.tint_time_left, data.tint_current_sat, data.tint_finish_sat);
		data.tint_time_left--;
	}

	if (data.flash_time_left > 0) {
		data.flash_current_level = interpolate(data.flash_time_left, data.flash_current_level / 31, 0);
		data.flash_time_left--;
		if (data.flash_time_left <= 0)
			data.flash_time_left = data.flash_continuous ? flash_period : 0;
	}

    if (data.shake_continuous || data.shake_time_left > 0 || data.shake_position != 0) {
		double delta = (data.shake_strength * data.shake_speed * shake_direction) / 10.0;
		if (data.shake_time_left <= 1 && data.shake_position * (data.shake_position + delta) < 0)
			data.shake_position = 0;
		else
			data.shake_position += delta;
		if (data.shake_position > data.shake_strength * 2)
			shake_direction = -1;
		if (data.shake_position < -data.shake_strength * 2)
			shake_direction = 1;

		if (data.shake_time_left > 0)
			data.shake_time_left--;
	}

	std::vector<EASYRPG_SHARED_PTR<Picture> >::const_iterator it;
	for (it = pictures.begin(); it != pictures.end(); it++) {
		if(*it) { (*it)->Update(); }
	}

	if (!movie_filename.empty()) {
		/* update movie */
	}

	switch (data.weather) {
		case Weather_None:
			break;
		case Weather_Rain:
			InitWeather();
			InitSnowRain();
			UpdateSnowRain(4);
			DrawRain();
			break;
		case Weather_Snow:
			InitWeather();
			InitSnowRain();
			UpdateSnowRain(2);
			DrawSnow();
			break;
		case Weather_Fog:
			InitWeather();
			DrawFog();
			break;
		case Weather_Sandstorm:
			InitWeather();
			DrawSandstorm();
			break;
	}

	if (animation) {
		animation->Update();
		if (animation->IsDone()) {
			animation.reset();
		}
	}
}
