/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <math.h>
#include "system.h"
#include "graphics.h"
#include "bitmap_screen.h"
#include "player.h"
#include "rect.h"
#include "util_macro.h"
#include "window.h"

////////////////////////////////////////////////////////////
Window::Window():
	type(TypeWindow),
	ID(Graphics::drawable_id++),
	windowskin(NULL),
	contents(NULL),
	stretch(true),
	active(true),
	visible(true),
	pause(false),
	up_arrow(false),
	down_arrow(false),
	x(0),
	y(0),
	width(0),
	height(0),
	z(0),
	ox(0),
	oy(0),
	border_x(8),
	border_y(8),
	opacity(255),
	back_opacity(255),
	contents_opacity(255),
	cursor_frame(0),
	pause_frame(0),
	animation_frames(0),
	animation_count(0.0),
	animation_increment(0.0) {

	zobj = Graphics::RegisterZObj(0, ID);
	Graphics::RegisterDrawable(ID, this);

	windowskin_screen = BitmapScreen::CreateBitmapScreen();
	contents_screen = BitmapScreen::CreateBitmapScreen();

	background = BitmapScreen::CreateBitmapScreen();
	frame_down = BitmapScreen::CreateBitmapScreen();
	frame_up = BitmapScreen::CreateBitmapScreen();
	frame_left = BitmapScreen::CreateBitmapScreen();
	frame_right = BitmapScreen::CreateBitmapScreen();
	cursor1 = BitmapScreen::CreateBitmapScreen();
	cursor2 = BitmapScreen::CreateBitmapScreen();
}

////////////////////////////////////////////////////////////
Window::~Window() {
	Graphics::RemoveZObj(ID);
	Graphics::RemoveDrawable(ID);

	delete windowskin_screen;
	delete contents_screen;
	delete background;
	delete cursor1;
	delete cursor2;
	delete frame_down;
	delete frame_up;
	delete frame_left;
	delete frame_right;
}

////////////////////////////////////////////////////////////
void Window::SetOpenAnimation(int frames) {
	animation_frames = frames;
	animation_count = 0.0;
	animation_increment = (height / 2.0) / frames;
}

////////////////////////////////////////////////////////////
void Window::SetCloseAnimation(int frames) {
	(void)frames;
	// ToDo
}

////////////////////////////////////////////////////////////
void Window::Draw(int z_order) {
	if (!visible) return;
	if (width <= 0 || height <= 0) return;
	if (x < -width || x > DisplayUi->GetWidth() || y < -height || y > DisplayUi->GetHeight()) return;


	if (windowskin != NULL) {
		if (width > 4 && height > 4 && (back_opacity * opacity / 255 > 0)) {
			if (background_needs_refresh) RefreshBackground();

			if (animation_frames > 0) {
				int ianimation_count = (int)animation_count;

				Rect src_rect(0, height / 2 - ianimation_count, width, ianimation_count * 2);

				background->BlitScreen(x, y + src_rect.y, src_rect);
			} else {
				background->BlitScreen(x, y);
			}
		}

		if (width > 0 && height > 0 && opacity > 0) {
			if (frame_needs_refresh) RefreshFrame();

			if (animation_frames > 0) {
				int ianimation_count = (int)animation_count;

				if (ianimation_count > 8) {
					Rect src_rect(0, height / 2 - ianimation_count, 8, ianimation_count * 2 - 16);

					frame_left->BlitScreen(x, y + 8 + src_rect.y, src_rect);
					frame_right->BlitScreen(x + width - 8, y + 8 + src_rect.y, src_rect);

					frame_up->BlitScreen(x, y + height / 2 - ianimation_count);
					frame_down->BlitScreen(x, y + height / 2 + ianimation_count - 8);
				} else {
					frame_up->BlitScreen(x, y + height / 2 - ianimation_count, Rect(0, 0, width, ianimation_count));
					frame_down->BlitScreen(x, y + height / 2, Rect(0, 8 - ianimation_count, width, ianimation_count));
				}
			} else {
				frame_up->BlitScreen(x, y);
				frame_down->BlitScreen(x, y + height - 8);
				frame_left->BlitScreen(x, y + 8);
				frame_right->BlitScreen(x + width - 8, y + 8);
			}
		}

		if (width > 16 && height > 16 && cursor_rect.width > 4 && cursor_rect.height > 4 && animation_frames == 0) {
			if (cursor_needs_refresh) RefreshCursor();

			Rect src_rect(
				-min(cursor_rect.x + border_x, 0),
				-min(cursor_rect.y + border_y, 0),
				min(cursor_rect.width, width - cursor_rect.x + border_x),
				min(cursor_rect.height, height - cursor_rect.y + border_y)
			);

			if (cursor_frame < 16)
				cursor1->BlitScreen(x + cursor_rect.x + border_x, y + cursor_rect.y + border_y, src_rect);
			else
				cursor2->BlitScreen(x + cursor_rect.x + border_x, y + cursor_rect.y + border_y, src_rect);
		}
	}

	if (contents != NULL) {
		if (width > 2 * border_x && height > 2 * border_y &&
			-ox < width - 2 * border_x && -oy < height - 2 * border_y &&
			contents_opacity > 0 && animation_frames == 0) {
			Rect src_rect(-min(-ox, 0), -min(-oy, 0),
						  min(width - 2 * border_x, width - 2 * border_x + ox),
						  min(height - 2 * border_y, height - 2 * border_y + oy));

			contents_screen->SetOpacityEffect(contents_opacity);

			contents_screen->BlitScreen(max(x + border_x, x + border_x - ox),
										max(y + border_y, y + border_y - oy), src_rect);
		}
	}

	if (pause && pause_frame > 16 && animation_frames <= 0) {
		Rect src_rect(40, 16, 16, 8);
		windowskin_screen->BlitScreen(x + width / 2 - 8, y + height - 8, src_rect);
	}

	if (up_arrow) {
		Rect src_rect(40, 8, 16, 8);
		windowskin_screen->BlitScreen(x + width / 2 - 8, y, src_rect);
	}

	if (down_arrow) {
		Rect src_rect(40, 16, 16, 8);
		windowskin_screen->BlitScreen(x + width / 2 - 8, y + height - 8, src_rect);
	}

	if (animation_frames > 0) {
		// Open Animation
		animation_frames -= 1;
		animation_count += animation_increment;
	}
}

