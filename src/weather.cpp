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
#include "bitmap_screen.h"
#include "color.h"
#include "game_screen.h"
#include "graphics.h"
#include "main_data.h"
#include "weather.h"

Weather::Weather() :
	ID(Graphics::drawable_id++) {

	zobj = Graphics::RegisterZObj(z, ID);
	Graphics::RegisterDrawable(ID, this);

	weather_screen = BitmapScreen::Create();
	weather_surface = Bitmap::Create(320, 240);
	weather_screen->SetBitmap(weather_surface);
}

Weather::~Weather() {
	Graphics::RemoveZObj(ID);
	Graphics::RemoveDrawable(ID);
}

int Weather::GetZ() const {
	return z;
}

unsigned long Weather::GetId() const {
	return ID;
}

DrawableType Weather::GetType() const {
	return type;
}

void Weather::Update() {
}

void Weather::Draw(int /* z_order */) {
	weather_surface->Clear();

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

	weather_screen->BlitScreen(0, 0);
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

static const int snowflake_visible = 150;

void Weather::DrawRain() {
	if (!rain_bitmap) {
		rain_bitmap = Bitmap::Create(rain_image, sizeof(rain_image));
	}

	Rect rect = rain_bitmap->GetRect();

	const std::vector<Game_Screen::Snowflake>& snowflakes = Main_Data::game_screen->GetSnowflakes();

	std::vector<Game_Screen::Snowflake>::const_iterator it;
	for (it = snowflakes.begin(); it != snowflakes.end(); it++) {
		const Game_Screen::Snowflake& f = *it;
		if (f.life > snowflake_visible)
			continue;
		weather_surface->Blit(f.x - f.y/2, f.y, *rain_bitmap, rect, 192);
	}
}

void Weather::DrawSnow() {
	if (!snow_bitmap) {
		snow_bitmap = Bitmap::Create(snow_image, sizeof(snow_image));
	}

	static const int wobble[2][18] = {
		{-1,-1, 0, 1, 0, 1, 1, 0,-1,-1, 0, 1, 0, 1, 1, 0,-1, 0},
		{-1,-1, 0, 0, 1, 1, 0,-1,-1, 0, 1, 0, 1, 1, 0,-1, 0, 0}
	};

	Rect rect = snow_bitmap->GetRect();

	const std::vector<Game_Screen::Snowflake>& snowflakes = Main_Data::game_screen->GetSnowflakes();

	std::vector<Game_Screen::Snowflake>::const_iterator it;
	for (it = snowflakes.begin(); it != snowflakes.end(); it++) {
		const Game_Screen::Snowflake& f = *it;
		if (f.life > snowflake_visible)
			continue;
		int x = f.x - f.y/2;
		int y = f.y;
		int i = (y / 2) % 18;
		x += wobble[0][i];
		y += wobble[1][i];
		weather_surface->Blit(x, y, *snow_bitmap, rect, 192);
	}
}

void Weather::DrawFog() {
	static const int opacities[3] = {128, 160, 192};
	int opacity = opacities[Main_Data::game_screen->GetWeatherStrength()];

	weather_surface->Fill(Color(128, 128, 128, opacity));
}

void Weather::DrawSandstorm() {
	static const int opacities[3] = {128, 160, 192};
	int opacity = opacities[Main_Data::game_screen->GetWeatherStrength()];

	weather_surface->Fill(Color(192, 160, 128, opacity));

	// TODO
}
