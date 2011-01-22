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

#include "options.h"
#include "game_screen.h"

Game_Screen::Game_Screen() :
	weather_plane(NULL),
	snow_bitmap(NULL),
	rain_bitmap(NULL)
{
	Reset();
}

Game_Screen::~Game_Screen()
{
	if (weather_plane)
		delete weather_plane;
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
		p = new Picture();
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

static double interpolate(double d, double x0, double x1)
{
	return (x0 * (d - 1) + x1) / d;
}

void Game_Screen::InitWeather() {
	if (weather_plane == NULL) {
		weather_plane = new Plane();
		Bitmap* bitmap = Bitmap::CreateBitmap(320, 240);
		bitmap->SetTransparentColor(Color(0,0,0,0));
		weather_plane->SetBitmap(bitmap);
		weather_plane->SetZ(9999);
	}
	weather_plane->GetBitmap()->Clear();

	if (rain_bitmap == NULL) {
		Color gray(192,192,192,255);
		rain_bitmap = Bitmap::CreateBitmap(8, 16);
		rain_bitmap->Clear();
		for (int j = 0; j < 16; j++)
			rain_bitmap->SetPixel(7-j/2, j, gray);
	}

	if (snow_bitmap == NULL) {
		snow_bitmap = Bitmap::CreateBitmap(4, 4);
		snow_bitmap->Clear();
		Color gray(192,192,192,255);
		Color white(255,255,255,255);
		Color colors[3] = {
			snow_bitmap->GetTransparentColor(),
			gray,
			white
		};
		static const int snow[4][4] = {
			{0, 1, 1, 0},
			{1, 2, 2, 1},
			{1, 2, 2, 1},
			{0, 1, 1, 0}
		};
		for (int j = 0; j < 4; j++)
			for (int i = 0; i < 4; i++)
				snow_bitmap->SetPixel(i, j, colors[snow[j][i]]);
	}
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
	Bitmap* bitmap = weather_plane->GetBitmap();
	weather_plane->SetOpacity(192);

	Rect rect = rain_bitmap->GetRect();

	std::vector<Snowflake>::iterator it;
	for (it = snowflakes.begin(); it != snowflakes.end(); it++) {
		Snowflake& f = *it;
		if (f.life > snowflake_visible)
			continue;
		bitmap->Blit(f.x - f.y/2, f.y, rain_bitmap, rect, 255);
	}
}

void Game_Screen::DrawSnow() {
	static const int wobble[2][18] = {
		{-1,-1, 0, 1, 0, 1, 1, 0,-1,-1, 0, 1, 0, 1, 1, 0,-1, 0},
		{-1,-1, 0, 0, 1, 1, 0,-1,-1, 0, 1, 0, 1, 1, 0,-1, 0, 0}
	};
	Bitmap* bitmap = weather_plane->GetBitmap();
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
		bitmap->Blit(x, y, snow_bitmap, rect, 255);
	}
}

void Game_Screen::DrawFog() {

	Bitmap* bitmap = weather_plane->GetBitmap();
	bitmap->Fill(Color(128,128,128,255));
	static const int opacities[3] = {128, 160, 192};
	weather_plane->SetOpacity(opacities[weather_strength]);
}

void Game_Screen::DrawSandstorm() {

	Bitmap* bitmap = weather_plane->GetBitmap();
	bitmap->Fill(Color(192,160,128,255));
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
}

