/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include "data.h"
#include "options.h"
#include "game_screen.h"

Game_Screen::Game_Screen() :
	animation(NULL),
	weather_plane(NULL),
	weather_surface(NULL),
	snow_bitmap(NULL),
	rain_bitmap(NULL)
{
	Reset();
}

Game_Screen::~Game_Screen()
{
	if (animation)
		delete animation;
	if (weather_plane)
		delete weather_plane;
	if (weather_surface)
		delete weather_surface;
}

void Game_Screen::Reset()
{
	std::map<int, Picture*>::iterator it;

	for (it = pictures.begin(); it != pictures.end(); ++it) {
		delete it->second;
	}

	pictures.clear();

	tint_red = 0;
	tint_green = 0;
	tint_blue = 0;
	tint_saturation = 0;

	tint_finish_red = 0;
	tint_finish_green = 0;
	tint_finish_blue = 0;
	tint_finish_saturation = 0;
	tint_duration = 0;

	flash_red = 0;
	flash_green = 0;
	flash_blue = 0;
	flash_saturation = 0;
	flash_timer = 0;
	flash_period = 0;

	shake_power = 0;
	shake_speed = 0;
	shake_duration = 0;
	shake_direction = 0;
	shake_position = 0;
	shake_continuous = false;

	weather_type = 0;
	weather_strength = 0;

	movie_filename = "";
	movie_pos_x = 0;
	movie_pos_y = 0;
	movie_res_x = 0;
	movie_res_y = 0;

	snowflakes.clear();
	StopWeather();
}

Picture* Game_Screen::GetPicture(int id) {
	Picture*& p = pictures[id];
	if (p == NULL)
		p = new Picture(id);
	return p;
}

void Game_Screen::TintScreen(int r, int g, int b, int s, int tenths) {
	tint_finish_red = r;
	tint_finish_green = g;
	tint_finish_blue = b;
	tint_finish_saturation = s;

	tint_duration = tenths * DEFAULT_FPS / 10;

	if (tint_duration == 0) {
		tint_red = tint_finish_red;
		tint_green = tint_finish_green;
		tint_blue = tint_finish_blue;
		tint_saturation = tint_finish_saturation;
	}
}

void Game_Screen::FlashOnce(int r, int g, int b, int s, int tenths) {
	flash_red = r;
	flash_green = g;
	flash_blue = b;
	flash_saturation = s;
	flash_level = 1.0;

	flash_timer = tenths * DEFAULT_FPS / 10;
	flash_period = 0;
}

void Game_Screen::FlashBegin(int r, int g, int b, int s, int tenths) {
	flash_red = r;
	flash_green = g;
	flash_blue = b;
	flash_saturation = s;
	flash_level = 1.0;

	flash_timer = tenths * DEFAULT_FPS / 10;
	flash_period = flash_timer;
}

void Game_Screen::FlashEnd() {
	flash_timer = 0;
	flash_period = 0;
}

void Game_Screen::ShakeOnce(int power, int speed, int tenths) {
	shake_power = power;
	shake_speed = speed;
	shake_duration = tenths * DEFAULT_FPS / 10;
	shake_position = 0;
	shake_continuous = false;
}

void Game_Screen::ShakeBegin(int power, int speed) {
	shake_power = power;
	shake_speed = speed;
	shake_duration = 0;
	shake_position = 0;
	shake_continuous = true;
}

void Game_Screen::ShakeEnd() {
	shake_duration = 0;
	shake_continuous = false;
}

