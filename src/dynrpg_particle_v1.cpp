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

// Ported from DynRPG Particle Effects V1 by Kazesui (MIT license).

#include "dynrpg_particle_v1.h"

#include <algorithm>
#include <cmath>
#include <list>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "async_handler.h"
#include "bitmap.h"
#include "drawable.h"
#include "drawable_mgr.h"
#include "filefinder.h"
#include "game_map.h"
#include "output.h"
#include "rand.h"
#include "utils.h"

struct ParticleObj {
	double size;
	double x, dx, y, dy;
};

struct BurstObj {
	double r, g, b, alpha;
	std::list<ParticleObj> particles;
};

class ParticleEffectV1 : public Drawable {
public:
	ParticleEffectV1() : Drawable(Priority_Frame + 1) {
		DrawableMgr::Register(this);
	}

	~ParticleEffectV1() override = default;

	int amount = 50;
	int size = 2;
	int red = 255;
	int green = 255;
	int blue = 255;
	int rand_x = 0;
	int rand_y = 0;
	double spd = 2;
	double rand_spd = 2;
	double timeout = 30;
	double delay = 0;
	double angle_start = 0;
	double angle_span = 2 * M_PI;
	bool mask = false;
	std::string filename = "DynPlugins/Particle";

	double dr = 0;
	double dg = 0;
	double db = 0;
	double ds = 0;
	double da = 8.5;

	std::list<BurstObj> bursts;
	BitmapRef image;
	bool has_texture = false;

	void SetTimeout(double t) {
		double time_diff = t - delay;
		if (time_diff <= 0) {
			time_diff = 1.0;
		}

		da = 255.0 / time_diff;
		timeout = t;
	}

	void SetAngle(double a1, double a2) {
		angle_start = a1 * M_PI / 180.0;
		angle_span = (a2 - a1) * M_PI / 180.0;
	}

	void SetGrowth(int s, int new_size) {
		size = s;
		double t = (timeout > 0) ? timeout : 1.0;
		ds = (new_size - size) / t;
	}

	void ColorFade(int r = 0, int g = 0, int b = 0) {
		double t = (timeout > 0) ? timeout : 1.0;
		dr = (static_cast<double>(r) - red) / t;
		dg = (static_cast<double>(g) - green) / t;
		db = (static_cast<double>(b) - blue) / t;
	}

	void LoadTexture() {
		// A failed load falls back to untextured rectangles, so do not retry it
		has_texture = true;

		const std::string name = DynRpg::StripImageExtension(filename);
		if (name.empty()) {
			return;
		}

		// The plugin took a path relative to the game directory, not a picture name
		FileRequestAsync* req = AsyncHandler::RequestFile(name);
		req->Start();

		auto stream = FileFinder::OpenImage(".", name);
		if (!stream) {
			Output::Warning("DynRPG ParticleV1: Could not load '{}'", filename);
			return;
		}

		image = Bitmap::Create(std::move(stream), mask);
	}

	void NewBurst(int x, int y) {
		BurstObj burst;
		burst.r = red;
		burst.g = green;
		burst.b = blue;
		burst.alpha = 255.0;

		if (!has_texture && !filename.empty()) {
			LoadTexture();
		}

		for (int i = 0; i < amount; ++i) {
			double rand_val1 = static_cast<double>(Rand::GetRandomNumber(0, 32767)) / 32767.0;
			double rand_val2 = static_cast<double>(Rand::GetRandomNumber(0, 32767)) / 32767.0;
			double rand_val3 = static_cast<double>(Rand::GetRandomNumber(0, 32767)) / 32767.0;
			double rand_val4 = static_cast<double>(Rand::GetRandomNumber(0, 32767)) / 32767.0;

			double angle = angle_start + rand_val1 * angle_span;
			double new_spd = spd + rand_spd * rand_val2;

			ParticleObj pa;
			pa.x = x + 2 * rand_x * rand_val3 - rand_x;
			pa.y = y + 2 * rand_y * rand_val4 - rand_y;
			pa.dx = new_spd * std::cos(angle);
			pa.dy = new_spd * std::sin(angle);
			pa.size = size;

			burst.particles.push_back(pa);
		}
		bursts.push_back(burst);
	}