////////////////////////////////////////////////////////////
void Window::RefreshBackground() {
	background_needs_refresh = false;

	Surface* bitmap = Surface::CreateSurface(width, height);

	if (stretch) {
		bitmap->StretchBlit(windowskin, Rect(0, 0, 32, 32), 255);
	} else {
		bitmap->TiledBlit(Rect(0, 0, 16, 16), windowskin, bitmap->GetRect(), 255);
	}

	background->SetBitmap(bitmap, true);
}

////////////////////////////////////////////////////////////
void Window::RefreshFrame() {
	frame_needs_refresh = false;

	Surface* up_bitmap = Surface::CreateSurface(width, 8);
	Surface* down_bitmap = Surface::CreateSurface(width, 8);

	up_bitmap->SetTransparentColor(windowskin->GetTransparentColor());
	down_bitmap->SetTransparentColor(windowskin->GetTransparentColor());

	up_bitmap->Clear();
	down_bitmap->Clear();

	Rect src_rect, dst_rect;

	// Border Up
	src_rect.Set(32 + 8, 0, 16, 8);
	dst_rect.Set(8, 0, max(width - 16, 1), 8);
	up_bitmap->TiledBlit(8, 0, src_rect, windowskin, dst_rect, 255);

	// Border Down
	src_rect.Set(32 + 8, 32 - 8, 16, 8);
	dst_rect.Set(8, 0, max(width - 16, 1), 8);
	down_bitmap->TiledBlit(8, 0, src_rect, windowskin, dst_rect, 255);

	// Upper left corner
	up_bitmap->Blit(0, 0, windowskin, Rect(32, 0, 8, 8), 255);

	// Upper right corner
	up_bitmap->Blit(width - 8, 0, windowskin, Rect(64 - 8, 0, 8, 8), 255);

	// Lower left corner
	down_bitmap->Blit(0, 0, windowskin, Rect(32, 32 - 8, 8, 8), 255);

	// Lower right corner
	down_bitmap->Blit(width - 8, 0, windowskin, Rect(64 - 8, 32 - 8, 8, 8), 255);

	frame_up->SetBitmap(up_bitmap, true);
	frame_down->SetBitmap(down_bitmap, true);

	if (height > 16) {
		Surface* left_bitmap = Surface::CreateSurface(8, height - 16);
		Surface* right_bitmap = Surface::CreateSurface(8, height - 16);

		left_bitmap->SetTransparentColor(windowskin->GetTransparentColor());
		right_bitmap->SetTransparentColor(windowskin->GetTransparentColor());

		left_bitmap->Clear();
		right_bitmap->Clear();

		// Border Left
		src_rect.Set(32, 8, 8, 16);
		dst_rect.Set(0, 0, 8, height - 16);
		left_bitmap->TiledBlit(0, 8, src_rect, windowskin, dst_rect, 255);

		// Border Right
		src_rect.Set(64 - 8, 8, 8, 16);
		dst_rect.Set(0, 0, 8, height - 16);
		right_bitmap->TiledBlit(0, 8, src_rect, windowskin, dst_rect, 255);

		frame_left->SetBitmap(left_bitmap, true);
		frame_right->SetBitmap(right_bitmap, true);
	} else {
		frame_left->SetBitmap(NULL);
		frame_right->SetBitmap(NULL);
	}
}

