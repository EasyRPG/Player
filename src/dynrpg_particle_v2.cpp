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

// Ported from DynRPG Particle Effects V2 by Kazesui (MIT license).

#include "dynrpg_particle_v2.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include "async_handler.h"
#include "bitmap.h"
#include "cache.h"
#include "drawable.h"
#include "drawable_mgr.h"
#include "game_battle.h"
#include "game_map.h"
#include "game_switches.h"
#include "main_data.h"
#include "scene.h"

class ParticleEffect;

constexpr double kSinLutStep = 0.1963495408;

using EffectMap = std::map<std::string, std::unique_ptr<ParticleEffect>>;

EffectMap pfx_list;

class ParticleEffect : public Drawable {
public:
	ParticleEffect();
	~ParticleEffect() override;

	virtual void Update() = 0;
	void Draw(Bitmap& dst) override = 0;

	virtual void Clear() = 0;
	virtual void SetSimul(int new_simul) = 0;
	virtual void SetAmount(int new_amount) = 0;
	void SetAngle(float v1, float v2);
	void SetSecondaryAngle(float v);
	virtual void SetTimeout(int fade, int delay);
	void SetRad(int new_rad);
	void SetSpd(float new_spd);
	void SetGrowth(float ini_size, float end_size);
	void SetRandRad(int new_rnd_rad);
	void SetRandSpd(float new_rnd_spd);
	void SetRandPos(int new_rnd_x, int new_rnd_y);
	virtual void SetTexture(const std::string& filename);
	virtual void UnloadTexture();
	void UseScreenRelative(bool enabled);
	virtual void SetGeneratingFunction(const std::string& type) {}
	void SetGravityDirection(float angle, float factor);
	void SetAccelerationPoint(float x, float y, float factor);
	void SetColor0(uint8_t r, uint8_t g, uint8_t b);
	void SetColor1(uint8_t r, uint8_t g, uint8_t b);

	static void CreateTrigLut();

protected:
	bool is_screen_relative = false;

	int r0 = 50;
	int rand_r = 0;
	int rand_x = 0;
	int rand_y = 0;

	float spd = 0.5f;
	float rand_spd = 0.5f;
	float s0 = 1;
	float s1 = 1;
	float ds = 0;
	float da = 0;

	float gx = 0;
	float gy = 0;
	float ax0 = 0;
	float ay0 = 0;
	float afc = 0;

	BitmapRef image;
	bool has_texture = false;

	float beta = 6.2832f;
	float alpha = 0;
	float theta = 0;
	uint8_t fade = 30;
	uint8_t delay = 0;
	uint16_t amount = 50;
	uint32_t color0 = 0x00ffffff;
	uint32_t color1 = 0x00ffffff;

	std::array<Color, 256> palette;

	void UpdateColor();

	static float sin_lut[32];
};

float ParticleEffect::sin_lut[32];

ParticleEffect::ParticleEffect() : Drawable(Priority_Weather) {
	da = 255.0f / fade;
	image = Bitmap::Create(1, 1, true);

	DrawableMgr::Register(this);
}

ParticleEffect::~ParticleEffect() = default;

void ParticleEffect::SetTexture(const std::string& filename) {
	const std::string name = DynRpg::StripImageExtension(filename);
	FileRequestAsync* req = AsyncHandler::RequestFile("Picture", name);
	req->Start();
	image = Cache::Picture(name, true);
	has_texture = true;
}

void ParticleEffect::UnloadTexture() {
	image = Bitmap::Create(1, 1, true);
	has_texture = false;
	UpdateColor();
}

void ParticleEffect::SetGravityDirection(float angle, float factor) {
	angle *= 0.0174532925f;
	gx = factor * cosf(angle) / 600.0f;
	gy = factor * sinf(angle) / 600.0f;
}

void ParticleEffect::SetAccelerationPoint(float x, float y, float factor) {
	afc = factor / 600.0f;
	ax0 = x;
	ay0 = y;
}

void ParticleEffect::SetGrowth(float ini_size, float end_size) {
	s0 = ini_size;
	s1 = end_size;
	ds = (fade > 0) ? (s1 - s0) / fade : 0;
}

void ParticleEffect::UseScreenRelative(bool enabled) {
	is_screen_relative = enabled;
}

void ParticleEffect::SetAmount(int new_amount) {
	amount = new_amount;
}

void ParticleEffect::SetAngle(float v1, float v2) {
	v1 *= 0.0174532925f;
	v2 *= 0.0174532925f;
	beta = (v2 < 0) ? -v2 : v2;
	alpha = v1 - v2 / 2;
}

void ParticleEffect::SetSecondaryAngle(float v) {
	while (v > 360) {
		v -= 360;
	}
	while (v < -360) {
		v += 360;
	}
	theta = v * 0.0174532925f;
}

void ParticleEffect::SetSpd(float new_spd) {
	spd = new_spd / 60.0f;
}

void ParticleEffect::SetRandSpd(float new_rnd_spd) {
	rand_spd = new_rnd_spd / 60.0f;
}

void ParticleEffect::SetRad(int new_rad) {
	r0 = new_rad;
}