	void NewHeart(int x, int y) {
		BurstObj burst;
		burst.r = red;
		burst.g = green;
		burst.b = blue;
		burst.alpha = 255.0;

		if (!has_texture && !filename.empty()) {
			LoadTexture();
		}

		for (int i = 0; i < amount; ++i) {
			double rand_val = static_cast<double>(Rand::GetRandomNumber(0, 32767)) / 32767.0;
			double angle = rand_val * 2 * M_PI;

			ParticleObj pa;
			pa.x = x;
			pa.y = y;
			pa.dx = spd * (16 * std::pow(std::sin(angle), 3));
			pa.dy = -spd * (13 * std::cos(angle) - 5 * std::cos(2 * angle) - 2 * std::cos(3 * angle) - std::cos(4 * angle));
			pa.size = size;

			burst.particles.push_back(pa);
		}
		bursts.push_back(burst);
	}

	void KillParticles() {
		bursts.clear();
	}

	void Update() {
		if (bursts.empty()) {
			return;
		}

		auto burst_itr = bursts.begin();
		while (burst_itr != bursts.end()) {
			burst_itr->r += dr;
			burst_itr->g += dg;
			burst_itr->b += db;

			if (delay <= 0) {
				burst_itr->alpha -= da;
				if (burst_itr->alpha <= 0) {
					burst_itr = bursts.erase(burst_itr);
					continue;
				}
			} else {
				--delay;
			}

			for (auto& particle : burst_itr->particles) {
				particle.x += particle.dx;
				particle.y += particle.dy;
				particle.size += ds;
			}
			++burst_itr;
		}
	}

	void Draw(Bitmap& dst) override;
};

void ParticleEffectV1::Draw(Bitmap& dst) {
	if (bursts.empty()) {
		return;
	}

	const int cam_x = Game_Map::GetDisplayX() / 16;
	const int cam_y = Game_Map::GetDisplayY() / 16;

	for (auto& burst : bursts) {
		const int draw_alpha = std::clamp(static_cast<int>(burst.alpha), 0, 255);
		const int cur_r = std::clamp(static_cast<int>(burst.r), 0, 255);
		const int cur_g = std::clamp(static_cast<int>(burst.g), 0, 255);
		const int cur_b = std::clamp(static_cast<int>(burst.b), 0, 255);

		BitmapRef colored_image;
		if (has_texture && image) {
			colored_image = Bitmap::Create(image->GetWidth(), image->GetHeight(), true);
			colored_image->Clear();
			const Tone tone(cur_r * 128 / 100, cur_g * 128 / 100, cur_b * 128 / 100, 128);
			colored_image->ToneBlit(0, 0, *image, image->GetRect(), tone, Opacity::Opaque());
		}

		for (auto& particle : burst.particles) {
			const float draw_size = static_cast<float>(particle.size);
			const float draw_x = static_cast<float>(particle.x) - cam_x - draw_size / 2.0f;
			const float draw_y = static_cast<float>(particle.y) - cam_y - draw_size / 2.0f;
			const int rect_size = static_cast<int>(std::max(1.0f, draw_size));

			const Rect dst_rect(static_cast<int>(draw_x), static_cast<int>(draw_y), rect_size, rect_size);

			if (colored_image) {
				dst.StretchBlit(dst_rect, *colored_image, colored_image->GetRect(), Opacity(draw_alpha));
			} else {
				dst.FillRect(dst_rect, Color(cur_r, cur_g, cur_b, draw_alpha));
			}
		}
	}
}

DynRpg::ParticleV1::ParticleV1(Game_DynRpg& instance) : DynRpgPlugin("ParticleSystemV1", instance) {}