////////////////////////////////////////////////////////////
void Window::RefreshCursor() {
	cursor_needs_refresh = false;

	int cw = cursor_rect.width;
	int ch = cursor_rect.height;

	Surface* cursor1_bitmap = Surface::CreateSurface(cw, ch);
	Surface* cursor2_bitmap = Surface::CreateSurface(cw, ch);

	cursor1_bitmap->SetTransparentColor(windowskin->GetTransparentColor());
	cursor2_bitmap->SetTransparentColor(windowskin->GetTransparentColor());

	cursor1_bitmap->Clear();
	cursor2_bitmap->Clear();

	Rect dst_rect;

	// Border Up
	dst_rect.Set(8, 0, cw - 16, 8);
	cursor1_bitmap->TiledBlit(8, 0, Rect(64 + 8, 0, 16, 8), windowskin, dst_rect, 255);
	cursor2_bitmap->TiledBlit(8, 0, Rect(96 + 8, 0, 16, 8), windowskin, dst_rect, 255);

	// Border Down
	dst_rect.Set(8, ch - 8, cw - 16, 8);
	cursor1_bitmap->TiledBlit(8, 0, Rect(64 + 8, 32 - 8, 16, 8), windowskin, dst_rect, 255);
	cursor2_bitmap->TiledBlit(8, 0, Rect(96 + 8, 32 - 8, 16, 8), windowskin, dst_rect, 255);

	// Border Left
	dst_rect.Set(0, 8, 8, ch - 16);
	cursor1_bitmap->TiledBlit(0, 8, Rect(64, 8, 8, 16), windowskin, dst_rect, 255);
	cursor2_bitmap->TiledBlit(0, 8, Rect(96, 8, 8, 16), windowskin, dst_rect, 255);

	// Border Right
	dst_rect.Set(cw - 8, 8, 8, ch - 16);
	cursor1_bitmap->TiledBlit(0, 8, Rect(96 - 8, 8, 8, 16), windowskin, dst_rect, 255);
	cursor2_bitmap->TiledBlit(0, 8, Rect(128 - 8, 8, 8, 16), windowskin, dst_rect, 255);

	// Upper left corner
	cursor1_bitmap->Blit(0, 0, windowskin, Rect(64, 0, 8, 8), 255);
	cursor2_bitmap->Blit(0, 0, windowskin, Rect(96, 0, 8, 8), 255);

	// Upper right corner
	cursor1_bitmap->Blit(cw - 8, 0, windowskin, Rect(96 - 8, 0, 8, 8), 255);
	cursor2_bitmap->Blit(cw - 8, 0, windowskin, Rect(128 - 8, 0, 8, 8), 255);

	// Lower left corner
	cursor1_bitmap->Blit(0, ch - 8, windowskin, Rect(64, 32 - 8, 8, 8), 255);
	cursor2_bitmap->Blit(0, ch - 8, windowskin, Rect(96, 32 - 8, 8, 8), 255);

	// Lower right corner
	cursor1_bitmap->Blit(cw - 8, ch - 8, windowskin, Rect(96 - 8, 32 - 8, 8, 8), 255);
	cursor2_bitmap->Blit(cw - 8, ch - 8, windowskin, Rect(128 - 8, 32 - 8, 8, 8), 255);

	// Background
	dst_rect.Set(8, 8, cw - 16, ch - 16);
	cursor1_bitmap->TiledBlit(8, 8, Rect(64 + 8, 8, 16, 16), windowskin, dst_rect, 255);
	cursor2_bitmap->TiledBlit(8, 8, Rect(96 + 8, 8, 16, 16), windowskin, dst_rect, 255);

	cursor1->SetBitmap(cursor1_bitmap, true);
	cursor2->SetBitmap(cursor2_bitmap, true);
}