void ParticleEffect::SetRandPos(int new_rnd_x, int new_rnd_y) {
	rand_x = (new_rnd_x < 0) ? -new_rnd_x : new_rnd_x;
	rand_y = (new_rnd_y < 0) ? -new_rnd_y : new_rnd_y;
}

void ParticleEffect::SetRandRad(int new_rnd_rad) {
	rand_r = (new_rnd_rad < 0) ? -new_rnd_rad : new_rnd_rad;
}

void ParticleEffect::SetTimeout(int new_fade, int new_delay) {
	if (new_fade > 255) {
		new_fade = 255;
	} else if (new_fade <= 0) {
		new_fade = 1;
	}
	if (new_delay >= new_fade) {
		new_delay = new_fade - 1;
	} else if (new_delay < 0) {
		new_delay = 0;
	}
	fade = static_cast<uint8_t>(new_fade);
	delay = static_cast<uint8_t>(new_delay);
	da = 255.0f / new_fade;
	ds = (s1 - s0) / new_fade;
	UpdateColor();
}

void ParticleEffect::SetColor0(uint8_t r_val, uint8_t g_val, uint8_t b_val) {
	color0 = static_cast<uint32_t>(r_val) << 16 | static_cast<uint32_t>(g_val) << 8 | b_val;
	UpdateColor();
}

void ParticleEffect::SetColor1(uint8_t r_val, uint8_t g_val, uint8_t b_val) {
	color1 = static_cast<uint32_t>(r_val) << 16 | static_cast<uint32_t>(g_val) << 8 | b_val;
	UpdateColor();
}

void ParticleEffect::UpdateColor() {
	float r = static_cast<float>((color0 >> 16) & 0xff);
	float g = static_cast<float>((color0 >> 8) & 0xff);
	float b = static_cast<float>(color0 & 0xff);
	const float end_r = static_cast<float>((color1 >> 16) & 0xff);
	const float end_g = static_cast<float>((color1 >> 8) & 0xff);
	const float end_b = static_cast<float>(color1 & 0xff);

	int hold = delay;
	if (hold >= fade) {
		hold = fade - 1;
	}

	float dr = 0;
	float dg = 0;
	float db = 0;
	if (fade - hold > 0) {
		dr = (end_r - r) / (fade - hold);
		dg = (end_g - g) / (fade - hold);
		db = (end_b - b) / (fade - hold);
	}

	int i = 0;
	for (; i < hold; ++i) {
		palette[i] = Color(r, g, b, 255);
	}
	for (; i < fade; ++i) {
		palette[i] = Color(r, g, b, 255);
		r += dr;
		g += dg;
		b += db;
	}
}

void ParticleEffect::CreateTrigLut() {
	for (int i = 0; i < 32; ++i) {
		sin_lut[i] = static_cast<float>(std::sin(M_PI / 16.0 * i));
	}
}

class Burst : public ParticleEffect {
public:
	Burst();
	~Burst() override = default;

	void Update() override;
	void Draw(Bitmap& dst) override;
	void Clear() override;

	void NewBurst(int x, int y);

	void SetSimul(int new_simul) override;
	void SetAmount(int new_amount) override;
	void SetTimeout(int fade, int delay) override;
	void SetGeneratingFunction(const std::string& type) override;

private:
	uint16_t simul_cnt = 0;
	uint16_t simul_max = 1;

	std::vector<float> x, y, s, dx, dy;
	std::vector<uint8_t> itr;

	void Reallocate(bool preserve);

	void (Burst::*init)(int, int, int, int);

	void InitBasic(int x0, int y0, int a, int b);
	void InitRadial(int x0, int y0, int a, int b);
};

Burst::Burst() : init(&Burst::InitBasic) {
	Reallocate(false);
	UpdateColor();
}

void Burst::Reallocate(bool preserve) {
	const size_t pool_size = static_cast<size_t>(amount) * simul_max;

	x.resize(pool_size, 0.0f);
	y.resize(pool_size, 0.0f);
	s.resize(pool_size, 0.0f);
	dx.resize(pool_size, 0.0f);
	dy.resize(pool_size, 0.0f);
	itr.resize(simul_max, 0);

	if (!preserve) {
		simul_cnt = 0;
	}
}

void Burst::SetAmount(int new_amount) {
	amount = static_cast<uint16_t>(new_amount);
	Reallocate(false);
}

void Burst::SetSimul(int new_simul) {
	simul_max = static_cast<uint16_t>(new_simul);
	Reallocate(false);
}

void Burst::SetTimeout(int new_fade, int new_delay) {
	ParticleEffect::SetTimeout(new_fade, new_delay);
	Reallocate(false);
}

void Burst::SetGeneratingFunction(const std::string& type) {
	std::string name = type;
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);
	if (!name.compare(0, 8, "standard")) {
		init = &Burst::InitBasic;
		return;
	}
	if (!name.compare(0, 6, "radial")) {
		init = &Burst::InitRadial;
		return;
	}
}

void Burst::Clear() {
	simul_cnt = 0;
}

