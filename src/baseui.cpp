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
#include "system.h"
#include "bitmap.h"

#ifdef USE_SDL
#include "sdl_ui.h"
#endif

EASYRPG_SHARED_PTR<BaseUi> DisplayUi;

EASYRPG_SHARED_PTR<BaseUi> BaseUi::CreateUi(long width, long height, const std::string& title, bool fs_flag, bool /* zoom */) {
#ifdef USE_SDL
	return EASYRPG_MAKE_SHARED<SdlUi>(width, height, title, fs_flag);
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
	main_surface->FillRect(main_surface->GetRect(), back_color);
}