void Game_Screen::Weather(int type, int strength) {
	weather_type = type;
	weather_strength = strength;
	StopWeather();
	if (weather_type != Weather_None)
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

void Game_Screen::ShowBattleAnimation(int animation_id, Game_Character* target, bool global) {
	animation = new BattleAnimation(target->GetScreenX(), target->GetScreenY(),
									&Data::animations[animation_id - 1]);
	animation->SetVisible(true);
	// FIXME: target
	// FIXME: global
}

bool Game_Screen::IsBattleAnimationWaiting() const {
	return animation != NULL;
}

static double interpolate(double d, double x0, double x1)
{
	return (x0 * (d - 1) + x1) / d;
}

static const uint8 snow_image[] = {
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

static const uint8 rain_image[] = {
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
	if (weather_plane == NULL) {
		weather_plane = new Plane();
		weather_surface = Surface::CreateSurface(320, 240);
		weather_surface->SetTransparentColor(Color(0,0,0,0));
		weather_plane->SetBitmap(weather_surface);
		weather_plane->SetZ(9999);
	}
	weather_surface->Clear();

	if (rain_bitmap == NULL)
		rain_bitmap = Bitmap::CreateBitmap(rain_image, sizeof(rain_image));

	if (snow_bitmap == NULL)
		snow_bitmap = Bitmap::CreateBitmap(snow_image, sizeof(snow_image));
}

void Game_Screen::StopWeather() {
	if (weather_plane != NULL)
		delete weather_plane;
	weather_plane = NULL;
	snowflakes.clear();
}

void Game_Screen::InitSnowRain() {
	if (!snowflakes.empty())
		return;

	static const int num_snowflakes[3] = {100, 200, 300};

	for (int i = 0; i < num_snowflakes[weather_strength]; i++) {
		Snowflake f;
		f.x = (short) (rand() * 440.0 / RAND_MAX);
		f.y = (uint8) rand();
		f.life = (uint8) rand();
		snowflakes.push_back(f);
	}
}

static const int snowflake_life = 200;
static const int snowflake_visible = 150;

void Game_Screen::UpdateSnowRain(int speed) {
	std::vector<Snowflake>::iterator it;

	for (it = snowflakes.begin(); it != snowflakes.end(); it++) {
		Snowflake& f = *it;
		f.y += (uint8)speed;
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
		weather_surface->Blit(f.x - f.y/2, f.y, rain_bitmap, rect, 255);
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
		weather_surface->Blit(x, y, snow_bitmap, rect, 255);
	}
}

void Game_Screen::DrawFog() {

	weather_surface->Fill(Color(128,128,128,255));
	static const int opacities[3] = {128, 160, 192};
	weather_plane->SetOpacity(opacities[weather_strength]);
}

void Game_Screen::DrawSandstorm() {

	weather_surface->Fill(Color(192,160,128,255));
	static const int opacities[3] = {128, 160, 192};
	weather_plane->SetOpacity(opacities[weather_strength]);
	// TODO
}

void Game_Screen::Update() {
	if (tint_duration > 0) {
		tint_red = interpolate(tint_duration, tint_red, tint_finish_red);
		tint_green = interpolate(tint_duration, tint_green, tint_finish_green);
		tint_blue = interpolate(tint_duration, tint_blue, tint_finish_blue);
		tint_saturation = interpolate(tint_duration, tint_saturation, tint_finish_saturation);
		tint_duration--;
	}

	if (flash_timer > 0) {
		flash_level = interpolate(flash_timer, flash_level, 0);
		flash_timer--;
		if (flash_timer <= 0)
			flash_timer = flash_period;
	}

    if (shake_continuous || shake_duration > 0 || shake_position != 0) {
		double delta = (shake_power * shake_speed * shake_direction) / 10.0;
		if (shake_duration <= 1 && shake_position * (shake_position + delta) < 0)
			shake_position = 0;
		else
			shake_position += delta;
		if (shake_position > shake_power * 2)
			shake_direction = -1;
		if (shake_position < -shake_power * 2)
			shake_direction = 1;

		if (shake_duration > 0)
			shake_duration--;
	}

	std::map<int,Picture*>::const_iterator it;
	for (it = pictures.begin(); it != pictures.end(); it++)
		it->second->Update();

	if (!movie_filename.empty()) {
		/* update movie */
	}

	switch (weather_type) {
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

	if (animation != NULL) {
		animation->Update();
		if (animation->GetFrame() >= animation->GetFrames()) {
			delete animation;
			animation = NULL;
		}
	}
}