void Burst::NewBurst(int x0, int y0) {
	if (simul_cnt >= simul_max) {
		simul_max *= 2;
		Reallocate(true);
	}

	itr[simul_cnt] = 0;

	(this->*init)(x0, y0, simul_cnt * amount, (simul_cnt + 1) * amount);
	++simul_cnt;
}

void Burst::InitBasic(int x0, int y0, int a, int b) {
	for (int i = a; i < b; ++i) {
		x[i] = x0 + 2 * rand_x * static_cast<float>(rand()) / RAND_MAX - rand_x;
		y[i] = y0 + 2 * rand_y * static_cast<float>(rand()) / RAND_MAX - rand_y;
		s[i] = s0;

		float tmp_angle = static_cast<float>(rand()) / RAND_MAX * beta + alpha;
		float tmp_spd = spd + rand_spd * static_cast<float>(rand()) / RAND_MAX;
		const int v = static_cast<int>(tmp_angle / kSinLutStep);
		tmp_angle = (tmp_angle - v * kSinLutStep) / kSinLutStep;
		dx[i] = tmp_spd * (sin_lut[(v + 9) & 31] * tmp_angle + sin_lut[(v + 8) & 31] * (1 - tmp_angle));
		dy[i] = tmp_spd * (sin_lut[(v + 1) & 31] * tmp_angle + sin_lut[(v + 0) & 31] * (1 - tmp_angle));
	}
}

void Burst::InitRadial(int x0, int y0, int a, int b) {
	for (int i = a; i < b; ++i) {
		const float tmp_rnd = rand_r * static_cast<float>(rand()) / RAND_MAX;
		const float tmp_angle = static_cast<float>(rand()) / RAND_MAX * beta + alpha;
		const float tmp_spd = spd + rand_spd * static_cast<float>(rand()) / RAND_MAX;
		int v = static_cast<int>(tmp_angle / kSinLutStep);
		float p = (tmp_angle - v * kSinLutStep) / kSinLutStep;

		x[i] = x0 + (r0 + tmp_rnd) * (sin_lut[(v + 9) & 31] * p + sin_lut[(v + 8) & 31] * (1 - p));
		y[i] = y0 + (r0 + tmp_rnd) * (sin_lut[(v + 1) & 31] * p + sin_lut[(v + 0) & 31] * (1 - p));
		s[i] = s0;

		v = static_cast<int>((tmp_angle + theta) / kSinLutStep);
		p = (tmp_angle + theta - v * kSinLutStep) / kSinLutStep;
		dx[i] = -tmp_spd * (sin_lut[(v + 9) & 31] * p + sin_lut[(v + 8) & 31] * (1 - p));
		dy[i] = -tmp_spd * (sin_lut[(v + 1) & 31] * p + sin_lut[(v + 0) & 31] * (1 - p));
	}
}

void Burst::Update() {
	if (simul_cnt <= 0) {
		return;
	}

	for (int i = 0; i < simul_cnt; ++i) {
		if (itr[i] >= fade) {
			--simul_cnt;
			if (i < simul_cnt) {
				const size_t dead_offset = i * amount;
				const size_t last_offset = simul_cnt * amount;
				std::copy_n(&x[last_offset], amount, &x[dead_offset]);
				std::copy_n(&y[last_offset], amount, &y[dead_offset]);
				std::copy_n(&s[last_offset], amount, &s[dead_offset]);
				std::copy_n(&dx[last_offset], amount, &dx[dead_offset]);
				std::copy_n(&dy[last_offset], amount, &dy[dead_offset]);
				itr[i] = itr[simul_cnt];
			}
			--i;
			continue;
		}

		++itr[i];

		for (int j = i * amount; j < (i + 1) * amount; ++j) {
			x[j] += dx[j];
			y[j] += dy[j];
			const float tx = ax0 - x[j];
			const float ty = ay0 - y[j];
			const float tsqr = sqrtf(tx * tx + ty * ty + 0.001f);
			dx[j] += gx + afc * tx / tsqr;
			dy[j] += gy + afc * ty / tsqr;
			s[j] += ds;
		}
	}
}

void Burst::Draw(Bitmap& dst) {
	if (simul_cnt <= 0) {
		return;
	}

	const int cam_x = is_screen_relative ? 0 : Game_Map::GetDisplayX() / 16;
	const int cam_y = is_screen_relative ? 0 : Game_Map::GetDisplayY() / 16;

	for (int i = 0; i < simul_cnt; ++i) {
		const int age = itr[i];
		if (age >= fade) {
			continue;
		}

		const int alpha = static_cast<int>(255 - da * age);
		const Color color = palette[age];

		for (int j = i * amount; j < (i + 1) * amount; ++j) {
			const Rect dst_rect(static_cast<int>(x[j] - cam_x - s[j] / 2),
				static_cast<int>(y[j] - cam_y - s[j] / 2),
				static_cast<int>(s[j]), static_cast<int>(s[j]));

			if (has_texture) {
				dst.StretchBlit(dst_rect, *image, image->GetRect(), Opacity(alpha));
			} else {
				dst.FillRect(dst_rect, Color(color.red, color.green, color.blue, alpha));
			}
		}
	}
}

