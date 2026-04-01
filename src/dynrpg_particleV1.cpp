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
 * Based on DynRPG Particle Effects V1 by Kazesui. (MIT license)
 */

#include "dynrpg_particleV1.h"

#include <cmath>
#include <vector>
#include <string>
#include <algorithm>
#include <tuple>
#include <list>

#include "async_handler.h"
#include "drawable.h"
#include "drawable_mgr.h"
#include "bitmap.h"
#include "cache.h"
#include "game_screen.h"
#include "game_map.h"
#include "main_data.h"
#include "rand.h"
#include "output.h"

constexpr double PI = 3.14159265358979323846;

namespace {

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
        int amount, size;
        int red, green, blue;
        int rndX, rndY;
        double spd, rndSpd;
        double timeout, delay;
        double angleS, angleE;
        bool mask;
        std::string filename;

        double dr, dg, db, ds, dA;

        std::list<BurstObj> bursts;
        BitmapRef image;
        bool hasTexture;

        ParticleEffectV1() : Drawable(Priority_Weather) {
            dr = dg = db = ds = 0;
            red = green = blue = 255;
            timeout = 30;
            amount = 50;
            delay = 0;
            size = 2;
            dA = 8.5;
            spd = 2;
            rndSpd = 2;
            rndX = rndY = 0;
            angleS = 0;
            angleE = 2 * PI;
            mask = false;
            filename = "Particle";
            hasTexture = false;

            DrawableMgr::Register(this);
        }

        ~ParticleEffectV1() override {}

        void setTimeout(double t) {
            double time_diff = t - delay;
            if (time_diff <= 0) time_diff = 1.0;

            dA = 255.0 / time_diff;
            timeout = t;
        }

        void setAngle(double a1, double a2) {
            angleS = a1 * PI / 180.0;
            angleE = (a2 - a1) * PI / 180.0;
        }

        void setGrowth(int s, int newSize) {
            size = s;
            double t = (timeout > 0) ? timeout : 1.0;
            ds = (newSize - size) / t;
        }

        void colorFade(int r = 0, int g = 0, int b = 0) {
            double t = (timeout > 0) ? timeout : 1.0;
            dr = ((double)r - red) / t;
            dg = ((double)g - green) / t;
            db = ((double)b - blue) / t;
        }

        void loadTexture() {
            std::string name = filename;

            // Emulate RM2k3 legacy relative paths
            size_t pos = name.find("DynPlugins/");
            if (pos != std::string::npos) name.erase(pos, 11);
            pos = name.find("DynPlugins\\");
            if (pos != std::string::npos) name.erase(pos, 11);

            // Prevent Picture/Picture/ overlap
            pos = name.find("Picture/");
            if (pos != std::string::npos) name.erase(pos, 8);
            pos = name.find("Picture\\");
            if (pos != std::string::npos) name.erase(pos, 8);

            // Strip extension for EasyRPG Cache compatibility
            pos = name.find_last_of(".");
            if (pos != std::string::npos) name = name.substr(0, pos);

            if (!name.empty()) {
                FileRequestAsync* req = AsyncHandler::RequestFile("Picture", name);
                req->Start();
                image = Cache::Picture(name, mask);
                hasTexture = true;
            }
        }

        void newBurst(int x, int y) {
            BurstObj burst;
            burst.r = red;
            burst.g = green;
            burst.b = blue;
            burst.alpha = 255.0;

            if (!hasTexture && !filename.empty()) {
                loadTexture();
            }

            for (int i = 0; i < amount; i++) {
                double rand_val1 = (double)Rand::GetRandomNumber(0, 32767) / 32767.0;
                double rand_val2 = (double)Rand::GetRandomNumber(0, 32767) / 32767.0;
                double rand_val3 = (double)Rand::GetRandomNumber(0, 32767) / 32767.0;
                double rand_val4 = (double)Rand::GetRandomNumber(0, 32767) / 32767.0;

                double rnd = angleS + rand_val1 * angleE;
                double newSpd = (spd + rndSpd * rand_val2);

                ParticleObj pa;
                pa.x = x + 2 * rndX * rand_val3 - rndX;
                pa.y = y + 2 * rndY * rand_val4 - rndY;
                pa.dx = newSpd * std::cos(rnd);
                pa.dy = newSpd * std::sin(rnd);
                pa.size = size;

                burst.particles.push_back(pa);
            }
            bursts.push_back(burst);
        }

