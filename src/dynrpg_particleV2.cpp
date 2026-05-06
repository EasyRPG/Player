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
#include "dynrpg_particleV2.h"
#include <array>
#include <cmath>
#include <map>
#include <algorithm>
#include <functional>
#include <vector>
#include "async_handler.h"
#include "drawable.h"
#include "drawable_mgr.h"
#include "baseui.h"
#include "bitmap.h"
#include "cache.h"
#include "game_screen.h"
#include "game_pictures.h"
#include "game_map.h"
#include "game_switches.h"
#include "main_data.h"
#include "graphics.h"
#include "game_battle.h"
#include "scene.h"
#include "rand.h"

constexpr Drawable::Z_t default_priority = Priority_Weather;

class ParticleEffect;

namespace {
	typedef std::map<std::string, ParticleEffect*> ptag_t;
	ptag_t pfx_list;
}

void linear_fade(ParticleEffect* effect, uint32_t color0, uint32_t color1, int fade, int delay);

class ParticleEffect : public Drawable {
public:
	enum class RenderType {
		Screen, // 2D Screen Space (Overlay)
		Map,    // Mode7 World Space (Flat on ground)
		Sprite  // Mode7 World Space (Billboard/Upright)
	};

	ParticleEffect();
	~ParticleEffect() override;

	virtual void Update() = 0;
	void Draw(Bitmap& dst) override = 0;

	bool isScreenSpaceEffect;

	virtual void clear() = 0;
	virtual void setSimul(int newSimul) = 0;
	virtual void setAmount(int newAmount) = 0;
	void setAngle(float v1, float v2);
	void setSecondaryAngle(float v);
	virtual void setTimeout(int fade, int delay);
	void setRad(int new_rad);
	void setSpd(float new_spd);
	void setGrowth(float ini_size, float end_size);
	void setRandRad(int new_rnd_rad);
	void setRandSpd(float new_rnd_spd);
	void setRandPos(int new_rnd_x, int new_rnd_y);
	void setInterval(uint32_t new_interval);
	virtual void setTexture(std::string filename);
	virtual void unloadTexture();
	void useScreenRelative(bool enabled);
	virtual void setGeneratingFunction(std::string type) {}
	void setGravityDirection(float angle, float factor);
	void setAccelerationPoint(float x, float y, float factor);
	void setColor0(uint8_t r, uint8_t g, uint8_t b);
	void setColor1(uint8_t r, uint8_t g, uint8_t b);
	void setRenderType(RenderType type);
	void setZOffset(int offset);
	void setLayer(int layer);

	static void create_trig_lut();

	std::array<Color, 256> palette;

protected:
	bool isScreenRelative;
	RenderType renderType;
	int z_offset;
	Drawable::Z_t base_z;

	int r0, rand_r, rand_x, rand_y;
	float spd, rand_spd, s0, s1, ds, da;
	float gx, gy, ax0, ay0, afc;

	BitmapRef image;
	bool hasTexture;

	float beta, alpha, theta;
	uint8_t fade, delay;
	uint16_t amount;
	uint32_t color0, color1;
	uint32_t interval = 1;
	uint32_t cur_interval = 1;

	void update_color();
	static float sin_lut[32];
};

void linear_fade(ParticleEffect* effect, uint32_t color0, uint32_t color1, int fade, int delay) {
	float r = (color0 >> 16) & 0xff;
	float g = (color0 >> 8) & 0xff;
	float b = (color0 & 0xff);
	if (delay >= fade) delay = fade - 1;

	float dr, dg, db;
	float end_r = (color1 >> 16) & 0xff;
	float end_g = (color1 >> 8) & 0xff;
	float end_b = (color1 & 0xff);

	if (fade - delay > 0) {
		dr = (end_r - r) / (fade - delay);
		dg = (end_g - g) / (fade - delay);
		db = (end_b - b) / (fade - delay);
	} else {
		dr = dg = db = 0.0f;
	}

	int i = 0;
	for (; i < delay; ++i) {
		effect->palette[i] = Color(r, g, b, 255);
	}
	for (; i < fade; ++i) {
		effect->palette[i] = Color(r, g, b, 255);
		r += dr;
		g += dg;
		b += db;
	}
}

float ParticleEffect::sin_lut[32];

ParticleEffect::ParticleEffect() : Drawable(0), isScreenSpaceEffect(false), isScreenRelative(false), renderType(RenderType::Map), z_offset(0), r0(50), rand_r(0), rand_x(0), rand_y(0), spd(0.5), rand_spd(0.5),
s0(1), s1(1), ds(0), gx(0), gy(0), ax0(0), ay0(0), afc(0), beta(6.2832),
alpha(0), theta(0), fade(30), delay(0), amount(50) {
	base_z = default_priority;
	SetZ(base_z);

	da = 255.0f / fade;
	color0 = 0x00ffffff;
	color1 = 0x00ffffff;
	image = Bitmap::Create(1, 1, true);
	hasTexture = false;

	DrawableMgr::Register(this);
}

ParticleEffect::~ParticleEffect() {}

void ParticleEffect::setTexture(std::string filename) {
	FileRequestAsync* req = AsyncHandler::RequestFile("Picture", filename);
	req->Start();
	image = Cache::Picture(filename, true);
	hasTexture = true;
}

void ParticleEffect::unloadTexture() {
	image = Bitmap::Create(1, 1, true);
	hasTexture = false;
	update_color();
}