class Stream : public ParticleEffect {
public:
	Stream();
	~Stream() override = default;

	void Update() override;
	void Draw(Bitmap& dst) override;
	void Clear() override;
	void StopAll();
	void Stop(const std::string& tag);
	void Start(int x, int y, const std::string& tag);

	void SetSimul(int new_simul) override;
	void SetAmount(int new_amount) override;
	void SetTimeout(int fade, int delay) override;
	void SetGeneratingFunction(const std::string& type) override;
	void SetPosition(const std::string& tag, int x_pos, int y_pos);

private:
	uint16_t simul_beg = 0;
	uint16_t simul_run = 0;
	uint16_t simul_cnt = 0;
	uint16_t simul_max = 1;

	std::vector<float> x, y, s, dx, dy;
	std::vector<uint8_t> itr, end_cnt;
	std::vector<int16_t> str_x, str_y;
	std::vector<uint16_t> pfx_ref;

	std::map<std::string, int> pfx_tag;

	void Reallocate(bool preserve);
	void StreamToEnd(uint16_t idx);
	void StartToStream(uint16_t idx);

	void (Stream::*init)(int, int, int);

	void InitBasic(int a, int b, int idx);
	void InitRadial(int a, int b, int idx);
};

Stream::Stream() : init(&Stream::InitBasic) {
	amount = 10;
	Reallocate(false);
	UpdateColor();
}

void Stream::Reallocate(bool preserve) {
	const size_t pool_size = static_cast<size_t>(amount) * fade * simul_max;

	x.resize(pool_size, 0.0f);
	y.resize(pool_size, 0.0f);
	s.resize(pool_size, 0.0f);
	dx.resize(pool_size, 0.0f);
	dy.resize(pool_size, 0.0f);
	itr.resize(simul_max, 0);
	str_x.resize(simul_max, 0);
	str_y.resize(simul_max, 0);
	end_cnt.resize(simul_max, 0);

	if (preserve) {
		const size_t old_max = pfx_ref.size();
		pfx_ref.resize(simul_max);
		for (uint16_t i = static_cast<uint16_t>(old_max); i < simul_max; ++i) {
			pfx_ref[i] = i;
		}
		return;
	}

	pfx_ref.resize(simul_max);
	for (uint16_t i = 0; i < simul_max; ++i) {
		pfx_ref[i] = i;
	}
	simul_beg = 0;
	simul_run = 0;
	simul_cnt = 0;
	pfx_tag.clear();
}

void Stream::Start(int x0, int y0, const std::string& tag) {
	if (pfx_tag.count(tag)) {
		return;
	}
	if (simul_cnt >= simul_max) {
		simul_max *= 2;
		Reallocate(true);
	}

	const uint16_t idx = pfx_ref[simul_cnt];

	std::swap(pfx_ref[simul_cnt], pfx_ref[simul_run]);
	std::swap(pfx_ref[simul_run], pfx_ref[simul_beg]);

	pfx_tag[tag] = idx;

	end_cnt[idx] = fade - 1;
	str_x[idx] = static_cast<int16_t>(x0);
	str_y[idx] = static_cast<int16_t>(y0);
	itr[idx] = 0;
	++simul_beg;
	++simul_run;
	++simul_cnt;
}

void Stream::Stop(const std::string& tag) {
	auto pfx_itr = pfx_tag.find(tag);
	if (pfx_itr == pfx_tag.end()) {
		return;
	}
	const uint8_t probe = static_cast<uint8_t>(pfx_itr->second);

	auto it = std::find(pfx_ref.begin(), pfx_ref.begin() + simul_cnt, probe);
	if (it == pfx_ref.begin() + simul_cnt) {
		return;
	}

	--simul_run;
	std::swap(*it, pfx_ref[simul_run]);
	pfx_tag.erase(pfx_itr);
}

void Stream::StopAll() {
	simul_beg = 0;
	simul_run = 0;
	pfx_tag.clear();
}

void Stream::Clear() {
	simul_beg = 0;
	simul_run = 0;
	simul_cnt = 0;
	pfx_tag.clear();
}

void Stream::SetGeneratingFunction(const std::string& type) {
	std::string name = type;
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);
	if (!name.compare(0, 8, "standard")) {
		init = &Stream::InitBasic;
		return;
	}
	if (!name.compare(0, 6, "radial")) {
		init = &Stream::InitRadial;
		return;
	}
}

void Stream::InitBasic(int a, int b, int idx) {
	const float x0 = str_x[idx];
	const float y0 = str_y[idx];
	for (int i = a; i < b; ++i) {
		x[i] = x0 + 2 * rand_x * static_cast<float>(rand()) / RAND_MAX - rand_x;
		y[i] = y0 + 2 * rand_y * static_cast<float>(rand()) / RAND_MAX - rand_y;
		s[i] = s0;

		float tmp_angle = static_cast<float>(rand()) / RAND_MAX * beta + alpha;
		const float tmp_spd = spd + rand_spd * static_cast<float>(rand()) / RAND_MAX;
		const int v = static_cast<int>(tmp_angle / kSinLutStep);
		tmp_angle = (tmp_angle - v * kSinLutStep) / kSinLutStep;
		dx[i] = tmp_spd * (sin_lut[(v + 9) & 31] * tmp_angle + sin_lut[(v + 8) & 31] * (1 - tmp_angle));
		dy[i] = tmp_spd * (sin_lut[(v + 1) & 31] * tmp_angle + sin_lut[(v + 0) & 31] * (1 - tmp_angle));
	}
}