////////////////////////////////////////////////////////////
void Window::Update() {
	if (active) {
		cursor_frame += 1;
		if (cursor_frame > 32) cursor_frame = 0;
		if (pause) {
			pause_frame += 1;
			if (pause_frame == 40) pause_frame = 0;
		}
	}
}

////////////////////////////////////////////////////////////
Bitmap* Window::GetWindowskin() const {
	return windowskin;
}
void Window::SetWindowskin(Bitmap* nwindowskin, bool delete_bitmap) {
	background_needs_refresh = true;
	frame_needs_refresh = true;
	cursor_needs_refresh = true;
	windowskin = nwindowskin;
	windowskin_screen->SetBitmap(windowskin, delete_bitmap);
}

Surface* Window::GetContents() const {
	return contents;
}
void Window::SetContents(Surface* ncontents) {
	contents = ncontents;
	contents_screen->SetBitmap(contents, true);
}

bool Window::GetStretch() const {
	return stretch;
}
void Window::SetStretch(bool nstretch) {
	if (stretch != nstretch) background_needs_refresh = true;
	stretch = nstretch;
}

Rect Window::GetCursorRect() const {
	return cursor_rect;
}
void Window::SetCursorRect(Rect ncursor_rect) {
	if (cursor_rect.width != ncursor_rect.width || cursor_rect.height != ncursor_rect.height) cursor_needs_refresh = true;
	cursor_rect = ncursor_rect;
}

bool Window::GetActive() const {
	return active;
}
void Window::SetActive(bool nactive) {
	active = nactive;
}

bool Window::GetVisible() const {
	return visible;
}
void Window::SetVisible(bool nvisible) {
	visible = nvisible;
}

bool Window::GetPause() const {
	return pause;
}
void Window::SetPause(bool npause) {
	pause = npause;
}

bool Window::GetUpArrow() const {
	return up_arrow;
}
void Window::SetUpArrow(bool nup_arrow) {
	up_arrow = nup_arrow;
}

bool Window::GetDownArrow() const {
	return down_arrow;
}
void Window::SetDownArrow(bool ndown_arrow) {
	down_arrow = ndown_arrow;
}

int Window::GetX() const {
	return x;
}
void Window::SetX(int nx) {
	x = nx;
}

int Window::GetY() const {
	return y;
}
void Window::SetY(int ny) {
	y = ny;
}

int Window::GetWidth() const {
	return width;
}
void Window::SetWidth(int nwidth) {
	if (width != nwidth) {
		background_needs_refresh = true;
		frame_needs_refresh = true;
	}
	width = nwidth;
}

int Window::GetHeight() const {
	return height;
}
void Window::SetHeight(int nheight) {
	if (height != nheight) {
		background_needs_refresh = true;
		frame_needs_refresh = true;
	}
	height = nheight;
}

int Window::GetZ() const {
	return z;
}
void Window::SetZ(int nz) {
	if (z != nz) Graphics::UpdateZObj(zobj, nz);
	z = nz;
}

int Window::GetOx() const {
	return ox;
}
void Window::SetOx(int nox) {
	ox = nox;
}

int Window::GetOy() const {
	return oy;
}
void Window::SetOy(int noy) {
	oy = noy;
}

int Window::GetBorderX() const {
	return border_x;
}
void Window::SetBorderX(int x) {
	border_x = x;
}

int Window::GetBorderY() const {
	return border_y;
}
void Window::SetBorderY(int y) {
	border_y = y;
}

int Window::GetOpacity() const {
	return opacity;
}
void Window::SetOpacity(int nopacity) {
	opacity = nopacity;

	background->SetOpacityEffect(back_opacity * opacity / 255);

	frame_up->SetOpacityEffect(opacity);
	frame_down->SetOpacityEffect(opacity);
	frame_left->SetOpacityEffect(opacity);
	frame_right->SetOpacityEffect(opacity);
}

int Window::GetBackOpacity() const {
	return back_opacity;
}
void Window::SetBackOpacity(int nback_opacity) {
	back_opacity = nback_opacity;

	background->SetOpacityEffect(back_opacity * opacity / 255);
}

int Window::GetContentsOpacity() const {
	return contents_opacity;
}
void Window::SetContentsOpacity(int ncontents_opacity) {
	contents_opacity = ncontents_opacity;
}

////////////////////////////////////////////////////////////
unsigned long Window::GetId() const {
	return ID;
}

DrawableType Window::GetType() const {
	return type;
}