void ParticleEffect::setGravityDirection(float angle, float factor) {
	angle *= 0.0174532925;
	gx = factor * cosf(angle) / 600.0;
	gy = factor * sinf(angle) / 600.0;
}

void ParticleEffect::setAccelerationPoint(float x, float y, float factor) {
	afc = factor / 600.0;
	ax0 = x;
	ay0 = y;
}

void ParticleEffect::setGrowth(float ini_size, float end_size) {
	s0 = ini_size;
	s1 = end_size;
	if (fade > 0) ds = (s1 - s0) / fade; else ds = 0;
}

void ParticleEffect::useScreenRelative(bool enabled) {
	isScreenRelative = enabled;
}

void ParticleEffect::setAmount(int newAmount) {
	amount = newAmount;
}

void ParticleEffect::setAngle(float v1, float v2) {
	v1 *= 0.0174532925;
	v2 *= 0.0174532925;
	beta = (v2 < 0) ? -v2 : v2;
	alpha = v1 - v2 / 2;
	if (beta == 0.0f) isScreenSpaceEffect = true;
}

void ParticleEffect::setSecondaryAngle(float v) {
	while (v > 360) v -= 360;
	while (v < -360) v += 360;
	theta = v * 0.0174532925;
}

void ParticleEffect::setSpd(float new_spd) {
	spd = new_spd / 60.0;
}

void ParticleEffect::setRandSpd(float new_rnd_spd) {
	rand_spd = new_rnd_spd / 60.0;
}

void ParticleEffect::setRad(int new_rad) {
	r0 = new_rad;
}

void ParticleEffect::setRandPos(int new_rnd_x, int new_rnd_y) {
	rand_x = (new_rnd_x < 0) ? -new_rnd_x : new_rnd_x;
	rand_y = (new_rnd_y < 0) ? -new_rnd_y : new_rnd_y;
}

void ParticleEffect::setRandRad(int new_rnd_rad) {
	rand_r = (new_rnd_rad < 0) ? -new_rnd_rad : new_rnd_rad;
}

void ParticleEffect::setTimeout(int new_fade, int new_delay) {
	if (new_fade > 255) new_fade = 255;
	else if (new_fade <= 0) new_fade = 1;
	if (new_delay >= new_fade) new_delay = new_fade - 1;
	else if (new_delay < 0) new_delay = 0;
	this->fade = new_fade;
	this->delay = new_delay;
	da = 255.0f / new_fade;
	ds = (s1 - s0) / new_fade;
	update_color();
}

void ParticleEffect::setColor0(uint8_t r_val, uint8_t g_val, uint8_t b_val) {
	color0 = (r_val << 16) | (g_val << 8) | b_val;
	update_color();
}

void ParticleEffect::setColor1(uint8_t r_val, uint8_t g_val, uint8_t b_val) {
	color1 = (r_val << 16) | (g_val << 8) | b_val;
	update_color();
}

void ParticleEffect::setInterval(uint32_t new_interval) {
	if (new_interval < 1) return;
	cur_interval = new_interval;
	interval = new_interval;
}

void ParticleEffect::setRenderType(RenderType type) {
	renderType = type;
}

void ParticleEffect::setZOffset(int offset) {
	z_offset = offset;
}

void ParticleEffect::setLayer(int layer) {
	if (Game_Battle::IsBattleRunning()) {
		base_z = Drawable::GetPriorityForBattleLayer(layer);
	} else {
		base_z = Drawable::GetPriorityForMapLayer(layer);
	}
}

void ParticleEffect::update_color() {
	linear_fade(this, color0, color1, fade, delay);
}

void ParticleEffect::create_trig_lut() {
	double dr = 3.141592653589793 / 16.0;
	for (int i = 0; i < 32; i++)
		sin_lut[i] = sin(dr * i);
}

class Burst : public ParticleEffect {
public:
	Burst();
	~Burst() override = default;

	void Update() override;
	void Draw(Bitmap& dst) override;
	void clear() override;
	void newBurst(int x, int y);

	void setSimul(int newSimul) override;
	void setAmount(int newAmount) override;
	void setTimeout(int fade, int delay) override;
	void setGeneratingFunction(std::string type) override;

private:
	uint16_t simulCnt;
	uint16_t simulMax;

	std::vector<float> x, y, s, dx, dy;
	std::vector<uint8_t> itr;
	std::vector<PointF> origins;

	void reallocate(bool preserve);

	void (Burst::*init)(int, int, int, int);

	void init_basic(int x0, int y0, int a, int b);
	void init_radial(int x0, int y0, int a, int b);
};

Burst::Burst() : ParticleEffect(), simulCnt(0), simulMax(1) {
	reallocate(false);
	init = &Burst::init_basic;
	update_color();
}

void Burst::reallocate(bool preserve) {
	size_t pool_size = static_cast<size_t>(amount) * simulMax;
	if (!preserve) {
		x.assign(pool_size, 0.0f);
		y.assign(pool_size, 0.0f);
		s.assign(pool_size, 0.0f);
		dx.assign(pool_size, 0.0f);
		dy.assign(pool_size, 0.0f);
		itr.assign(simulMax, 0);
		origins.assign(simulMax, {0.0f, 0.0f});
		simulCnt = 0;
	} else {
		x.resize(pool_size, 0.0f);
		y.resize(pool_size, 0.0f);
		s.resize(pool_size, 0.0f);
		dx.resize(pool_size, 0.0f);
		dy.resize(pool_size, 0.0f);
		itr.resize(simulMax, 0);
		origins.resize(simulMax, {0.0f, 0.0f});
	}
}