void Stream::InitRadial(int a, int b, int idx) {
	const float x0 = str_x[idx];
	const float y0 = str_y[idx];
	for (int i = a; i < b; ++i) {
		const float tmp_rnd = rand_r * static_cast<float>(rand()) / RAND_MAX;
		const float tmp_angle = static_cast<float>(rand()) / RAND_MAX * beta + alpha;
		const float tmp_spd = spd + rand_spd * static_cast<float>(rand()) / RAND_MAX;
		int v = static_cast<int>(tmp_angle / kSinLutStep);
		float p = (tmp_angle - v * kSinLutStep) / kSinLutStep;

		x[i] = x0 + (r0 + tmp_rnd) * (sin_lut[(v + 9) & 31] * p + sin_lut[(v + 8) & 31] * (1 - p));
		y[i] = y0 + (r0 + tmp_rnd) * (sin_lut[(v + 1) & 31] * p + sin_lut[(v + 0) & 31] * (1 - p));
		s[i] = s0;

		v = static_cast<int>((tmp_angle + theta) / kSinLutStep);
		p = (tmp_angle + theta - v * kSinLutStep) / kSinLutStep;
		dx[i] = -tmp_spd * (sin_lut[(v + 9) & 31] * p + sin_lut[(v + 8) & 31] * (1 - p));
		dy[i] = -tmp_spd * (sin_lut[(v + 1) & 31] * p + sin_lut[(v + 0) & 31] * (1 - p));
	}
}

void Stream::SetPosition(const std::string& tag, int x_pos, int y_pos) {
	auto pfx_itr = pfx_tag.find(tag);
	if (pfx_itr == pfx_tag.end()) {
		return;
	}
	const uint8_t probe = static_cast<uint8_t>(pfx_itr->second);

	auto it = std::find(pfx_ref.begin(), pfx_ref.begin() + simul_cnt, probe);
	if (it == pfx_ref.begin() + simul_cnt) {
		return;
	}

	str_x[probe] = static_cast<int16_t>(x_pos);
	str_y[probe] = static_cast<int16_t>(y_pos);
}

void Stream::SetAmount(int new_amount) {
	amount = static_cast<uint16_t>(new_amount);
	Reallocate(false);
}

void Stream::SetSimul(int new_simul) {
	simul_max = static_cast<uint16_t>(new_simul);
	Reallocate(false);
}

void Stream::SetTimeout(int new_fade, int new_delay) {
	ParticleEffect::SetTimeout(new_fade, new_delay);
	Reallocate(false);
}

void Stream::Update() {
	if (simul_cnt <= 0) {
		return;
	}

	const int block_size = amount * fade;

	for (int i = 0; i < simul_cnt; ++i) {
		const uint16_t p_ref = pfx_ref[i];
		const int base_idx = p_ref * block_size;
		for (int j = 0; j < block_size; ++j) {
			const int p_idx = base_idx + j;
			x[p_idx] += dx[p_idx];
			y[p_idx] += dy[p_idx];
			const float tx = ax0 - x[p_idx];
			const float ty = ay0 - y[p_idx];
			const float tsqr = sqrtf(tx * tx + ty * ty + 0.001f);
			dx[p_idx] += gx + afc * tx / tsqr;
			dy[p_idx] += gy + afc * ty / tsqr;
			s[p_idx] += ds;
		}
	}

	// Starting
	for (int i = 0; i < simul_beg; ++i) {
		const uint16_t idx = pfx_ref[i];
		if (itr[idx] < fade) {
			const uint8_t z = fade - itr[idx] - 1;
			(this->*init)(z * amount + idx * block_size, (z + 1) * amount + idx * block_size, idx);
			++itr[idx];
		} else {
			StartToStream(static_cast<uint16_t>(i--));
		}
	}

	// Running
	for (int i = simul_beg; i < simul_run; ++i) {
		const uint16_t idx = pfx_ref[i];
		const uint8_t z = fade - itr[idx] - 1;
		itr[idx] = (itr[idx] + 1) % fade;
		(this->*init)(z * amount + idx * block_size, (z + 1) * amount + idx * block_size, idx);
	}

	// Stopping
	for (int i = simul_run; i < simul_cnt; ++i) {
		const uint16_t idx = pfx_ref[i];
		if (end_cnt[idx] == 0) {
			StreamToEnd(static_cast<uint16_t>(i--));
		} else {
			--end_cnt[idx];
		}
	}
}

