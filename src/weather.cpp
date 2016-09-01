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
#include <string>
#include <vector>
#include "baseui.h"
#include "bitmap.h"
#include "color.h"
#include "game_screen.h"
#include "graphics.h"
#include "main_data.h"
#include "weather.h"

Weather::Weather() :
	dirty(false) {

	Graphics::RegisterDrawable(this);
}

Weather::~Weather() {
	Graphics::RemoveDrawable(this);
}

int Weather::GetZ() const {
	return z;
}

DrawableType Weather::GetType() const {
	return type;
}

void Weather::Update() {
}

void Weather::Draw() {
	if (Main_Data::game_screen->GetWeatherType() != Game_Screen::Weather_None) {
		if (!weather_surface) {
			weather_surface = Bitmap::Create(SCREEN_TARGET_WIDTH, SCREEN_TARGET_HEIGHT);
		}
	}

	if (dirty && weather_surface) {
		weather_surface->Clear();
		dirty = false;
	}

	switch (Main_Data::game_screen->GetWeatherType()) {
		case Game_Screen::Weather_None:
			break;
		case Game_Screen::Weather_Rain:
			DrawRain();
			break;
		case Game_Screen::Weather_Snow:
			DrawSnow();
			break;
		case Game_Screen::Weather_Fog:
			DrawFog();
			break;
		case Game_Screen::Weather_Sandstorm:
			DrawSandstorm();
			break;
	}

	if (dirty && weather_surface) {
		BitmapRef dst = DisplayUi->GetDisplaySurface();
		dst->Blit(0, 0, *weather_surface, weather_surface->GetRect(), 255);
	}
}

static const uint8_t snow_image[] =
{
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00,
    0x0d, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
    0x00, 0x02, 0x04, 0x03, 0x00, 0x00, 0x00, 0x80, 0x98, 0x10, 0x17,
    0x00, 0x00, 0x00, 0x06, 0x50, 0x4c, 0x54, 0x45, 0xff, 0x00, 0x00,
    0xff, 0xff, 0xff, 0x41, 0x1d, 0x34, 0x11, 0x00, 0x00, 0x00, 0x09,
    0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x0a, 0xf0, 0x00, 0x00, 0x0a,
    0xf0, 0x01, 0x42, 0xac, 0x34, 0x98, 0x00, 0x00, 0x00, 0x07, 0x74,
    0x49, 0x4d, 0x45, 0x07, 0xe0, 0x05, 0x1e, 0x16, 0x12, 0x02, 0x2c,
    0xe7, 0xd0, 0xc3, 0x00, 0x00, 0x00, 0x2b, 0x74, 0x45, 0x58, 0x74,
    0x43, 0x72, 0x65, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x54, 0x69,
    0x6d, 0x65, 0x00, 0x44, 0x69, 0x20, 0x33, 0x31, 0x20, 0x4d, 0x61,
    0x69, 0x20, 0x32, 0x30, 0x31, 0x36, 0x20, 0x30, 0x30, 0x3a, 0x31,
    0x35, 0x3a, 0x31, 0x35, 0x20, 0x2b, 0x30, 0x31, 0x30, 0x30, 0xc0,
    0x15, 0x68, 0x2f, 0x00, 0x00, 0x00, 0x0c, 0x49, 0x44, 0x41, 0x54,
    0x08, 0x5b, 0x63, 0x10, 0x64, 0x10, 0x04, 0x00, 0x00, 0x48, 0x00,
    0x23, 0xf5, 0x3f, 0xea, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45,
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

static const int snowflake_visible = 150;

void Weather::DrawRain() {
	if (!rain_bitmap) {
		rain_bitmap = Bitmap::Create(rain_image, sizeof(rain_image));
		if (tone_effect != Tone()) {
			rain_bitmap->ToneBlit(0, 0, *rain_bitmap, rain_bitmap->GetRect(), tone_effect, Opacity::opaque);
		}
	}

	Rect rect = rain_bitmap->GetRect();

	const std::vector<Game_Screen::Snowflake>& snowflakes = Main_Data::game_screen->GetSnowflakes();

	std::vector<Game_Screen::Snowflake>::const_iterator it;
	for (it = snowflakes.begin(); it != snowflakes.end(); ++it) {
		const Game_Screen::Snowflake& f = *it;
		if (f.life > snowflake_visible)
			continue;
		weather_surface->Blit(f.x - f.y/2, f.y, *rain_bitmap, rect, 96);
	}

	dirty = true;
}

void Weather::DrawSnow() {
	if (!snow_bitmap) {
		snow_bitmap = Bitmap::Create(snow_image, sizeof(snow_image));
		if (tone_effect != Tone()) {
			snow_bitmap->ToneBlit(0, 0, *snow_bitmap, snow_bitmap->GetRect(), tone_effect, Opacity::opaque);
		}
	}


	static const int wobble[2][18] = {
		{-1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{-1,-1, 0, 0, 1, 1, 0,-1,-1, 0, 1, 0, 1, 1, 0,-1, 0, 0}
	};

	Rect rect = snow_bitmap->GetRect();

	const std::vector<Game_Screen::Snowflake>& snowflakes = Main_Data::game_screen->GetSnowflakes();

	for (const Game_Screen::Snowflake& f : snowflakes) {
		int x = f.x - f.y / 4;
		int y = f.y;
		int i = (y / 2) % 18;
		x += wobble[0][i];
		y += wobble[1][i];
		weather_surface->Blit(x, y, *snow_bitmap, rect, f.life);
	}

	dirty = true;
}

void Weather::DrawFog() {
	static const int opacities[3] = {128, 160, 192};
	int opacity = opacities[Main_Data::game_screen->GetWeatherStrength()];

	weather_surface->Fill(Color(128, 128, 128, opacity));

	dirty = true;
}

void Weather::DrawSandstorm() {
	static const int opacities[3] = {128, 160, 192};
	int opacity = opacities[Main_Data::game_screen->GetWeatherStrength()];

	weather_surface->Fill(Color(192, 160, 128, opacity));

	// TODO

	dirty = true;
}

Tone Weather::GetTone() const {
	return tone_effect;
}

void Weather::SetTone(Tone tone) {
	if (tone != tone_effect) {
		tone_effect = tone;
		rain_bitmap.reset();
		snow_bitmap.reset();
	}
}