        void newHeart(int x, int y) {
            BurstObj burst;
            burst.r = red;
            burst.g = green;
            burst.b = blue;
            burst.alpha = 255.0;

            if (!hasTexture && !filename.empty()) {
                loadTexture();
            }

            for (int i = 0; i < amount; i++) {
                double rand_val = (double)Rand::GetRandomNumber(0, 32767) / 32767.0;
                double rnd = rand_val * 2 * PI;

                ParticleObj pa;
                pa.x = x;
                pa.y = y;
                pa.dx = spd * (16 * std::pow(std::sin(rnd), 3));
                pa.dy = -spd * (13 * std::cos(rnd) - 5 * std::cos(2 * rnd) - 2 * std::cos(3 * rnd) - std::cos(4 * rnd));
                pa.size = size;

                burst.particles.push_back(pa);
            }
            bursts.push_back(burst);
        }

        void killParticles() {
            bursts.clear();
        }

        void Update() {
            if (bursts.empty()) return;

            auto burstItr = bursts.begin();
            while (burstItr != bursts.end()) {
                // 1. Color Fade physics
                burstItr->r += dr;
                burstItr->g += dg;
                burstItr->b += db;

                // 2. Alpha fade physics
                if (this->delay <= 0) {
                    burstItr->alpha -= dA;
                    if (burstItr->alpha <= 0) {
                        burstItr = bursts.erase(burstItr);
                        continue;
                    }
                } else {
                    this->delay--;
                }

                // 3. Particle Movement physics
                auto partItr = burstItr->particles.begin();
                while (partItr != burstItr->particles.end()) {
                    partItr->x += partItr->dx;
                    partItr->y += partItr->dy;
                    partItr->size += ds;
                    partItr++;
                }
                burstItr++;
            }
        }

        void Draw(Bitmap& dst) override;
    };

    std::vector<ParticleEffectV1*> v1_effects;
    bool v1_draw = true;

} // anonymous namespace


void ParticleEffectV1::Draw(Bitmap& dst) {
    if (!v1_draw || bursts.empty()) return;

    int cam_x = Game_Map::GetDisplayX() / 16;
    int cam_y = Game_Map::GetDisplayY() / 16;

    for (auto& burst : bursts) {
        int draw_alpha = std::clamp((int)burst.alpha, 0, 255);
        int cur_r = std::clamp((int)burst.r, 0, 255);
        int cur_g = std::clamp((int)burst.g, 0, 255);
        int cur_b = std::clamp((int)burst.b, 0, 255);

        // Pre-tint the texture for this specific burst frame
        BitmapRef colored_image;
        if (hasTexture && image) {
            colored_image = Bitmap::Create(image->GetWidth(), image->GetHeight(), true);
            colored_image->Clear();

            // Map 0-200% RM2k3 tone scale to EasyRPG's 0-255 offset scale where 128 is neutral
            Tone tone(cur_r * 128 / 100, cur_g * 128 / 100, cur_b * 128 / 100, 128);
            colored_image->ToneBlit(0, 0, *image, image->GetRect(), tone, Opacity::Opaque());
        }

        for (auto& particle : burst.particles) {
            int draw_size = std::max(1, (int)particle.size);
            Rect dst_rect((int)particle.x - cam_x, (int)particle.y - cam_y, draw_size, draw_size);

            if (colored_image) {
                dst.StretchBlit(dst_rect, *colored_image, colored_image->GetRect(), Opacity(draw_alpha));
            } else {
                // Fallback if no texture was found
                dst.FillRect(dst_rect, Color(cur_r, cur_g, cur_b, draw_alpha));
            }
        }
    }
}

DynRpg::ParticleV1::ParticleV1(Game_DynRpg& instance) : DynRpgPlugin("ParticleSystemV1", instance) {}

