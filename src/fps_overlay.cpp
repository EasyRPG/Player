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

#include <sstream>

#include "fps_overlay.h"
#include "game_clock.h"
#include "bitmap.h"
#include "utils.h"
#include "input.h"
#include "font.h"
#include "drawable_mgr.h"

using namespace std::chrono_literals;

static constexpr auto refresh_frequency = 1s;

FpsOverlay::FpsOverlay() :
	Drawable(Priority_Overlay + 100, Drawable::Flags::Global)
{
	DrawableMgr::Register(this);

	UpdateText();
}

void FpsOverlay::UpdateText() {
	auto fps = Utils::RoundTo<int>(Game_Clock::GetFPS());
	text = "FPS: " + std::to_string(fps);
	fps_dirty = true;
}

bool FpsOverlay::Update() {
	int mod = static_cast<int>(Game_Clock::GetGameSpeedFactor());
	if (mod != last_speed_mod) {
		speedup_dirty = true;
		last_speed_mod = mod;
	}

	auto now = Game_Clock::GetFrameTime();
	auto dt = now - last_refresh_time;
	if (dt < refresh_frequency) {
		return false;
	}
	last_refresh_time = now;

	UpdateText();

	return true;
}

void FpsOverlay::Draw(Bitmap& dst) {
	if (draw_fps) {
		if (fps_dirty) {
			std::string text = GetFpsString();
			Rect rect = Text::GetSize(*Font::DefaultBitmapFont(), text);

			if (!fps_bitmap || fps_bitmap->GetWidth() < rect.width + 1) {
				// Height never changes
				fps_bitmap = Bitmap::Create(rect.width + 1, rect.height - 1, true);
			}
			fps_bitmap->Clear();
			fps_bitmap->Fill(Color(0, 0, 0, 128));
			Text::Draw(*fps_bitmap, 1, 0, *Font::DefaultBitmapFont(), Color(255, 255, 255, 255), text);

			fps_rect = Rect(0, 0, rect.width + 1, rect.height - 1);

			fps_dirty = false;
		}

		dst.Blit(1, 2, *fps_bitmap, fps_rect, 255);
	}

	// Always drawn when speedup is on independent of FPS
	if (last_speed_mod > 1) {
		if (speedup_dirty) {
			std::string text = "> x" + std::to_string(last_speed_mod);

			Rect rect = Text::GetSize(*Font::DefaultBitmapFont(), text);

			if (!speedup_bitmap || speedup_bitmap->GetWidth() < rect.width + 1) {
				// Height never changes
				speedup_bitmap = Bitmap::Create(rect.width + 1, rect.height - 1, true);
			}
			speedup_bitmap->Clear();
			speedup_bitmap->Fill(Color(0, 0, 0, 128));
			Text::Draw(*speedup_bitmap, 1, 0, *Font::DefaultBitmapFont(), Color(255, 255, 255, 255), text);

			speedup_rect = Rect(0, 0, rect.width + 1, rect.height - 1);

			speedup_dirty = false;
		}

		int dwidth = dst.GetWidth();
		dst.Blit(dwidth - speedup_rect.width - 1, 2, *speedup_bitmap, speedup_rect, 255);
	}
}