void Stream::Draw(Bitmap& dst) {
	if (simul_cnt <= 0) {
		return;
	}

	const int cam_x = is_screen_relative ? 0 : Game_Map::GetDisplayX() / 16;
	const int cam_y = is_screen_relative ? 0 : Game_Map::GetDisplayY() / 16;
	const int block_size = amount * fade;

	auto draw_block = [&](uint16_t idx, uint8_t n, uint8_t z, uint8_t c0) {
		for (uint8_t i = 0; i < n; ++i) {
			const int age = i + c0;
			if (age >= fade) {
				continue;
			}

			const int alpha = static_cast<int>(255 - da * age);
			const Color color = palette[age];
			const int block_start = idx * block_size + z * amount;

			for (int j = 0; j < amount; ++j) {
				const int p_idx = block_start + j;
				const float size = s[p_idx];
				const Rect dst_rect(static_cast<int>(x[p_idx] - cam_x - size / 2.0f),
					static_cast<int>(y[p_idx] - cam_y - size / 2.0f),
					static_cast<int>(size), static_cast<int>(size));

				if (has_texture) {
					dst.StretchBlit(dst_rect, *image, image->GetRect(), Opacity(alpha));
				} else {
					dst.FillRect(dst_rect, Color(color.red, color.green, color.blue, alpha));
				}
			}
			z = (z + 1) % fade;
		}
	};

	// Starting
	for (int i = 0; i < simul_beg; ++i) {
		const uint16_t idx = pfx_ref[i];
		draw_block(idx, itr[idx], fade - itr[idx], 0);
	}

	// Running
	for (int i = simul_beg; i < simul_run; ++i) {
		const uint16_t idx = pfx_ref[i];
		const uint8_t z_old = (itr[idx] == 0) ? 0 : fade - itr[idx];
		draw_block(idx, fade, z_old, 0);
	}

	// Stopping
	for (int i = simul_run; i < simul_cnt; ++i) {
		const uint16_t idx = pfx_ref[i];
		const uint8_t count = end_cnt[idx] + 1;
		const uint8_t z_old = (fade - itr[idx]) % fade;
		draw_block(idx, count, z_old, fade - count);
	}
}

void Stream::StartToStream(uint16_t idx) {
	itr[pfx_ref[idx]] = 0;
	--simul_beg;
	std::swap(pfx_ref[simul_beg], pfx_ref[idx]);
}

void Stream::StreamToEnd(uint16_t idx) {
	--simul_cnt;
	std::swap(pfx_ref[simul_cnt], pfx_ref[idx]);
}

namespace {
	std::map<std::string_view, std::function<bool(dyn_arg_list)>> function_list;

	ParticleEffect* GetPfx(const std::string& tag) {
		auto it = pfx_list.find(tag);
		if (it != pfx_list.end()) {
			return it->second.get();
		}
		Output::Debug("DynParticle: Particle effect with tag '{}' not found.", tag);
		return nullptr;
	}
}

static bool CreateEffect(dyn_arg_list args) {
	auto func = "pfx_create_effect";
	bool okay;
	std::string tag, type;
	std::tie(tag, type) = DynRpg::ParseArgs<std::string, std::string>(func, args, &okay);
	if (!okay) {
		return true;
	}
	if (pfx_list.count(tag)) {
		return true;
	}
	std::transform(type.begin(), type.end(), type.begin(), ::tolower);
	if (!type.compare(0, 5, "burst")) {
		pfx_list[tag] = std::make_unique<Burst>();
	} else if (!type.compare(0, 6, "stream")) {
		pfx_list[tag] = std::make_unique<Stream>();
	}
	return true;
}

static bool DestroyEffect(dyn_arg_list args) {
	auto func = "pfx_destroy_effect";
	bool okay;
	auto [tag] = DynRpg::ParseArgs<std::string>(func, args, &okay);
	if (!okay) {
		return true;
	}
	pfx_list.erase(tag);
	return true;
}

static bool DestroyAll(dyn_arg_list) {
	pfx_list.clear();
	return true;
}

static bool DoesEffectExist(dyn_arg_list args) {
	auto func = "pfx_does_effect_exist";
	bool okay;
	std::string tag;
	int idx;
	std::tie(tag, idx) = DynRpg::ParseArgs<std::string, int>(func, args, &okay);
	if (!okay) {
		return true;
	}
	Main_Data::game_switches->Set(idx, pfx_list.count(tag) > 0);
	Game_Map::SetNeedRefresh(true);
	return true;
}

static bool BurstCmd(dyn_arg_list args) {
	auto func = "pfx_burst";
	bool okay;
	std::string tag;
	int x, y;
	std::tie(tag, x, y) = DynRpg::ParseArgs<std::string, int, int>(func, args, &okay);
	if (!okay) {
		return true;
	}
	if (auto pfx = GetPfx(tag)) {
		if (auto b = dynamic_cast<Burst*>(pfx)) {
			b->NewBurst(x, y);
		}
	}
	return true;
}

