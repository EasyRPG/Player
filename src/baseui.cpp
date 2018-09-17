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
#include "sdl2_ui.h"
#elif USE_SDL==1
#include "sdl_ui.h"
#elif _3DS
#include "3ds_ui.h"
#elif PSP2
#include "psp2_ui.h"
#elif SWITCH
#include "switch_ui.h"
#endif

std::shared_ptr<BaseUi> DisplayUi;

std::shared_ptr<BaseUi> BaseUi::CreateUi(long width, long height, bool fs_flag, int zoom) {
#if USE_SDL!=2
	/* unused */
	(void) zoom;
#endif
#if USE_SDL==2
	return std::make_shared<Sdl2Ui>(width, height, fs_flag, zoom);
#elif USE_SDL==1
	return std::make_shared<SdlUi>(width, height, fs_flag);
#elif _3DS
	return std::make_shared<CtrUi>(width, height);
#elif PSP2
	return std::make_shared<Psp2Ui>(width, height);
#elif SWITCH
	return std::make_shared<NxUi>(width, height);
#else
#error cannot create UI
#endif
}

BaseUi::BaseUi()
	: mouse_focus(false)
	, mouse_x(0)
	, mouse_y(0)
	, cursor_visible(false)
	, back_color(0, 0, 0, 255)
{
	keys.reset();
}

BaseUi::KeyStatus& BaseUi::GetKeyStates() {
	return keys;
}

BitmapRef const& BaseUi::GetDisplaySurface() const {
	return main_surface;
}

BitmapRef& BaseUi::GetDisplaySurface() {
	return main_surface;
}

BitmapRef BaseUi::CaptureScreen() {
	return Bitmap::Create(*main_surface, main_surface->GetRect());
}

long BaseUi::GetWidth() const {
	return current_display_mode.width;
}

long BaseUi::GetHeight() const {
	return current_display_mode.height;
}

bool BaseUi::GetMouseFocus() const {
	return mouse_focus;
}

int BaseUi::GetMousePosX() const {
	return mouse_x;
}

int BaseUi::GetMousePosY() const {
	return mouse_y;
}

Color const& BaseUi::GetBackcolor() const {
	return back_color;
}
void BaseUi::SetBackcolor(const Color &color) {
	back_color = color;
}

void BaseUi::CleanDisplay() {
	main_surface->Clear();
}

void BaseUi::AddBackground() {
	main_surface->Fill(back_color);
}
