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
#include <map>
#include <memory>
#include <string>
#include <string_view>
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
#include "output.h"
#include "scene.h"

constexpr double kSinLutStep = 0.1963495408;

class ParticleEffectV2 : public Drawable {
public:
	enum class Type {
		Burst,
		Stream
	};

	~ParticleEffectV2() override;

	Type GetType() const { return type; }

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
	virtual void SetGeneratingFunction(const std::string&) {}
	void SetGravityDirection(float angle, float factor);
	void SetAccelerationPoint(float x, float y, float factor);
	void SetColor0(uint8_t r, uint8_t g, uint8_t b);
	void SetColor1(uint8_t r, uint8_t g, uint8_t b);

	static void CreateTrigLut();

protected:
	explicit ParticleEffectV2(Type type);

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

private:
	Type type;
};

float ParticleEffectV2::sin_lut[32];

ParticleEffectV2::ParticleEffectV2(Type type) : Drawable(Priority_Weather), type(type) {
	da = 255.0f / fade;
	image = Bitmap::Create(1, 1, true);

	DrawableMgr::Register(this);
}

ParticleEffectV2::~ParticleEffectV2() = default;

void ParticleEffectV2::SetTexture(const std::string& filename) {
	const std::string name = DynRpg::StripImageExtension(filename);
	FileRequestAsync* req = AsyncHandler::RequestFile("Picture", name);
	req->Start();
	image = Cache::Picture(name, true);
	has_texture = true;
}

void ParticleEffectV2::UnloadTexture() {
	image = Bitmap::Create(1, 1, true);
	has_texture = false;
	UpdateColor();
}

void ParticleEffectV2::SetGravityDirection(float angle, float factor) {
	angle *= 0.0174532925f;
	gx = factor * cosf(angle) / 600.0f;
	gy = factor * sinf(angle) / 600.0f;
}

void ParticleEffectV2::SetAccelerationPoint(float x, float y, float factor) {
	afc = factor / 600.0f;
	ax0 = x;
	ay0 = y;
}

void ParticleEffectV2::SetGrowth(float ini_size, float end_size) {
	s0 = ini_size;
	s1 = end_size;
	ds = (fade > 0) ? (s1 - s0) / fade : 0;
}

void ParticleEffectV2::UseScreenRelative(bool enabled) {
	is_screen_relative = enabled;
}

void ParticleEffectV2::SetAmount(int new_amount) {
	amount = new_amount;
}

void ParticleEffectV2::SetAngle(float v1, float v2) {
	v1 *= 0.0174532925f;
	v2 *= 0.0174532925f;
	beta = (v2 < 0) ? -v2 : v2;
	alpha = v1 - v2 / 2;
}

void ParticleEffectV2::SetSecondaryAngle(float v) {
	while (v > 360) {
		v -= 360;
	}
	while (v < -360) {
		v += 360;
	}
	theta = v * 0.0174532925f;
}

void ParticleEffectV2::SetSpd(float new_spd) {
	spd = new_spd / 60.0f;
}

void ParticleEffectV2::SetRandSpd(float new_rnd_spd) {
	rand_spd = new_rnd_spd / 60.0f;
}

void ParticleEffectV2::SetRad(int new_rad) {
	r0 = new_rad;
}

void ParticleEffectV2::SetRandPos(int new_rnd_x, int new_rnd_y) {
	rand_x = (new_rnd_x < 0) ? -new_rnd_x : new_rnd_x;
	rand_y = (new_rnd_y < 0) ? -new_rnd_y : new_rnd_y;
}

void ParticleEffectV2::SetRandRad(int new_rnd_rad) {
	rand_r = (new_rnd_rad < 0) ? -new_rnd_rad : new_rnd_rad;
}