void Burst::setAmount(int newAmount) {
	amount = newAmount;
	reallocate(false);
}

void Burst::setSimul(int newSimul) {
	simulMax = newSimul;
	reallocate(false);
}

void Burst::setTimeout(int new_fade, int new_delay) {
	ParticleEffect::setTimeout(new_fade, new_delay);
	reallocate(false);
}

void Burst::setGeneratingFunction(std::string type) {
	std::transform(type.begin(), type.end(), type.begin(), ::tolower);
	if (!type.substr(0, 8).compare("standard")) {
		init = &Burst::init_basic;
		isScreenSpaceEffect = false;
		renderType = RenderType::Map;
		return;
	}
	if (!type.substr(0, 6).compare("radial")) {
		init = &Burst::init_radial;
		isScreenSpaceEffect = true;
		renderType = RenderType::Screen;
		return;
	}
}

void Burst::clear() {
	simulCnt = 0;
}

void Burst::newBurst(int x0, int y0) {
	if (simulCnt >= simulMax) {
		simulMax *= 2;
		reallocate(true);
	}

	itr[simulCnt] = 0;
	origins[simulCnt] = { (float)x0, (float)y0 };

	(this->*init)(x0, y0, simulCnt * amount, (simulCnt + 1) * amount);
	simulCnt++;
}

void Burst::init_basic(int x0, int y0, int a, int b) {
	for (int i = a; i < b; i++) {
		x[i] = x0 + 2 * rand_x * (float)rand() / RAND_MAX - rand_x;
		y[i] = y0 + 2 * rand_y * (float)rand() / RAND_MAX - rand_y;
		s[i] = s0;

		float tmp_angle = (float)rand() / RAND_MAX * beta + alpha;
		float tmp_spd = spd + rand_spd * (float)rand() / RAND_MAX;
		int v = tmp_angle / 0.1963495408;
		tmp_angle = (tmp_angle - v * 0.1963495408) / 0.1963495408;
		dx[i] = tmp_spd * (sin_lut[(v + 9) & 31] * tmp_angle + sin_lut[(v + 8) & 31] * (1 - tmp_angle));
		dy[i] = tmp_spd * (sin_lut[(v + 1) & 31] * tmp_angle + sin_lut[(v + 0) & 31] * (1 - tmp_angle));
	}
}

void Burst::init_radial(int x0, int y0, int a, int b) {
	for (int i = a; i < b; i++) {
		float tmp_rnd = rand_r * (float)rand() / RAND_MAX;
		float tmp_angle = (float)rand() / RAND_MAX * beta + alpha;
		float tmp_spd = spd + rand_spd * (float)rand() / RAND_MAX;
		int   v = tmp_angle / 0.1963495408;
		float p = (tmp_angle - v * 0.1963495408) / 0.1963495408;

		x[i] = x0 + (r0 + tmp_rnd) * (sin_lut[(v + 9) & 31] * p + sin_lut[(v + 8) & 31] * (1 - p));
		y[i] = y0 + (r0 + tmp_rnd) * (sin_lut[(v + 1) & 31] * p + sin_lut[(v + 0) & 31] * (1 - p));
		s[i] = s0;

		v = (tmp_angle + theta) / 0.1963495408;
		p = (tmp_angle + theta - v * 0.1963495408) / 0.1963495408;
		dx[i] = -tmp_spd * (sin_lut[(v + 9) & 31] * p + sin_lut[(v + 8) & 31] * (1 - p));
		dy[i] = -tmp_spd * (sin_lut[(v + 1) & 31] * p + sin_lut[(v + 0) & 31] * (1 - p));
	}
}

void Burst::Update() {
	if (simulCnt <= 0) return;

	for (int i = 0; i < simulCnt; ++i) {
		if (itr[i] >= fade) {
			simulCnt--;
			if (i < simulCnt) {
				size_t dead_offset = i * amount;
				size_t last_offset = simulCnt * amount;
				std::copy_n(&x[last_offset], amount, &x[dead_offset]);
				std::copy_n(&y[last_offset], amount, &y[dead_offset]);
				std::copy_n(&s[last_offset], amount, &s[dead_offset]);
				std::copy_n(&dx[last_offset], amount, &dx[dead_offset]);
				std::copy_n(&dy[last_offset], amount, &dy[dead_offset]);
				itr[i] = itr[simulCnt];
				origins[i] = origins[simulCnt];
			}
			--i;
			continue;
		}

		itr[i]++;

		float tx, ty, tsqr;
		for (int j = i * amount; j < (i + 1) * amount; j++) {
			x[j] += dx[j];
			y[j] += dy[j];
			tx = ax0 - x[j];
			ty = ay0 - y[j];
			tsqr = sqrtf(tx*tx + ty*ty + 0.001);
			dx[j] += gx + afc * tx / tsqr;
			dy[j] += gy + afc * ty / tsqr;
			s[j] += ds;
		}
	}
}

