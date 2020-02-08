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
	Drawable(Priority_Weather, Drawable::Flags::Shared)
{
	DrawableMgr::Register(this);

	auto rect = Main_Data::game_screen->GetScreenEffectsRect();
	weather_surface = Bitmap::Create(rect.width, rect.height, true);
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

static constexpr auto rain_bitmap_rect = Rect{ 0, 0, 6, 24 };
static constexpr auto snow_bitmap_rect = Rect{ 0, 0, 2, 2 };
static constexpr auto overlay_bitmap_rect = Rect{ 0, 0, TILE_SIZE, TILE_SIZE };

static constexpr int num_sand_colors = 4;
static constexpr int num_sand_strength = 3;
static constexpr auto sand_particle_rect = Rect{ 0, 0, 1, 2 };

static constexpr auto sand_particle_bitmap_rect = Rect{
	0, 0,
	sand_particle_rect.width,
	sand_particle_rect.height * num_sand_colors * num_sand_strength
};

static constexpr Rect MakeMaxBitmapRect(std::initializer_list<Rect> list) {
	int max_w = 0;
	int max_h = 0;
	for (auto& rect: list) {
		max_w = std::max(rect.width, max_w);
		max_h = std::max(rect.height, max_h);
	}
	return Rect{ 0, 0, max_w, max_h };
}

static constexpr auto tone_bitmap_rect = MakeMaxBitmapRect({ rain_bitmap_rect, snow_bitmap_rect, overlay_bitmap_rect, sand_particle_bitmap_rect });

static constexpr int num_overlay_colors = 3;

static constexpr Color fog_overlay_colors[num_overlay_colors] = {
	{ 230, 230, 230, 255 },
	{ 240, 240, 240, 255 },
	{ 255, 255, 255, 255 },
};

static constexpr Color sand_overlay_colors[num_overlay_colors] = {
	{ 220, 220, 160, 255 },
	{ 230, 230, 170, 255 },
	{ 240, 240, 180, 255 },
};

// RPG_RT only allows strength 0, 1, and 2. If you hack strength 3,
// it will show the upper layer as fully opaque and the lower layer
// with opacity 64. We set lower layer to 0 and don't render it
// since it can't be seen anyway.
// If you hack strength 4 or higher, RPG_RT will exhibit buggy
// wraparound behavior. In Player, we clamp at 3.
static constexpr int num_opacities = 4;
static constexpr int fog_opacity[2][4] = {
	{ 32, 64, 96, 0 },
	{ 64, 80, 160, 255 },
};

const Bitmap* Weather::ApplyToneEffect(const Bitmap& bitmap, Rect rect) {
	if (tone_effect == Tone()) {
		return &bitmap;
	}

	if (!tone_bitmap) {
		assert(tone_dirty && "Tone Bitmap Created but tone was not marked dirty!");
		tone_bitmap = Bitmap::Create(tone_bitmap_rect.width, tone_bitmap_rect.height, true);
	}

	if (tone_dirty) {
		tone_bitmap->ToneBlit(0, 0, bitmap, rect, tone_effect, Opacity::Opaque(), true);
	}
	return tone_bitmap.get();
}

void Weather::CreateRainParticle() {
	constexpr int w = rain_bitmap_rect.width;
	constexpr int h = rain_bitmap_rect.height;
	rain_bitmap = Bitmap::Create(w, h, true);

	const auto pixel = Bitmap::pixel_format.rgba_to_uint32_t(255,255,255,255);

	auto* img = reinterpret_cast<uint32_t*>(rain_bitmap->pixels());

	for (int y = 0; y < h; ++y) {
		int x = w - (y / 4) - 1;

		img[y * w + x] = pixel;
	}
}

void Weather::DrawRain(Bitmap& dst) {
	if (!rain_bitmap) {
		CreateRainParticle();
	}
	DrawParticles(dst, *rain_bitmap, rain_bitmap_rect);
}


void Weather::CreateSnowParticle() {
	constexpr auto w = snow_bitmap_rect.width;
	constexpr auto h = snow_bitmap_rect.height;
	snow_bitmap = Bitmap::Create(w, h, true);

	const auto pixel = Bitmap::pixel_format.rgba_to_uint32_t(255,255,255,255);

	auto* img = reinterpret_cast<uint32_t*>(snow_bitmap->pixels());

	for (int i = 0; i < w * h; ++i) {
		img[i] = pixel;
	}
}

void Weather::DrawSnow(Bitmap& dst) {
	if (!snow_bitmap) {
		CreateSnowParticle();
	}
	DrawParticles(dst, *snow_bitmap, snow_bitmap_rect);
}

void Weather::DrawParticles(Bitmap& dst, const Bitmap& particle, const Rect rect) {
	auto* bitmap = ApplyToneEffect(particle, rect);

	const auto& particles = Main_Data::game_screen->GetParticles();

	auto surface_rect = weather_surface->GetRect();
	weather_surface->Clear();

	for (auto& p: particles) {
		auto x = Utils::PositiveModulo(p.x, surface_rect.width);
		auto y = Utils::PositiveModulo(p.y, surface_rect.height);

		weather_surface->EdgeMirrorBlit(x, y, *bitmap, rect, true, true, p.life);
	}

	const auto shake_x = Main_Data::game_screen->GetShakeOffsetX();
	const auto shake_y = Main_Data::game_screen->GetShakeOffsetY();
	auto pan_rect = Main_Data::game_screen->GetScreenEffectsRect();
	dst.TiledBlit(-pan_rect.x + shake_x, -pan_rect.y + shake_y, surface_rect, *weather_surface, dst.GetRect(), Opacity::Opaque());
}

void Weather::DrawFog(Bitmap& dst) {
	if (!fog_bitmap) {
		CreateFogOverlay();
	}

	DrawFogOverlay(dst, *fog_bitmap);
}

void Weather::DrawSandstorm(Bitmap& dst) {
	if (!sand_bitmap) {
		CreateFogOverlay();
	}
	if (!sand_particle_bitmap) {
		CreateSandParticle();
	}

	DrawFogOverlay(dst, *sand_bitmap);
	DrawSandParticles(dst, *sand_particle_bitmap);
}

void Weather::CreateSandParticle() {
	constexpr int w = sand_particle_bitmap_rect.width;
	constexpr int h = sand_particle_bitmap_rect.height;

	sand_particle_bitmap = Bitmap::Create(w, h, true);

	// FIXME: Close but probably not accurate
	const std::array<uint32_t,num_sand_colors * num_sand_strength> pixels = {{
		// Strength 0
		Bitmap::pixel_format.rgba_to_uint32_t(212,212,196,255),
		Bitmap::pixel_format.rgba_to_uint32_t(208,48,40,255),
		Bitmap::pixel_format.rgba_to_uint32_t(208,208,40,255),
		Bitmap::pixel_format.rgba_to_uint32_t(208,152,40,255),
		// Strength 1
		Bitmap::pixel_format.rgba_to_uint32_t(236,236,215,255),
		Bitmap::pixel_format.rgba_to_uint32_t(222,54,45,255),
		Bitmap::pixel_format.rgba_to_uint32_t(218,218,45,255),
		Bitmap::pixel_format.rgba_to_uint32_t(222,164,44,255),
		// Strength 2
		Bitmap::pixel_format.rgba_to_uint32_t(236,236,215,255),
		Bitmap::pixel_format.rgba_to_uint32_t(240,64,52,255),
		Bitmap::pixel_format.rgba_to_uint32_t(236,236,60,255),
		Bitmap::pixel_format.rgba_to_uint32_t(236,180,60,255),
	}};

	auto* img = reinterpret_cast<uint32_t*>(sand_particle_bitmap->pixels());

	for (int i = 0; i < w * h; ++i) {
		img[i] = pixels[i / 2];
	}
}

void Weather::DrawSandParticles(Bitmap& dst, const Bitmap& particle_bitmap) {
	const auto& particles = Main_Data::game_screen->GetParticles();
	const auto strength = Utils::Clamp(Main_Data::game_screen->GetWeatherStrength(), 0, num_sand_strength - 1);
	const auto offset = strength * sand_particle_rect.height * num_sand_colors;

	auto* bitmap = ApplyToneEffect(particle_bitmap, particle_bitmap.GetRect());

	for (int i = 0; i < static_cast<int>(particles.size()); ++i) {
		auto& p = particles[i];
		auto rect = Rect{
			0,
			(i % num_sand_colors) * sand_particle_rect.height + offset,
			sand_particle_rect.width,
			sand_particle_rect.height
		};
		auto x = p.x / 16;
		auto y = p.y / 16;
		dst.Blit(x, y, *bitmap, rect, p.life);
	}
}

void Weather::CreateFogOverlay() {
	uint32_t fog_pixels[num_overlay_colors];
	uint32_t sand_pixels[num_overlay_colors];

	for (int i = 0; i < num_overlay_colors; ++i) {
		auto fc = fog_overlay_colors[i];
		auto sc = sand_overlay_colors[i];
		fog_pixels[i] = Bitmap::pixel_format.rgba_to_uint32_t(fc.red, fc.green, fc.blue, fc.alpha);
		sand_pixels[i] = Bitmap::pixel_format.rgba_to_uint32_t(sc.red, sc.green, sc.blue, sc.alpha);
	}

	constexpr auto w = overlay_bitmap_rect.width;
	constexpr auto h = overlay_bitmap_rect.height;

	fog_bitmap = Bitmap::Create(w, h);
	sand_bitmap = Bitmap::Create(w, h);

	auto* fog_img = reinterpret_cast<uint32_t*>(fog_bitmap->pixels());
	auto* sand_img = reinterpret_cast<uint32_t*>(sand_bitmap->pixels());

	for (int i = 0; i < w * h; ++i) {
		// FIXME: How well does this match RPG_RT textures?
		int px = Utils::GetRandomNumber(0, num_overlay_colors - 1);
		// FIXME: This only works for 32bit pixel formats
		fog_img[i] = fog_pixels[px];
		sand_img[i] = sand_pixels[px];
	}
}

void Weather::DrawFogOverlay(Bitmap& dst, const Bitmap& overlay) {
	const auto dr = dst.GetRect();
	constexpr auto sr = overlay_bitmap_rect;

	auto* src = ApplyToneEffect(overlay, sr);

	auto strength = Utils::Clamp(Main_Data::game_screen->GetWeatherStrength(), 0, num_opacities - 1);
	int back_opacity = fog_opacity[0][strength];
	int front_opacity = fog_opacity[1][strength];

	const auto shake_x = Main_Data::game_screen->GetShakeOffsetX();
	const auto shake_y = Main_Data::game_screen->GetShakeOffsetY();

	// FIXME: Confirm exact speed in x direction
	// FIXME: Confirm algorithm for changes in y. Appears to be very slow and random.
	int frames = Player::GetFrames();
	const int x = (frames * 32 / 256) % sr.width + shake_x;
	const int y = (frames * 1 / 256) % sr.height - shake_y;

	// FIXME: Confirm whether back layer moves right or is still?
	dst.TiledBlit(-x + 8, -y, sr, *src, dr, back_opacity);
	dst.TiledBlit(x, -y, sr, *src, dr, front_opacity);
}

void Weather::SetTone(Tone tone) {
	if (tone != tone_effect) {
		tone_effect = tone;
		tone_dirty = true;
	}
}

void Weather::OnWeatherChanged() {
	tone_dirty = true;
	if (tone_bitmap) {
		tone_bitmap->Clear();
	}
}