static bool StartCmd(dyn_arg_list args) {
	auto func = "pfx_start";
	bool okay;
	std::string tag1, tag2;
	int x, y;
	std::tie(tag1, tag2, x, y) = DynRpg::ParseArgs<std::string, std::string, int, int>(func, args, &okay);
	if (!okay) {
		return true;
	}
	if (auto pfx = GetPfx(tag1)) {
		if (auto s = dynamic_cast<Stream*>(pfx)) {
			s->Start(x, y, tag2);
		}
	}
	return true;
}

static bool StopCmd(dyn_arg_list args) {
	auto func = "pfx_stop";
	bool okay;
	std::string tag1, tag2;
	std::tie(tag1, tag2) = DynRpg::ParseArgs<std::string, std::string>(func, args, &okay);
	if (!okay) {
		return true;
	}
	if (auto pfx = GetPfx(tag1)) {
		if (auto s = dynamic_cast<Stream*>(pfx)) {
			s->Stop(tag2);
		}
	}
	return true;
}

static bool StopAllCmd(dyn_arg_list args) {
	auto func = "pfx_stopall";
	bool okay;
	auto [tag] = DynRpg::ParseArgs<std::string>(func, args, &okay);
	if (!okay) {
		return true;
	}
	if (auto pfx = GetPfx(tag)) {
		if (auto s = dynamic_cast<Stream*>(pfx)) {
			s->StopAll();
		}
	}
	return true;
}

static bool SetPositionCmd(dyn_arg_list args) {
	auto func = "pfx_set_position";
	bool okay;
	std::string tag1, tag2;
	int x, y;
	std::tie(tag1, tag2, x, y) = DynRpg::ParseArgs<std::string, std::string, int, int>(func, args, &okay);
	if (!okay) {
		return true;
	}
	if (auto pfx = GetPfx(tag1)) {
		if (auto s = dynamic_cast<Stream*>(pfx)) {
			s->SetPosition(tag2, x, y);
		}
	}
	return true;
}

static bool LoadEffectCmd(dyn_arg_list) {
	return true;
}