void ParticleEffectV2::SetTimeout(int new_fade, int new_delay) {
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

void ParticleEffectV2::SetColor0(uint8_t r_val, uint8_t g_val, uint8_t b_val) {
	color0 = static_cast<uint32_t>(r_val) << 16 | static_cast<uint32_t>(g_val) << 8 | b_val;
	UpdateColor();
}

void ParticleEffectV2::SetColor1(uint8_t r_val, uint8_t g_val, uint8_t b_val) {
	color1 = static_cast<uint32_t>(r_val) << 16 | static_cast<uint32_t>(g_val) << 8 | b_val;
	UpdateColor();
}

void ParticleEffectV2::UpdateColor() {
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

void ParticleEffectV2::CreateTrigLut() {
	for (int i = 0; i < 32; ++i) {
		sin_lut[i] = static_cast<float>(std::sin(M_PI / 16.0 * i));
	}
}

class Burst : public ParticleEffectV2 {
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

Burst::Burst() : ParticleEffectV2(Type::Burst), init(&Burst::InitBasic) {
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
	ParticleEffectV2::SetTimeout(new_fade, new_delay);
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

class Stream : public ParticleEffectV2 {
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

Stream::Stream() : ParticleEffectV2(Type::Stream), init(&Stream::InitBasic) {
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
	ParticleEffectV2::SetTimeout(new_fade, new_delay);
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

DynRpg::ParticleV2::ParticleV2(Game_DynRpg& instance) : DynRpgPlugin("KazeParticles", instance) {
	ParticleEffectV2::CreateTrigLut();
}

bool DynRpg::ParticleV2::Invoke(std::string_view func, dyn_arg_list args, bool& /*do_yield*/,
	Game_Interpreter* /*interpreter*/) {
	bool okay;

	if (!StartsWith(func, "pfx_")) {
		return false;
	}

	if (func == "pfx_destroy_all") {
		pfx_list.clear();
		return true;
	}
	if (func == "pfx_load_effect") {
		return true;
	}
	if (func == "pfx_create_effect") {
		auto [tag, type] = DynRpg::ParseArgs<std::string, std::string>(func, args, &okay);
		if (!okay) {
			return true;
		}
		if (pfx_list.count(tag)) {
			return true;
		}
		std::string kind = type;
		std::transform(kind.begin(), kind.end(), kind.begin(), ::tolower);
		if (!kind.compare(0, 5, "burst")) {
			pfx_list[tag] = std::make_unique<Burst>();
		} else if (!kind.compare(0, 6, "stream")) {
			pfx_list[tag] = std::make_unique<Stream>();
		}
		return true;
	}
	if (func == "pfx_destroy_effect") {
		const std::string tag = DynRpg::ParseArg<std::string>(func, args, 0, &okay);
		if (!okay) {
			return true;
		}
		pfx_list.erase(tag);
		return true;
	}
	if (func == "pfx_does_effect_exist") {
		auto [tag, switch_id] = DynRpg::ParseArgs<std::string, int>(func, args, &okay);
		if (!okay) {
			return true;
		}
		Main_Data::game_switches->Set(switch_id, pfx_list.count(tag) > 0);
		Game_Map::SetNeedRefreshForSwitchChange(switch_id);
		return true;
	}

	const std::string tag = DynRpg::ParseArg<std::string>(func, args, 0, &okay);
	if (!okay) {
		return true;
	}

	ParticleEffectV2* effect = nullptr;

	auto it = pfx_list.find(tag);
	if (it != pfx_list.end()) {
		effect = it->second.get();
	}

	if (!effect) {
		Output::Debug("DynParticle: Particle effect with tag '{}' not found.", tag);
		return true;
	}

	if (func == "pfx_burst") {
		auto [x, y] = DynRpg::ParseArgs<int, int>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		if (effect->GetType() == ParticleEffectV2::Type::Burst) {
			static_cast<Burst*>(effect)->NewBurst(x, y);
		}
		return true;
	}
	if (func == "pfx_start") {
		auto [stream_tag, x, y] = DynRpg::ParseArgs<std::string, int, int>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		if (effect->GetType() == ParticleEffectV2::Type::Stream) {
			static_cast<Stream*>(effect)->Start(x, y, stream_tag);
		}
		return true;
	}
	if (func == "pfx_stop") {
		const std::string stream_tag = DynRpg::ParseArg<std::string>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		if (effect->GetType() == ParticleEffectV2::Type::Stream) {
			static_cast<Stream*>(effect)->Stop(stream_tag);
		}
		return true;
	}
	if (func == "pfx_stopall") {
		if (effect->GetType() == ParticleEffectV2::Type::Stream) {
			static_cast<Stream*>(effect)->StopAll();
		}
		return true;
	}
	if (func == "pfx_set_simul_effects") {
		const int count = DynRpg::ParseArg<int>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		effect->SetSimul(count);
		return true;
	}
	if (func == "pfx_set_amount") {
		const int count = DynRpg::ParseArg<int>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		effect->SetAmount(count);
		return true;
	}
	if (func == "pfx_set_timeout") {
		auto [fade, delay] = DynRpg::ParseArgs<int, int>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		effect->SetTimeout(fade, delay);
		return true;
	}
	if (func == "pfx_set_initial_color") {
		auto [r, g, b] = DynRpg::ParseArgs<int, int, int>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		effect->SetColor0(static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b));
		return true;
	}
	if (func == "pfx_set_final_color") {
		auto [r, g, b] = DynRpg::ParseArgs<int, int, int>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		effect->SetColor1(static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b));
		return true;
	}
	if (func == "pfx_set_growth") {
		auto [ini_size, end_size] = DynRpg::ParseArgs<float, float>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		effect->SetGrowth(ini_size, end_size);
		return true;
	}
	if (func == "pfx_set_position") {
		auto [stream_tag, x, y] = DynRpg::ParseArgs<std::string, int, int>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		if (effect->GetType() == ParticleEffectV2::Type::Stream) {
			static_cast<Stream*>(effect)->SetPosition(stream_tag, x, y);
		}
		return true;
	}
	if (func == "pfx_set_random_position") {
		auto [rand_x, rand_y] = DynRpg::ParseArgs<int, int>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		effect->SetRandPos(rand_x, rand_y);
		return true;
	}
	if (func == "pfx_set_random_radius") {
		const int rand_rad = DynRpg::ParseArg<int>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		effect->SetRandRad(rand_rad);
		return true;
	}
	if (func == "pfx_set_radius") {
		const int rad = DynRpg::ParseArg<int>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		effect->SetRad(rad);
		return true;
	}
	if (func == "pfx_set_texture") {
		const std::string file = DynRpg::ParseArg<std::string>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		effect->SetTexture(file);
		return true;
	}
	if (func == "pfx_set_acceleration_point") {
		auto [x, y, force] = DynRpg::ParseArgs<float, float, float>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		effect->SetAccelerationPoint(x, y, force);
		return true;
	}
	if (func == "pfx_set_gravity_direction") {
		auto [angle, force] = DynRpg::ParseArgs<float, float>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		effect->SetGravityDirection(angle, force);
		return true;
	}
	if (func == "pfx_set_velocity") {
		auto [speed, rand_speed] = DynRpg::ParseArgs<float, float>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		effect->SetSpd(speed);
		effect->SetRandSpd(rand_speed);
		return true;
	}
	if (func == "pfx_set_angle") {
		auto [v1, v2] = DynRpg::ParseArgs<float, float>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		effect->SetAngle(v1, v2);
		return true;
	}
	if (func == "pfx_set_secondary_angle") {
		const float angle = DynRpg::ParseArg<float>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		effect->SetSecondaryAngle(angle);
		return true;
	}
	if (func == "pfx_set_generating_function") {
		const std::string type = DynRpg::ParseArg<std::string>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		effect->SetGeneratingFunction(type);
		return true;
	}
	if (func == "pfx_use_screen_relative") {
		const std::string value = DynRpg::ParseArg<std::string>(func, args, 1, &okay);
		if (!okay) {
			return true;
		}
		effect->UseScreenRelative(!value.empty() && (value[0] == 't' || value[0] == 'T'));
		return true;
	}
	if (func == "pfx_unload_texture") {
		effect->UnloadTexture();
		return true;
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