DynRpg::ParticleV1::~ParticleV1() {
    for (auto* pfx : v1_effects) {
        delete pfx;
    }
    v1_effects.clear();
}

bool DynRpg::ParticleV1::Invoke(std::string_view func, dyn_arg_list args, bool& do_yield, Game_Interpreter* interpreter) {

    if (func == "new_effect") {
        v1_effects.push_back(new ParticleEffectV1());
        return true;
    }
    if (func == "stop") {
        v1_draw = false;
        return true;
    }
    if (func == "start") {
        v1_draw = true;
        return true;
    }

    if (func == "effect_burst" || func == "effect_heart" || func == "effect_color" ||
        func == "effect_colorfade" || func == "effect_amount" || func == "effect_random" ||
        func == "effect_angle" || func == "effect_growth" || func == "effect_speed" ||
        func == "effect_timeout" || func == "effect_delay" || func == "effect_file" ||
        func == "effect_kill") {

        auto get_int = [&](size_t idx, int def = 0) {
            if (idx < args.size() && !args[idx].empty()) {
                try { return std::stoi(args[idx]); } catch (...) {}
            }
            return def;
        };

        auto get_double = [&](size_t idx, double def = 0.0) {
            if (idx < args.size() && !args[idx].empty()) {
                try { return std::stod(args[idx]); } catch (...) {}
            }
            return def;
        };

        auto get_string = [&](size_t idx, std::string def = "") {
            if (idx < args.size() && !args[idx].empty()) return args[idx];
            return def;
        };

        int index = get_int(0, -1);

        if (index < 0 || index >= (int)v1_effects.size()) {
            return true;
        }

        if (func == "effect_burst") {
            v1_effects[index]->newBurst(get_int(1, 0), get_int(2, 0));
            return true;
        }

        if (func == "effect_heart") {
            v1_effects[index]->newHeart(get_int(1, 0), get_int(2, 0));
            return true;
        }

        if (func == "effect_color") {
            v1_effects[index]->red = get_int(1, 255);
            v1_effects[index]->green = get_int(2, 255);
            v1_effects[index]->blue = get_int(3, 255);
            return true;
        }

        if (func == "effect_colorfade") {
            v1_effects[index]->colorFade(get_int(1, 0), get_int(2, 0), get_int(3, 0));
            return true;
        }

        if (func == "effect_amount") {
            v1_effects[index]->amount = get_int(1, 50);
            return true;
        }

        if (func == "effect_random") {
            v1_effects[index]->rndX = get_int(1, 0);
            v1_effects[index]->rndY = get_int(2, 0);
            return true;
        }

        if (func == "effect_angle") {
            v1_effects[index]->setAngle(get_double(1, 0.0), get_double(2, 360.0));
            return true;
        }

        if (func == "effect_growth") {
            v1_effects[index]->setGrowth(get_int(1, 2), get_int(2, 2));
            return true;
        }

        if (func == "effect_speed") {
            v1_effects[index]->spd = get_double(1, 2.0);
            v1_effects[index]->rndSpd = get_double(2, 2.0);
            return true;
        }

        if (func == "effect_timeout") {
            v1_effects[index]->setTimeout(get_double(1, 30.0));
            return true;
        }

        if (func == "effect_delay") {
            v1_effects[index]->delay = get_double(1, 0.0);
            return true;
        }

        if (func == "effect_file") {
            std::string filename = get_string(1, "Particle");
            std::string mask_str = get_string(2, "false");
            v1_effects[index]->filename = filename;
            v1_effects[index]->mask = (mask_str.substr(0, 4) == "true" || mask_str.substr(0, 4) == "TRUE");
            v1_effects[index]->loadTexture();
            return true;
        }

        if (func == "effect_kill") {
            v1_effects[index]->killParticles();
            return true;
        }
    }

    return false;
}

void DynRpg::ParticleV1::Update() {
    if (!v1_draw) return;

    for (auto* pfx : v1_effects) {
        pfx->Update();
    }
}

void DynRpg::ParticleV1::OnMapChange() {
    for (auto* pfx : v1_effects) {
        pfx->killParticles();
    }
}