DynRpg::ParticleV2::ParticleV2(Game_DynRpg& instance) : DynRpgPlugin("KazeParticles", instance) {
	ParticleEffect::CreateTrigLut();

	if (!function_list.empty()) {
		return;
	}

	function_list["pfx_destroy_all"] = &DestroyAll;
	function_list["pfx_create_effect"] = &CreateEffect;
	function_list["pfx_destroy_effect"] = &DestroyEffect;
	function_list["pfx_does_effect_exist"] = &DoesEffectExist;
	function_list["pfx_burst"] = &BurstCmd;
	function_list["pfx_start"] = &StartCmd;
	function_list["pfx_stop"] = &StopCmd;
	function_list["pfx_stopall"] = &StopAllCmd;
	function_list["pfx_set_position"] = &SetPositionCmd;
	function_list["pfx_load_effect"] = &LoadEffectCmd;

	auto add_setter_1_int = [](std::string_view name, void (ParticleEffect::*setter)(int)) {
		function_list[name] = [name, setter](dyn_arg_list args) {
			bool okay;
			std::string tag;
			int val;
			std::tie(tag, val) = DynRpg::ParseArgs<std::string, int>(name, args, &okay);
			if (!okay) {
				return true;
			}
			if (auto pfx = GetPfx(tag)) {
				(pfx->*setter)(val);
			}
			return true;
		};
	};
	auto add_setter_2_int = [](std::string_view name, void (ParticleEffect::*setter)(int, int)) {
		function_list[name] = [name, setter](dyn_arg_list args) {
			bool okay;
			std::string tag;
			int val1, val2;
			std::tie(tag, val1, val2) = DynRpg::ParseArgs<std::string, int, int>(name, args, &okay);
			if (!okay) {
				return true;
			}
			if (auto pfx = GetPfx(tag)) {
				(pfx->*setter)(val1, val2);
			}
			return true;
		};
	};
	auto add_setter_2_float = [](std::string_view name, void (ParticleEffect::*setter)(float, float)) {
		function_list[name] = [name, setter](dyn_arg_list args) {
			bool okay;
			std::string tag;
			float val1, val2;
			std::tie(tag, val1, val2) = DynRpg::ParseArgs<std::string, float, float>(name, args, &okay);
			if (!okay) {
				return true;
			}
			if (auto pfx = GetPfx(tag)) {
				(pfx->*setter)(val1, val2);
			}
			return true;
		};
	};
	auto add_setter_3_int = [](std::string_view name, void (ParticleEffect::*setter)(uint8_t, uint8_t, uint8_t)) {
		function_list[name] = [name, setter](dyn_arg_list args) {
			bool okay;
			std::string tag;
			int r, g, b;
			std::tie(tag, r, g, b) = DynRpg::ParseArgs<std::string, int, int, int>(name, args, &okay);
			if (!okay) {
				return true;
			}
			if (auto pfx = GetPfx(tag)) {
				(pfx->*setter)(static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b));
			}
			return true;
		};
	};

	add_setter_1_int("pfx_set_simul_effects", &ParticleEffect::SetSimul);
	add_setter_1_int("pfx_set_amount", &ParticleEffect::SetAmount);
	add_setter_2_int("pfx_set_timeout", &ParticleEffect::SetTimeout);
	add_setter_1_int("pfx_set_random_radius", &ParticleEffect::SetRandRad);
	add_setter_1_int("pfx_set_radius", &ParticleEffect::SetRad);
	add_setter_2_int("pfx_set_random_position", &ParticleEffect::SetRandPos);
	add_setter_3_int("pfx_set_initial_color", &ParticleEffect::SetColor0);
	add_setter_3_int("pfx_set_final_color", &ParticleEffect::SetColor1);
	add_setter_2_float("pfx_set_growth", &ParticleEffect::SetGrowth);
	add_setter_2_float("pfx_set_angle", &ParticleEffect::SetAngle);

	function_list["pfx_set_velocity"] = [](dyn_arg_list args) {
		bool okay;
		std::string tag;
		float speed, rand_speed;
		std::tie(tag, speed, rand_speed) = DynRpg::ParseArgs<std::string, float, float>("pfx_set_velocity", args, &okay);
		if (!okay) {
			return true;
		}
		if (auto pfx = GetPfx(tag)) {
			pfx->SetSpd(speed);
			pfx->SetRandSpd(rand_speed);
		}
		return true;
	};
	function_list["pfx_set_texture"] = [](dyn_arg_list args) {
		bool okay;
		std::string tag, texture;
		std::tie(tag, texture) = DynRpg::ParseArgs<std::string, std::string>("pfx_set_texture", args, &okay);
		if (!okay) {
			return true;
		}
		if (auto pfx = GetPfx(tag)) {
			pfx->SetTexture(texture);
		}
		return true;
	};
	function_list["pfx_set_acceleration_point"] = [](dyn_arg_list args) {
		bool okay;
		std::string tag;
		float x, y, force;
		std::tie(tag, x, y, force) =
			DynRpg::ParseArgs<std::string, float, float, float>("pfx_set_acceleration_point", args, &okay);
		if (!okay) {
			return true;
		}
		if (auto pfx = GetPfx(tag)) {
			pfx->SetAccelerationPoint(x, y, force);
		}
		return true;
	};
	function_list["pfx_set_gravity_direction"] = [](dyn_arg_list args) {
		bool okay;
		std::string tag;
		float angle, force;
		std::tie(tag, angle, force) = DynRpg::ParseArgs<std::string, float, float>("pfx_set_gravity_direction", args, &okay);
		if (!okay) {
			return true;
		}
		if (auto pfx = GetPfx(tag)) {
			pfx->SetGravityDirection(angle, force);
		}
		return true;
	};
	function_list["pfx_set_secondary_angle"] = [](dyn_arg_list args) {
		bool okay;
		std::string tag;
		float angle;
		std::tie(tag, angle) = DynRpg::ParseArgs<std::string, float>("pfx_set_secondary_angle", args, &okay);
		if (!okay) {
			return true;
		}
		if (auto pfx = GetPfx(tag)) {
			pfx->SetSecondaryAngle(angle);
		}
		return true;
	};
	function_list["pfx_set_generating_function"] = [](dyn_arg_list args) {
		bool okay;
		std::string tag, type;
		std::tie(tag, type) = DynRpg::ParseArgs<std::string, std::string>("pfx_set_generating_function", args, &okay);
		if (!okay) {
			return true;
		}
		if (auto pfx = GetPfx(tag)) {
			pfx->SetGeneratingFunction(type);
		}
		return true;
	};
	function_list["pfx_unload_texture"] = [](dyn_arg_list args) {
		bool okay;
		std::string tag;
		std::tie(tag) = DynRpg::ParseArgs<std::string>("pfx_unload_texture", args, &okay);
		if (!okay) {
			return true;
		}
		if (auto pfx = GetPfx(tag)) {
			pfx->UnloadTexture();
		}
		return true;
	};
	function_list["pfx_use_screen_relative"] = [](dyn_arg_list args) {
		bool okay;
		std::string tag, val;
		std::tie(tag, val) = DynRpg::ParseArgs<std::string, std::string>("pfx_use_screen_relative", args, &okay);
		if (!okay) {
			return true;
		}
		if (auto pfx = GetPfx(tag)) {
			pfx->UseScreenRelative(!val.empty() && (val[0] == 't' || val[0] == 'T'));
		}
		return true;
	};
}

DynRpg::ParticleV2::~ParticleV2() {
	DestroyAll({});
}

bool DynRpg::ParticleV2::Invoke(std::string_view func, dyn_arg_list args, bool& /*do_yield*/,
	Game_Interpreter* /*interpreter*/) {
	auto it = function_list.find(func);
	if (it != function_list.end()) {
		return it->second(args);
	}
	return false;
}

void DynRpg::ParticleV2::Update() {
	if (pfx_list.empty()) {
		return;
	}
	if ((Scene::instance && Scene::instance->type == Scene::Map) || Game_Battle::IsBattleRunning()) {
		for (auto& entry : pfx_list) {
			entry.second->Update();
		}
	}
}

void DynRpg::ParticleV2::OnMapChange() {
	for (auto& entry : pfx_list) {
		entry.second->Clear();
	}
}
