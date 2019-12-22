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
#include "bitmap.h"
#include "color.h"
#include "game_screen.h"
#include "main_data.h"
#include "weather.h"
#include "drawable_mgr.h"
#include "player.h"

Weather::Weather() :
	Drawable(TypeWeather, Priority_Weather, false)
{
	DrawableMgr::Register(this);
}

void Weather::Update() {
}

void Weather::Draw(Bitmap& dst) {
	switch (Main_Data::game_screen->GetWeatherType()) {
		case Game_Screen::Weather_None:
			break;
		case Game_Screen::Weather_Rain:
			DrawRain(dst);
			break;
		case Game_Screen::Weather_Snow:
			DrawSnow(dst);
			break;
		case Game_Screen::Weather_Fog:
			DrawFog(dst);
			break;
		case Game_Screen::Weather_Sandstorm:
			DrawSandstorm(dst);
			break;
	}
}

static constexpr uint8_t snow_image[] =
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

static constexpr uint8_t rain_image[] = {
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

static constexpr int fog_overlay_tile_width = TILE_SIZE;
static constexpr int fog_overlay_tile_height = TILE_SIZE;
static constexpr int fog_overlay_num_colors = 3;

static constexpr Color fog_overlay_colors[fog_overlay_num_colors] = {
	{ 230, 230, 230, 255 },
	{ 240, 240, 240, 255 },
	{ 255, 255, 255, 255 },
};

static constexpr Color sand_overlay_colors[fog_overlay_num_colors] = {
	{ 220, 220, 160, 255 },
	{ 230, 230, 170, 255 },
	{ 240, 240, 180, 255 },
};

// RPG_RT only allows strength 0, 1, and 2. If you hack strength 3,
// it will show the upper layer as fully opaque and the lower layer
// with opacity 64. We set lower layer to 0 and don't render it
// since it can't be seen anyway.
static constexpr int num_opacities = 4;
static constexpr int fog_opacity[2][4] = {
	{ 32, 64, 96, 0 },
	{ 64, 80, 160, 255 },
};

static constexpr int snowflake_visible = 150;

void Weather::DrawRain(Bitmap& dst) {
	if (!rain_bitmap) {
		rain_bitmap = Bitmap::Create(rain_image, sizeof(rain_image));
		if (tone_effect != Tone()) {
			rain_bitmap->ToneBlit(0, 0, *rain_bitmap, rain_bitmap->GetRect(), tone_effect, Opacity::opaque, true);
		}
	}

	Rect rect = rain_bitmap->GetRect();

	const auto& snowflakes = Main_Data::game_screen->GetSnowflakes();

	for (auto& sf: snowflakes) {
		if (sf.life > snowflake_visible) {
			continue;
		}
		dst.Blit(sf.x - sf.y/2, sf.y, *rain_bitmap, rect, 96);
	}
}

void Weather::DrawSnow(Bitmap& dst) {
	if (!snow_bitmap) {
		snow_bitmap = Bitmap::Create(snow_image, sizeof(snow_image));
		if (tone_effect != Tone()) {
			snow_bitmap->ToneBlit(0, 0, *snow_bitmap, snow_bitmap->GetRect(), tone_effect, Opacity::opaque, true);
		}
	}

	static constexpr int wobble[2][18] = {
		{-1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{-1,-1, 0, 0, 1, 1, 0,-1,-1, 0, 1, 0, 1, 1, 0,-1, 0, 0}
	};

	Rect rect = snow_bitmap->GetRect();

	const auto& snowflakes = Main_Data::game_screen->GetSnowflakes();

	for (const auto& sf : snowflakes) {
		int x = sf.x - sf.y / 4;
		int y = sf.y;
		int i = (y / 2) % 18;
		x += wobble[0][i];
		y += wobble[1][i];
		dst.Blit(x, y, *snow_bitmap, rect, sf.life);
	}
}

void Weather::DrawFog(Bitmap& dst) {
	if (!fog_bitmap) {
		CreateFogOverlay();
	}

	DrawFogOverlay(dst, *fog_bitmap, fog_tone_bitmap);
}

void Weather::DrawSandstorm(Bitmap& dst) {
	if (!sand_bitmap) {
		CreateFogOverlay();
	}

	DrawFogOverlay(dst, *sand_bitmap, sand_tone_bitmap);

	// FIXME: Figure out sand particules
	// 4 colors: white, red, orange, yellow, 1 pixel wide, 2 pixels tall.
}

void Weather::CreateFogOverlay() {
	uint32_t fog_pixels[fog_overlay_num_colors];
	uint32_t sand_pixels[fog_overlay_num_colors];

	for (int i = 0; i < fog_overlay_num_colors; ++i) {
		auto fc = fog_overlay_colors[i];
		auto sc = sand_overlay_colors[i];
		fog_pixels[i] = Bitmap::pixel_format.rgba_to_uint32_t(fc.red, fc.green, fc.blue, fc.alpha);
		sand_pixels[i] = Bitmap::pixel_format.rgba_to_uint32_t(sc.red, sc.green, sc.blue, sc.alpha);
	}

	const auto h = fog_overlay_tile_height;
	const auto w = fog_overlay_tile_width;

	fog_bitmap = Bitmap::Create(w, h);
	sand_bitmap = Bitmap::Create(w, h);

	auto* fog_img = reinterpret_cast<uint32_t*>(fog_bitmap->pixels());
	auto* sand_img = reinterpret_cast<uint32_t*>(sand_bitmap->pixels());

	for (int i = 0; i < w * h; ++i) {
		// FIXME: How well does this match RPG_RT textures?
		int px = Utils::GetRandomNumber(0, fog_overlay_num_colors - 1);
		// FIXME: This only works for 32bit pixel formats
		fog_img[i] = fog_pixels[px];
		sand_img[i] = sand_pixels[px];
	}
}

void Weather::DrawFogOverlay(Bitmap& dst, const Bitmap& overlay, BitmapRef& tone_overlay) {
	auto* src = &overlay;

	const auto dr = dst.GetRect();
	const auto sr = src->GetRect();

	if (tone_effect != Tone()) {
		if (!tone_overlay) {
			tone_overlay = Bitmap::Create(overlay, sr);
		}
		if (tone_dirty) {
			tone_overlay->ToneBlit(0, 0, overlay, sr, tone_effect, Opacity::opaque, false);
		}
		src = tone_overlay.get();
	}

	auto str = Utils::Clamp(Main_Data::game_screen->GetWeatherStrength(), 0, num_opacities - 1);
	int back_opacity = fog_opacity[0][str];
	int front_opacity = fog_opacity[1][str];


	// FIXME: Confirm exact speed in x direction
	// FIXME: Confirm algorithm for changes in y. Appears to be very slow and random.
	int frames = Player::GetFrames();
	const int x = (frames * 32 / 256) % fog_overlay_tile_width;
	const int y = (frames * 1 / 256) % fog_overlay_tile_width;

	// FIXME: Confirm whether back layer moves right or is still?
	dst.TiledBlit(-x + 8, -y, sr, *src, dr, back_opacity);
	dst.TiledBlit(x, -y, sr, *src, dr, front_opacity);
}

void Weather::SetTone(Tone tone) {
	if (tone != tone_effect) {
		tone_effect = tone;
		rain_bitmap.reset();
		snow_bitmap.reset();

		tone_dirty = true;
	}
}