void Burst::Draw(Bitmap& dst) {
	if (simulCnt <= 0) return;

	int cam_x = (isScreenRelative) ? 0 : Game_Map::GetDisplayX() / 16;
	int cam_y = (isScreenRelative) ? 0 : Game_Map::GetDisplayY() / 16;

	SetZ(base_z + z_offset);

	for (int i = 0; i < simulCnt; i++) {
		int age = itr[i];
		if (age >= fade) continue;

		int alpha = static_cast<int>(255 - da * age);
		Color color = palette[age];

		for (int j = i * amount; j < (i + 1) * amount; j++) {
			Rect dst_rect(x[j] - cam_x - s[j] / 2, y[j] - cam_y - s[j] / 2 - z_offset, s[j], s[j]);

			if (hasTexture) {
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
	void clear() override;
	void stopAll();
	void stop(std::string tag);
	void start(int x, int y, std::string tag);

	void setSimul(int newSimul) override;
	void setAmount(int newAmount) override;
	void setTimeout(int fade, int delay) override;
	void setGeneratingFunction(std::string type) override;
	void setPosition(std::string tag, int x, int y);

private:
	uint16_t simulBeg;
	uint16_t simulRun;
	uint16_t simulCnt;
	uint16_t simulMax;

	std::vector<float> x, y, s, dx, dy;
	std::vector<uint8_t> itr, end_cnt;
	std::vector<int16_t> str_x, str_y;
	std::vector<uint16_t> pfx_ref;

	std::map<std::string, int> pfx_tag;

	void reallocate(bool preserve);
	void stream_to_end(uint16_t idx);
	void start_to_stream(uint16_t idx);

	void (Stream::*init)(int, int, int);

	void init_basic(int a, int b, int idx);
	void init_radial(int a, int b, int idx);
};

Stream::Stream() : ParticleEffect(), simulBeg(0), simulRun(0), simulCnt(0), simulMax(1) {
	amount = 10;
	reallocate(false);
	init = &Stream::init_basic;
	update_color();
}

void Stream::reallocate(bool preserve) {
	size_t pool_size = static_cast<size_t>(amount) * fade * simulMax;
	if (!preserve) {
		x.assign(pool_size, 0.0f);
		y.assign(pool_size, 0.0f);
		s.assign(pool_size, 0.0f);
		dx.assign(pool_size, 0.0f);
		dy.assign(pool_size, 0.0f);
		itr.assign(simulMax, 0);
		str_x.assign(simulMax, 0);
		str_y.assign(simulMax, 0);
		pfx_ref.resize(simulMax);
		for (uint16_t i = 0; i < simulMax; i++) {
			pfx_ref[i] = i;
		}
		end_cnt.assign(simulMax, 0);
		simulBeg = 0;
		simulRun = 0;
		simulCnt = 0;
		pfx_tag.clear();
	} else {
		x.resize(pool_size, 0.0f);
		y.resize(pool_size, 0.0f);
		s.resize(pool_size, 0.0f);
		dx.resize(pool_size, 0.0f);
		dy.resize(pool_size, 0.0f);
		itr.resize(simulMax, 0);
		str_x.resize(simulMax, 0);
		str_y.resize(simulMax, 0);
		size_t old_simulMax = pfx_ref.size();
		pfx_ref.resize(simulMax);
		for (uint16_t i = old_simulMax; i < simulMax; i++) {
			pfx_ref[i] = i;
		}
		end_cnt.resize(simulMax, 0);
	}
}

void Stream::start(int x0, int y0, std::string tag) {
	if (pfx_tag.count(tag)) return;
	if (simulCnt >= simulMax) {
		simulMax *= 2;
		reallocate(true);
	}

	uint16_t idx = pfx_ref[simulCnt];

	std::swap(pfx_ref[simulCnt], pfx_ref[simulRun]);
	std::swap(pfx_ref[simulRun], pfx_ref[simulBeg]);

	pfx_tag[tag] = idx;

	end_cnt[idx] = fade - 1;
	str_x[idx] = x0;
	str_y[idx] = y0;
	itr[idx] = 0;
	simulBeg++;
	simulRun++;
	simulCnt++;
}

void Stream::stop(std::string tag) {
	auto pfx_itr = pfx_tag.find(tag);
	if (pfx_itr == pfx_tag.end()) return;
	uint8_t probe = pfx_itr->second;

	auto it = std::find(pfx_ref.begin(), pfx_ref.begin() + simulCnt, probe);
	if (it == pfx_ref.begin() + simulCnt) return;

	simulRun--;
	std::swap(*it, pfx_ref[simulRun]);
	pfx_tag.erase(pfx_itr);
}

void Stream::stopAll() {
	simulBeg = 0;
	simulRun = 0;
	pfx_tag.clear();
}

void Stream::clear() {
	simulBeg = 0;
	simulRun = 0;
	simulCnt = 0;
	pfx_tag.clear();
}

void Stream::setGeneratingFunction(std::string type) {
	std::transform(type.begin(), type.end(), type.begin(), ::tolower);
	if (!type.substr(0, 8).compare("standard")) {
		init = &Stream::init_basic;
		isScreenSpaceEffect = false;
		renderType = RenderType::Map;
		return;
	}
	if (!type.substr(0, 6).compare("radial")) {
		init = &Stream::init_radial;
		isScreenSpaceEffect = true;
		renderType = RenderType::Screen;
		return;
	}
}

void Stream::init_basic(int a, int b, int idx) {
	float x0 = str_x[idx];
	float y0 = str_y[idx];
	for (int i = a; i < b; i++) {
		x[i] = x0 + 2 * rand_x * (float)rand() / RAND_MAX - rand_x;
		y[i] = y0 + 2 * rand_y * (float)rand() / RAND_MAX - rand_y;
		s[i] = s0;

		float tmp_angle = (float)rand() / RAND_MAX * beta + alpha;
		float tmp_spd = spd + rand_spd * (float)rand() / RAND_MAX;
		int v = tmp_angle / 0.1963495408;
		tmp_angle = (tmp_angle - v * 0.1963495408) / 0.1963495408;
		dx[i] = tmp_spd * (sin_lut[(v + 9) & 31] * tmp_angle + sin_lut[(v + 8) & 31] * (1 - tmp_angle));
		dy[i] = tmp_spd * (sin_lut[(v + 1) & 31] * tmp_angle + sin_lut[(v + 0) & 31] * (1 - tmp_angle));
	}
}

void Stream::init_radial(int a, int b, int idx) {
	float x0 = str_x[idx];
	float y0 = str_y[idx];
	for (int i = a; i < b; i++) {
		float tmp_rnd = rand_r * (float)rand() / RAND_MAX;
		float tmp_angle = (float)rand() / RAND_MAX * beta + alpha;
		float tmp_spd = spd + rand_spd * (float)rand() / RAND_MAX;
		int   v = tmp_angle / 0.1963495408;
		float p = (tmp_angle - v * 0.1963495408) / 0.1963495408;

		x[i] = x0 + (r0 + tmp_rnd) * (sin_lut[(v + 9) & 31] * p + sin_lut[(v + 8) & 31] * (1 - p));
		y[i] = y0 + (r0 + tmp_rnd) * (sin_lut[(v + 1) & 31] * p + sin_lut[(v + 0) & 31] * (1 - p));
		s[i] = s0;

		v = (tmp_angle + theta) / 0.1963495408;
		p = (tmp_angle + theta - v * 0.1963495408) / 0.1963495408;
		dx[i] = -tmp_spd * (sin_lut[(v + 9) & 31] * p + sin_lut[(v + 8) & 31] * (1 - p));
		dy[i] = -tmp_spd * (sin_lut[(v + 1) & 31] * p + sin_lut[(v + 0) & 31] * (1 - p));
	}
}

void Stream::setPosition(std::string tag, int x_pos, int y_pos) {
	auto pfx_itr = pfx_tag.find(tag);
	if (pfx_itr == pfx_tag.end()) return;
	uint8_t probe = pfx_itr->second;

	auto it = std::find(pfx_ref.begin(), pfx_ref.begin() + simulCnt, probe);
	if (it == pfx_ref.begin() + simulCnt) return;
	auto i = std::distance(pfx_ref.begin(), it);
	str_x[pfx_ref[i]] = x_pos;
	str_y[pfx_ref[i]] = y_pos;
}

void Stream::setAmount(int newAmount) {
	amount = newAmount;
	reallocate(false);
}

void Stream::setSimul(int newSimul) {
	simulMax = newSimul;
	reallocate(false);
}

void Stream::setTimeout(int new_fade, int new_delay) {
	ParticleEffect::setTimeout(new_fade, new_delay);
	reallocate(false);
}

void Stream::Update() {
	if (simulCnt <= 0) return;

	int block_size = amount * fade;

	// --- Physics Update Section ---
	for (int i = 0; i < simulCnt; ++i) {
		uint16_t p_ref = pfx_ref[i];
		int base_idx = p_ref * block_size;
		for (int j = 0; j < block_size; ++j) {
			int p_idx = base_idx + j;
			x[p_idx] += dx[p_idx];
			y[p_idx] += dy[p_idx];
			float tx = ax0 - x[p_idx];
			float ty = ay0 - y[p_idx];
			float tsqr = sqrtf(tx*tx + ty*ty + 0.001);
			dx[p_idx] += gx + afc * tx / tsqr;
			dy[p_idx] += gy + afc * ty / tsqr;
			s[p_idx] += ds;
		}
	}

	// --- Spawning Section ---
	--cur_interval;
	if (cur_interval == 0) {
		for (int i = simulBeg; i < simulRun; i++) {
			uint16_t idx = pfx_ref[i];
			uint8_t z = fade - itr[idx] - 1;
			(this->*init)(z * amount + idx * block_size, (z + 1) * amount + idx * block_size, idx);
		}
		cur_interval = interval;
	}

	// --- State Update Section ---
	// Starting
	for (int i = 0; i < simulBeg; i++) {
		uint16_t idx = pfx_ref[i];
		if (itr[idx] < fade) {
			uint8_t z = fade - itr[idx] - 1;
			(this->*init)(z * amount + idx * block_size, (z + 1) * amount + idx * block_size, idx);
			itr[idx]++;
		}
		else {
			start_to_stream(i--);
		}
	}

	// Streaming
	for (int i = simulBeg; i < simulRun; i++) {
		uint16_t idx = pfx_ref[i];
		itr[idx] = (itr[idx] + 1) % fade;
	}

	// Stopping
	for (int i = simulRun; i < simulCnt; i++) {
		uint16_t idx = pfx_ref[i];
		if (end_cnt[idx] == 0) {
			stream_to_end(i--);
		} else {
			end_cnt[idx]--;
		}
	}
}

void Stream::Draw(Bitmap& dst) {
	if (simulCnt <= 0) return;

	int cam_x = (isScreenRelative) ? 0 : Game_Map::GetDisplayX() / 16;
	int cam_y = (isScreenRelative) ? 0 : Game_Map::GetDisplayY() / 16;
	int block_size = amount * fade;

	SetZ(base_z + z_offset);

	auto draw_block = [&](uint16_t idx, uint8_t n, uint8_t z, uint8_t c0) {
		for (uint8_t i = 0; i < n; i++) {
			int age = i + c0;
			if (age >= fade) continue;

			int alpha = static_cast<int>(255 - da * age);
			Color color = palette[age];
			int block_start_idx = idx * block_size + z * amount;

			for (int j = 0; j < amount; j++) {
				int p_idx = block_start_idx + j;
				float size = s[p_idx];
				Rect dst_rect(x[p_idx] - cam_x - size / 2.0f, y[p_idx] - cam_y - size / 2.0f - z_offset, size, size);

				if (hasTexture) {
					dst.StretchBlit(dst_rect, *image, image->GetRect(), Opacity(alpha));
				} else {
					dst.FillRect(dst_rect, Color(color.red, color.green, color.blue, alpha));
				}
			}
			z = (z + 1) % fade;
		}
	};

	// Starting
	for (int i = 0; i < simulBeg; i++) {
		uint16_t idx = pfx_ref[i];
		uint8_t z = fade - itr[idx];
		draw_block(idx, itr[idx], z, 0);
	}
	// Streaming
	for (int i = simulBeg; i < simulRun; i++) {
		uint16_t idx = pfx_ref[i];
		uint8_t z_old = (itr[idx] == 0) ? 0 : fade - itr[idx];
		draw_block(idx, fade, z_old, 0);
	}
	// Stopping
	for (int i = simulRun; i < simulCnt; i++) {
		uint16_t idx = pfx_ref[i];
		uint8_t count = end_cnt[idx] + 1;
		uint8_t z_old = (fade - itr[idx]) % fade;
		draw_block(idx, count, z_old, fade - count);
	}
}

void Stream::start_to_stream(uint16_t idx) {
	itr[pfx_ref[idx]] = 0;
	--simulBeg;
	std::swap(pfx_ref[simulBeg], pfx_ref[idx]);
}

void Stream::stream_to_end(uint16_t idx) {
	--simulCnt;
	std::swap(pfx_ref[simulCnt], pfx_ref[idx]);
}

// ============================================================================
// DynRPG Plugin Interface Implementation
// ============================================================================
namespace {
	std::map<std::string_view, std::function<bool(dyn_arg_list)>> function_list;

	ParticleEffect* GetPfx(const std::string& tag) {
		auto it = pfx_list.find(tag);
		if (it != pfx_list.end()) {
			return it->second;
		}
		Output::Debug("DynParticle: Particle effect with tag '{}' not found.", tag.c_str());
		return nullptr;
	}
}

static bool create_effect(dyn_arg_list args) {
	auto func = "pfx_create_effect";
	bool okay;
	std::string tag, type;
	std::tie(tag, type) = DynRpg::ParseArgs<std::string, std::string>(func, args, &okay);
	if (!okay) return true;
	if (pfx_list.count(tag)) return true;
	std::transform(type.begin(), type.end(), type.begin(), ::tolower);
	if (!type.substr(0, 5).compare("burst")) pfx_list[tag] = new Burst();
	else if (!type.substr(0, 6).compare("stream")) pfx_list[tag] = new Stream();
	return true;
}

static bool destroy_effect(dyn_arg_list args) {
	auto func = "pfx_destroy_effect";
	bool okay;
	auto [tag] = DynRpg::ParseArgs<std::string>(func, args, &okay);
	if (!okay) return true;
	auto itr = pfx_list.find(tag);
	if (itr != pfx_list.end()) {
		delete itr->second;
		pfx_list.erase(itr);
	}
	return true;
}

static bool destroy_all(dyn_arg_list) {
	for (auto const& [tag, pfx] : pfx_list) {
		delete pfx;
	}
	pfx_list.clear();
	return true;
}

static bool does_effect_exist(dyn_arg_list args) {
	auto func = "pfx_does_effect_exist";
	bool okay;
	std::string tag;
	int idx;
	std::tie(tag, idx) = DynRpg::ParseArgs<std::string, int>(func, args, &okay);
	if (!okay) return true;
	Main_Data::game_switches->Set(idx, pfx_list.count(tag));
	Game_Map::SetNeedRefresh(true);
	return true;
}

static bool burst(dyn_arg_list args) {
	auto func = "pfx_burst";
	bool okay;
	std::string tag;
	int x, y;
	std::tie(tag, x, y) = DynRpg::ParseArgs<std::string, int, int>(func, args, &okay);
	if (!okay) return true;
	if (auto pfx = GetPfx(tag)) {
		if (auto b = dynamic_cast<Burst*>(pfx)) {
			b->newBurst(x, y);
		}
	}
	return true;
}

static bool start(dyn_arg_list args) {
	auto func = "pfx_start";
	bool okay;
	std::string tag1, tag2;
	int x, y;
	std::tie(tag1, tag2, x, y) = DynRpg::ParseArgs<std::string, std::string, int, int>(func, args, &okay);
	if (!okay) return true;
	if (auto pfx = GetPfx(tag1)) {
		if (auto s = dynamic_cast<Stream*>(pfx)) {
			s->start(x, y, tag2);
		}
	}
	return true;
}

static bool stop(dyn_arg_list args) {
	auto func = "pfx_stop";
	bool okay;
	std::string tag1, tag2;
	std::tie(tag1, tag2) = DynRpg::ParseArgs<std::string, std::string>(func, args, &okay);
	if (!okay) return true;
	if (auto pfx = GetPfx(tag1)) {
		if (auto s = dynamic_cast<Stream*>(pfx)) {
			s->stop(tag2);
		}
	}
	return true;
}

static bool stopall(dyn_arg_list args) {
	auto func = "pfx_stopall";
	bool okay;
	auto [tag] = DynRpg::ParseArgs<std::string>(func, args, &okay);
	if (!okay) return true;
	if (auto pfx = GetPfx(tag)) {
		if (auto s = dynamic_cast<Stream*>(pfx)) {
			s->stopAll();
		}
	}
	return true;
}

static bool set_position(dyn_arg_list args) {
	auto func = "pfx_set_position";
	bool okay;
	std::string tag1, tag2;
	int x, y;
	std::tie(tag1, tag2, x, y) = DynRpg::ParseArgs<std::string, std::string, int, int>(func, args, &okay);
	if (!okay) return true;
	if (auto pfx = GetPfx(tag1)) {
		if (auto s = dynamic_cast<Stream*>(pfx)) {
			s->setPosition(tag2, x, y);
		}
	}
	return true;
}

static bool set_interval(dyn_arg_list args) {
	auto func = "pfx_set_interval";
	bool okay;
	std::string tag;
	int interval;
	std::tie(tag, interval) = DynRpg::ParseArgs<std::string, int>(func, args, &okay);
	if (!okay) return true;
	if (auto pfx = GetPfx(tag)) {
		pfx->setInterval(interval);
	}
	return true;
}

static bool load_effect(dyn_arg_list) {
	return true;
}

static bool SetZ(dyn_arg_list args) {
	auto func = "pfx_set_z_offset";
	bool okay;
	std::string tag;
	int z;
	std::tie(tag, z) = DynRpg::ParseArgs<std::string, int>(func, args, &okay);
	if (!okay) return true;
	if (auto pfx = GetPfx(tag)) {
		pfx->setZOffset(z);
	}
	return true;
}

static bool SetLayer(dyn_arg_list args) {
	auto func = "pfx_set_layer";
	bool okay;
	std::string tag;
	int layer;
	std::tie(tag, layer) = DynRpg::ParseArgs<std::string, int>(func, args, &okay);
	if (!okay) return true;
	if (auto pfx = GetPfx(tag)) {
		pfx->setLayer(layer);
	}
	return true;
}

DynRpg::Particle::Particle(Game_DynRpg& instance) : DynRpgPlugin("KazeParticles", instance) {
	ParticleEffect::create_trig_lut();

	if (function_list.empty()) {
		function_list["pfx_destroy_all"] = &destroy_all;
		function_list["pfx_create_effect"] = &create_effect;
		function_list["pfx_destroy_effect"] = &destroy_effect;
		function_list["pfx_does_effect_exist"] = &does_effect_exist;
		function_list["pfx_burst"] = &burst;
		function_list["pfx_start"] = &start;
		function_list["pfx_stop"] = &stop;
		function_list["pfx_stopall"] = &stopall;

		auto add_setter_1_int =[](const char* name, void (ParticleEffect::*setter)(int)) {
			function_list[name] = [name, setter](dyn_arg_list args) {
				bool okay; std::string tag; int val;
				std::tie(tag, val) = DynRpg::ParseArgs<std::string, int>(name, args, &okay);
				if (okay) if (auto pfx = GetPfx(tag)) (pfx->*setter)(val);
				return true;
			};
		};
		auto add_setter_2_int =[](const char* name, void (ParticleEffect::*setter)(int, int)) {
			function_list[name] =[name, setter](dyn_arg_list args) {
				bool okay; std::string tag; int val1, val2;
				std::tie(tag, val1, val2) = DynRpg::ParseArgs<std::string, int, int>(name, args, &okay);
				if (okay) if (auto pfx = GetPfx(tag)) (pfx->*setter)(val1, val2);
				return true;
			};
		};
		auto add_setter_2_float =[](const char* name, void (ParticleEffect::*setter)(float, float)) {
			function_list[name] = [name, setter](dyn_arg_list args) {
				bool okay; std::string tag; float val1, val2;
				std::tie(tag, val1, val2) = DynRpg::ParseArgs<std::string, float, float>(name, args, &okay);
				if (okay) if (auto pfx = GetPfx(tag)) (pfx->*setter)(val1, val2);
				return true;
			};
		};
		auto add_setter_3_int =[](const char* name, void (ParticleEffect::*setter)(uint8_t, uint8_t, uint8_t)) {
			function_list[name] = [name, setter](dyn_arg_list args) {
				bool okay; std::string tag; int r, g, b;
				std::tie(tag, r, g, b) = DynRpg::ParseArgs<std::string, int, int, int>(name, args, &okay);
				if (okay) if (auto pfx = GetPfx(tag)) (pfx->*setter)(r, g, b);
				return true;
			};
		};

		add_setter_1_int("pfx_set_simul_effects", &ParticleEffect::setSimul);
		add_setter_1_int("pfx_set_amount", &ParticleEffect::setAmount);
		add_setter_2_int("pfx_set_timeout", &ParticleEffect::setTimeout);
		add_setter_1_int("pfx_set_random_radius", &ParticleEffect::setRandRad);
		add_setter_1_int("pfx_set_radius", &ParticleEffect::setRad);
		add_setter_2_int("pfx_set_random_position", &ParticleEffect::setRandPos);
		add_setter_3_int("pfx_set_initial_color", &ParticleEffect::setColor0);
		add_setter_3_int("pfx_set_final_color", &ParticleEffect::setColor1);
		add_setter_2_float("pfx_set_growth", &ParticleEffect::setGrowth);
		add_setter_2_float("pfx_set_angle", &ParticleEffect::setAngle);

		function_list["pfx_set_velocity"] =[](dyn_arg_list args) {
			bool okay; std::string tag; float speed, rand_speed;
			std::tie(tag, speed, rand_speed) = DynRpg::ParseArgs<std::string, float, float>("pfx_set_velocity", args, &okay);
			if (okay) if (auto pfx = GetPfx(tag)) { pfx->setSpd(speed); pfx->setRandSpd(rand_speed); }
			return true;
		};
		function_list["pfx_set_texture"] =[](dyn_arg_list args) {
			bool okay; std::string tag, texture;
			std::tie(tag, texture) = DynRpg::ParseArgs<std::string, std::string>("pfx_set_texture", args, &okay);
			if (okay) if (auto pfx = GetPfx(tag)) pfx->setTexture(texture);
			return true;
		};
		function_list["pfx_set_acceleration_point"] =[](dyn_arg_list args) {
			bool okay; std::string tag; float x, y, force;
			std::tie(tag, x, y, force) = DynRpg::ParseArgs<std::string, float, float, float>("pfx_set_acceleration_point", args, &okay);
			if (okay) if (auto pfx = GetPfx(tag)) pfx->setAccelerationPoint(x, y, force);
			return true;
		};
		function_list["pfx_set_gravity_direction"] =[](dyn_arg_list args) {
			bool okay; std::string tag; float angle, force;
			std::tie(tag, angle, force) = DynRpg::ParseArgs<std::string, float, float>("pfx_set_gravity_direction", args, &okay);
			if (okay) if (auto pfx = GetPfx(tag)) pfx->setGravityDirection(angle, force);
			return true;
		};
		function_list["pfx_set_secondary_angle"] =[](dyn_arg_list args) {
			bool okay; std::string tag; float angle;
			std::tie(tag, angle) = DynRpg::ParseArgs<std::string, float>("pfx_set_secondary_angle", args, &okay);
			if (okay) if (auto pfx = GetPfx(tag)) pfx->setSecondaryAngle(angle);
			return true;
		};
		function_list["pfx_set_generating_function"] =[](dyn_arg_list args) {
			bool okay; std::string tag, type;
			std::tie(tag, type) = DynRpg::ParseArgs<std::string, std::string>("pfx_set_generating_function", args, &okay);
			if (okay) if (auto pfx = GetPfx(tag)) pfx->setGeneratingFunction(type);
			return true;
		};
		function_list["pfx_unload_texture"] =[](dyn_arg_list args) {
			bool okay; std::string tag;
			std::tie(tag) = DynRpg::ParseArgs<std::string>("pfx_unload_texture", args, &okay);
			if (okay) if (auto pfx = GetPfx(tag)) pfx->unloadTexture();
			return true;
		};
		function_list["pfx_use_screen_relative"] =[](dyn_arg_list args) {
			bool okay; std::string tag, val;
			std::tie(tag, val) = DynRpg::ParseArgs<std::string, std::string>("pfx_use_screen_relative", args, &okay);
			if (okay) if (auto pfx = GetPfx(tag)) pfx->useScreenRelative(val[0] == 't' || val[0] == 'T');
			return true;
		};
		function_list["pfx_set_position"] = &set_position;
		function_list["pfx_set_interval"] = &set_interval;
		function_list["pfx_load_effect"] = &load_effect;
		function_list["pfx_set_z_offset"] = &SetZ;
		function_list["pfx_set_layer"] = &SetLayer;
		function_list["pfx_set_render_plane"] =[](dyn_arg_list args) {
			bool okay; std::string tag, plane;
			std::tie(tag, plane) = DynRpg::ParseArgs<std::string, std::string>("pfx_set_render_plane", args, &okay);
			if (okay) {
				if (auto pfx = GetPfx(tag)) {
					std::transform(plane.begin(), plane.end(), plane.begin(), ::tolower);
					if (plane == "screen") {
						pfx->setRenderType(ParticleEffect::RenderType::Screen);
						pfx->isScreenSpaceEffect = true;
					} else if (plane == "sprite" || plane == "event") {
						pfx->setRenderType(ParticleEffect::RenderType::Sprite);
					} else {
						pfx->setRenderType(ParticleEffect::RenderType::Map);
					}
				}
			}
			return true;
		};
	}
}

DynRpg::Particle::~Particle() {
	destroy_all({});
}

bool DynRpg::Particle::Invoke(std::string_view func, dyn_arg_list args, bool&, Game_Interpreter*) {
	auto it = function_list.find(func);
	if (it != function_list.end()) {
		return it->second(args);
	}
	return false;
}

void DynRpg::Particle::Update() {
	if (!pfx_list.empty()) {
		if ((Scene::instance && Scene::instance->type == Scene::Map) || Game_Battle::IsBattleRunning()) {
			for (auto const& [tag, pfx] : pfx_list) {
				pfx->Update();
			}
		}
	}
}

void DynRpg::Particle::OnMapChange() {
	for (auto const& [tag, pfx] : pfx_list) {
		pfx->clear();
	}
}
