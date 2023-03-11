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
#include "baseui.h"
#include "bitmap.h"
#include "player.h"

#if USE_SDL==2
#  include "platform/sdl/sdl2_ui.h"
#elif USE_SDL==1
#  include "platform/sdl/sdl_ui.h"
#elif USE_LIBRETRO
#  include "platform/libretro/ui.h"
#elif defined(__3DS__)
#  include "platform/3ds/ui.h"
#elif defined(__vita__)
#  include "platform/psvita/ui.h"
#elif defined(__SWITCH__)
#  include "platform/switch/ui.h"
#endif

std::shared_ptr<BaseUi> DisplayUi;

std::shared_ptr<BaseUi> BaseUi::CreateUi(long width, long height, const Game_Config& cfg) {
#if USE_SDL==2
	return std::make_shared<Sdl2Ui>(width, height, cfg);
#elif USE_SDL==1
	return std::make_shared<SdlUi>(width, height, cfg);
#elif defined(PLAYER_UI)
	return std::make_shared<PLAYER_UI>(width, height, cfg);
#else
#  error cannot create UI
#endif
}

BaseUi::BaseUi(const Game_Config& cfg)
{
	keys.reset();

	vcfg = cfg.video;

	auto fps_limit = vcfg.fps_limit.Get();
	frame_limit = (fps_limit == 0 ? Game_Clock::duration(0) : Game_Clock::TimeStepFromFps(fps_limit));
}

BitmapRef BaseUi::CaptureScreen() {
	BitmapRef capture = Bitmap::Create(main_surface->width(), main_surface->height(), false);
	capture->BlitFast(0, 0, *main_surface, main_surface->GetRect(), Opacity::Opaque());
	return capture;
}

void BaseUi::CleanDisplay() {
	main_surface->Clear();
}

void BaseUi::SetGameResolution(GameResolution resolution) {
	vcfg.game_resolution.Set(resolution);
}

Game_ConfigVideo BaseUi::GetConfig() const {
	Game_ConfigVideo cfg = vcfg;

	cfg.Hide();

	vGetConfig(cfg);

	Rect metrics = GetWindowMetrics();
	cfg.window_x.Set(metrics.x);
	cfg.window_y.Set(metrics.y);
	cfg.window_width.Set(metrics.width);
	cfg.window_height.Set(metrics.height);

	if (cfg.fullscreen.IsOptionVisible()) {
		cfg.fps_render_window.SetLocked(cfg.fullscreen.Get());
		if (cfg.fps_render_window.IsLocked()) {
			cfg.fps_render_window.SetDescription("This options requires to be in windowed mode");
		}
	}

	if (cfg.vsync.IsOptionVisible()
			&& cfg.vsync.Get()) {
		cfg.fps_limit.SetLocked(true);
		cfg.fps_limit.SetDescription("This option requires V-Sync to be disabled");
	}

	if (cfg.fullscreen.IsOptionVisible()
			&& cfg.fullscreen.Get()) {
		cfg.window_zoom.SetLocked(true);
		cfg.window_zoom.SetDescription("This option requires to be in windowed mode");
	}

	if (Player::has_custom_resolution) {
		cfg.game_resolution.SetLocked(true);
		cfg.game_resolution.SetDescription("This game uses a custom resolution");
	}

	return cfg;
}

bool BaseUi::ChangeDisplaySurfaceResolution(int new_width, int new_height) {
	if (new_width == current_display_mode.width && new_height == current_display_mode.height) {
		return true;
	}

	return vChangeDisplaySurfaceResolution(new_width, new_height);
}