bool DynRpg::ParticleV1::Invoke(std::string_view func, dyn_arg_list args, bool& /*do_yield*/,
	Game_Interpreter* /*interpreter*/) {

	if (func == "new_effect") {
		effects.push_back(std::make_unique<ParticleEffectV1>());
		effects.back()->SetVisible(do_draw);
		return true;
	}
	if (func == "stop") {
		do_draw = false;
		for (auto& effect: effects) {
			effect->SetVisible(false);
		}
		return true;
	}
	if (func == "start") {
		do_draw = true;
		for (auto& effect: effects) {
			effect->SetVisible(true);
		}
		return true;
	}

	if (!StartsWith(func, "effect_")) {
		return false;
	}

	bool okay;
	const int index = DynRpg::ParseArg<int>(func, args, 0, &okay);
	if (!okay) {
		return true;
	}

	if (index < 0 || index >= static_cast<int>(effects.size())) {
		return true;
	}
	ParticleEffectV1* effect = effects[index].get();

	if (!effect) {
		return true;
	}

	if (func == "effect_burst") {
		auto [x, y] = DynRpg::ParseArgs<int, int>(func, args, 1, &okay);
		effect->NewBurst(x, y);
		return true;
	}
	if (func == "effect_heart") {
		auto [x, y] = DynRpg::ParseArgs<int, int>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		effect->NewHeart(x, y);
		return true;
	}
	if (func == "effect_color") {
		auto [r, g, b] = DynRpg::ParseArgs<int, int, int>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		effect->red = r;
		effect->green = g;
		effect->blue = b;
		return true;
	}
	if (func == "effect_colorfade") {
		auto [r, g, b] = DynRpg::ParseArgs<int, int, int>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		effect->ColorFade(r, g, b);
		return true;
	}
	if (func == "effect_amount") {
		auto [amount] = DynRpg::ParseArgs<int>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		effect->amount = amount;
		return true;
	}
	if (func == "effect_random") {
		auto [position_x, position_y] = DynRpg::ParseArgs<int, int>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		effect->rand_x = position_x;
		effect->rand_y = position_y;
		return true;
	}
	if (func == "effect_angle") {
		auto [angle_begin, angle_end] = DynRpg::ParseArgs<float, float>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		effect->SetAngle(angle_begin, angle_end);
		return true;
	}
	if (func == "effect_growth") {
		auto [size_begin, size_end] = DynRpg::ParseArgs<int, int>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		effect->SetGrowth(size_begin, size_end);
		return true;
	}

	if (func == "effect_speed") {
		bool spd_okay = false;
		auto speed = DynRpg::ParseArg<float>(func, args, 1, &spd_okay);
		if (!spd_okay) {
			return true;
		}
		effect->spd = speed;

		// Check if a second argument (rand_spd) was provided; otherwise keep/default it safely
		if (args.size() > 2) {
			bool rand_okay = false;
			auto rand_speed = DynRpg::ParseArg<float>(func, args, 2, &rand_okay);
			if (rand_okay) {
				effect->rand_spd = rand_speed;
			}
		}
		return true;
	}


	if (func == "effect_timeout") {
		auto [timeout] = DynRpg::ParseArgs<float>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		effect->SetTimeout(timeout);
		return true;
	}
	if (func == "effect_delay") {
		auto [delay] = DynRpg::ParseArgs<float>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		effect->delay = delay;
		return true;
	}
	if (func == "effect_file") {
		auto [file, mask] = DynRpg::ParseArgs<std::string, std::string>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		effect->filename = file;
		if (Utils::LowerCase(mask).substr(0, 4) == "true") {
			effect->mask = true;
		}
		effect->has_texture = false;
		effect->LoadTexture();
		return true;
	}
	if (func == "effect_kill") {
		effect->KillParticles();
		return true;
	}

	return false;
}

void DynRpg::ParticleV1::Update() {
	if (!do_draw) {
		return;
	}

	for (auto& pfx : effects) {
		pfx->Update();
	}
}
