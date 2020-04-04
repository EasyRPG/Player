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

#if USE_SDL==2
#  include "sdl2_ui.h"
#elif USE_SDL==1
#  include "sdl_ui.h"
#elif USE_LIBRETRO
#  include "platform/libretro/libretro_ui.h"
#elif defined(_3DS)
#  include "platform/3ds/3ds_ui.h"
#elif defined(PSP2)
#  include "platform/psvita/psp2_ui.h"
#elif defined(__SWITCH__)
#  include "platform/switch/switch_ui.h"
#endif

std::shared_ptr<BaseUi> DisplayUi;

std::shared_ptr<BaseUi> BaseUi::CreateUi(long width, long height, const Game_ConfigVideo& cfg) {
#if USE_SDL==2
	auto ui = std::make_shared<Sdl2Ui>(width, height, cfg.window_zoom.Get(), cfg.fullscreen.Get(), cfg.vsync.Get());
#elif USE_SDL==1
	auto ui = std::make_shared<SdlUi>(width, height, cfg.fullscreen.Get());
#elif defined(USE_LIBRETRO)
	auto ui = std::make_shared<LibretroUi>(width, height);
#elif defined(_3DS)
	auto ui = std::make_shared<CtrUi>(width, height);
#elif defined(PSP2)
	auto ui = std::make_shared<Psp2Ui>(width, height);
#elif defined(__SWITCH__)
	auto ui = std::make_shared<NxUi>(width, height);
#else
#  error cannot create UI
#endif
	ui->show_fps = cfg.show_fps.Get();
	ui->fps_render_window = cfg.fps_render_window.Get();

	return ui;
}

BaseUi::BaseUi()
{
	keys.reset();
}

BitmapRef BaseUi::CaptureScreen() {
	return Bitmap::Create(*main_surface, main_surface->GetRect());
}

void BaseUi::CleanDisplay() {
	main_surface->Clear();
}

