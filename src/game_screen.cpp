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

Game_Screen::Game_Screen()
{
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

	weather_type = 0;
	weather_strength = 0;
}

Game_Screen::~Game_Screen()
{
}

Picture& Game_Screen::GetPicture(int id) {
	if ((size_t) id >= pictures.size())
		pictures.resize(id);
	return pictures[id - 1];
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

void Game_Screen::Weather(int type, int strength) {
	weather_type = type;
	weather_strength = strength;
}

void Game_Screen::ShakeEnd() {
	shake_duration = 0;
	shake_continuous = false;
}

static double interpolate(double d, double x0, double x1)
{
	return (x0 * (d - 1) + x1) / d;
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

	std::vector<Picture>::iterator it;
	for (it = pictures.begin(); it < pictures.end(); it++)
		it->Update();
}

