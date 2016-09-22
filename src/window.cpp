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
#include <cmath>
#include "system.h"
#include "graphics.h"
#include "player.h"
#include "rect.h"
#include "util_macro.h"
#include "window.h"
#include "bitmap.h"

Window::Window():
	type(TypeWindow),
	stretch(true),
	active(true),
	visible(true),
	pause(false),
	closing(false),
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

	Graphics::RegisterDrawable(this);

	background = BitmapRef();
	frame_down = BitmapRef();
	frame_up = BitmapRef();
	frame_left = BitmapRef();
	frame_right = BitmapRef();
	cursor1 = BitmapRef();
	cursor2 = BitmapRef();
}

Window::~Window() {
	Graphics::RemoveDrawable(this);
}

void Window::SetOpenAnimation(int frames) {
	closing = false;
	visible = true;

	if (frames > 0) {
		animation_frames = frames;
		animation_count = 0.0;
		animation_increment = (height / 2.0) / frames;
	}
	else {
		animation_frames = 0;
	}
}

void Window::SetCloseAnimation(int frames) {
	if (frames > 0) {
		closing = true;
		animation_frames = frames;
		animation_count = (height / 2.0);
		animation_increment = - animation_count / frames;
	} else {
		visible = false;
	}
}

void Window::Draw() {
	if (!visible) return;
	if (width <= 0 || height <= 0) return;
	if (x < -width || x > DisplayUi->GetWidth() || y < -height || y > DisplayUi->GetHeight()) return;

	BitmapRef dst = DisplayUi->GetDisplaySurface();

	if (windowskin) {
		if (width > 4 && height > 4 && (back_opacity * opacity / 255 > 0)) {
			if (background_needs_refresh) RefreshBackground();

			if (animation_frames > 0) {
				int ianimation_count = (int)animation_count;

				Rect src_rect(0, height / 2 - ianimation_count, width, ianimation_count * 2);

				dst->Blit(x, y + src_rect.y, *background, src_rect, back_opacity * opacity / 255);
			} else {
				dst->Blit(x, y, *background, background->GetRect(), back_opacity * opacity / 255);
			}
		}

		if (width > 0 && height > 0 && opacity > 0) {
			if (frame_needs_refresh) RefreshFrame();

			if (animation_frames > 0) {
				int ianimation_count = (int)animation_count;

				if (ianimation_count > 8) {
					Rect src_rect(0, height / 2 - ianimation_count, 8, ianimation_count * 2 - 16);

					dst->Blit(x, y + 8 + src_rect.y, *frame_left, src_rect, opacity);
					dst->Blit(x + width - 8, y + 8 + src_rect.y, *frame_right, src_rect, opacity);

					dst->Blit(x, y + height / 2 - ianimation_count, *frame_up, frame_up->GetRect(), opacity);
					dst->Blit(x, y + height / 2 + ianimation_count - 8, *frame_down, frame_down->GetRect(), opacity);
				} else {
					dst->Blit(x, y + height / 2 - ianimation_count, *frame_up, Rect(0, 0, width, ianimation_count), opacity);
					dst->Blit(x, y + height / 2 , *frame_down, Rect(0, 8 - ianimation_count, width, ianimation_count), opacity);
				}
			} else {
				dst->Blit(x, y, *frame_up, frame_up->GetRect(), opacity);
				dst->Blit(x, y + height - 8, *frame_down, frame_down->GetRect(), opacity);
				dst->Blit(x, y + 8, *frame_left, frame_left->GetRect(), opacity);
				dst->Blit(x + width - 8, y + 8, *frame_right, frame_right->GetRect(), opacity);
			}
		}

		if (width >= 16 && height > 16 && cursor_rect.width > 4 && cursor_rect.height > 4 && animation_frames == 0) {
			if (cursor_needs_refresh) RefreshCursor();

			Rect src_rect(
				-min(cursor_rect.x + border_x, 0),
				-min(cursor_rect.y + border_y, 0),
				min(cursor_rect.width, width - cursor_rect.x + border_x),
				min(cursor_rect.height, height - cursor_rect.y + border_y)
			);

			if (cursor_frame <= 10)
				dst->Blit(x + cursor_rect.x + border_x, y + cursor_rect.y + border_y, *cursor1, src_rect, 255);
			else
				dst->Blit(x + cursor_rect.x + border_x, y + cursor_rect.y + border_y, *cursor2, src_rect, 255);
		}
	}

	if (contents) {
		if (width > 2 * border_x && height > 2 * border_y &&
			-ox < width - 2 * border_x && -oy < height - 2 * border_y &&
			contents_opacity > 0 && animation_frames == 0) {
			Rect src_rect(-min(-ox, 0), -min(-oy, 0),
						  min(width - 2 * border_x, width - 2 * border_x + ox),
						  min(height - 2 * border_y, height - 2 * border_y + oy));

			dst->Blit(max(x + border_x, x + border_x - ox),
					  max(y + border_y, y + border_y - oy),
					  *contents, src_rect, contents_opacity);
		}
	}

	if (pause && pause_frame > 16 && animation_frames <= 0) {
		Rect src_rect(40, 16, 16, 8);
		dst->Blit(x + width / 2 - 8, y + height - 8, *windowskin, src_rect, 255);
	}

	if (up_arrow) {
		Rect src_rect(40, 8, 16, 8);
		dst->Blit(x + width / 2 - 8, y, *windowskin, src_rect, 255);
	}

	if (down_arrow) {
		Rect src_rect(40, 16, 16, 8);
		dst->Blit(x + width / 2 - 8, y + height - 8, *windowskin, src_rect, 255);
	}
}

void Window::RefreshBackground() {
	background_needs_refresh = false;

	BitmapRef bitmap = Bitmap::Create(width, height, false);

	if (stretch) {
		bitmap->StretchBlit(*windowskin, Rect(0, 0, 32, 32), 255);
	} else {
		bitmap->TiledBlit(Rect(0, 0, 32, 32), *windowskin, bitmap->GetRect(), 255);
	}

	background = bitmap;
}

void Window::RefreshFrame() {
	frame_needs_refresh = false;

	BitmapRef up_bitmap = Bitmap::Create(width, 8);
	BitmapRef down_bitmap = Bitmap::Create(width, 8);

	up_bitmap->Clear();
	down_bitmap->Clear();

	Rect src_rect, dst_rect;

	// Border Up
	src_rect.Set(32 + 8, 0, 16, 8);
	dst_rect.Set(8, 0, max(width - 16, 1), 8);
	up_bitmap->TiledBlit(8, 0, src_rect, *windowskin, dst_rect, 255);

	// Border Down
	src_rect.Set(32 + 8, 32 - 8, 16, 8);
	dst_rect.Set(8, 0, max(width - 16, 1), 8);
	down_bitmap->TiledBlit(8, 0, src_rect, *windowskin, dst_rect, 255);

	// Upper left corner
	up_bitmap->Blit(0, 0, *windowskin, Rect(32, 0, 8, 8), 255);

	// Upper right corner
	up_bitmap->Blit(width - 8, 0, *windowskin, Rect(64 - 8, 0, 8, 8), 255);

	// Lower left corner
	down_bitmap->Blit(0, 0, *windowskin, Rect(32, 32 - 8, 8, 8), 255);

	// Lower right corner
	down_bitmap->Blit(width - 8, 0, *windowskin, Rect(64 - 8, 32 - 8, 8, 8), 255);

	frame_up = up_bitmap;
	frame_down = down_bitmap;

	if (height > 16) {
		BitmapRef left_bitmap = Bitmap::Create(8, height - 16);
		BitmapRef right_bitmap = Bitmap::Create(8, height - 16);

		left_bitmap->Clear();
		right_bitmap->Clear();

		// Border Left
		src_rect.Set(32, 8, 8, 16);
		dst_rect.Set(0, 0, 8, height - 16);
		left_bitmap->TiledBlit(0, 8, src_rect, *windowskin, dst_rect, 255);

		// Border Right
		src_rect.Set(64 - 8, 8, 8, 16);
		dst_rect.Set(0, 0, 8, height - 16);
		right_bitmap->TiledBlit(0, 8, src_rect, *windowskin, dst_rect, 255);

		frame_left = left_bitmap;
		frame_right = right_bitmap;
	} else {
		frame_left = BitmapRef();
		frame_right = BitmapRef();
	}
}

void Window::RefreshCursor() {
	cursor_needs_refresh = false;

	int cw = cursor_rect.width;
	int ch = cursor_rect.height;

	BitmapRef cursor1_bitmap = Bitmap::Create(cw, ch);
	BitmapRef cursor2_bitmap = Bitmap::Create(cw, ch);

	cursor1_bitmap->Clear();
	cursor2_bitmap->Clear();

	Rect dst_rect;

	// Border Up
	dst_rect.Set(8, 0, cw - 16, 8);
	cursor1_bitmap->TiledBlit(8, 0, Rect(64 + 8, 0, 16, 8), *windowskin, dst_rect, 255);
	cursor2_bitmap->TiledBlit(8, 0, Rect(96 + 8, 0, 16, 8), *windowskin, dst_rect, 255);

	// Border Down
	dst_rect.Set(8, ch - 8, cw - 16, 8);
	cursor1_bitmap->TiledBlit(8, 0, Rect(64 + 8, 32 - 8, 16, 8), *windowskin, dst_rect, 255);
	cursor2_bitmap->TiledBlit(8, 0, Rect(96 + 8, 32 - 8, 16, 8), *windowskin, dst_rect, 255);

	// Border Left
	dst_rect.Set(0, 8, 8, ch - 16);
	cursor1_bitmap->TiledBlit(0, 8, Rect(64, 8, 8, 16), *windowskin, dst_rect, 255);
	cursor2_bitmap->TiledBlit(0, 8, Rect(96, 8, 8, 16), *windowskin, dst_rect, 255);

	// Border Right
	dst_rect.Set(cw - 8, 8, 8, ch - 16);
	cursor1_bitmap->TiledBlit(0, 8, Rect(96 - 8, 8, 8, 16), *windowskin, dst_rect, 255);
	cursor2_bitmap->TiledBlit(0, 8, Rect(128 - 8, 8, 8, 16), *windowskin, dst_rect, 255);

	// Upper left corner
	cursor1_bitmap->Blit(0, 0, *windowskin, Rect(64, 0, 8, 8), 255);
	cursor2_bitmap->Blit(0, 0, *windowskin, Rect(96, 0, 8, 8), 255);

	// Upper right corner
	cursor1_bitmap->Blit(cw - 8, 0, *windowskin, Rect(96 - 8, 0, 8, 8), 255);
	cursor2_bitmap->Blit(cw - 8, 0, *windowskin, Rect(128 - 8, 0, 8, 8), 255);

	// Lower left corner
	cursor1_bitmap->Blit(0, ch - 8, *windowskin, Rect(64, 32 - 8, 8, 8), 255);
	cursor2_bitmap->Blit(0, ch - 8, *windowskin, Rect(96, 32 - 8, 8, 8), 255);

	// Lower right corner
	cursor1_bitmap->Blit(cw - 8, ch - 8, *windowskin, Rect(96 - 8, 32 - 8, 8, 8), 255);
	cursor2_bitmap->Blit(cw - 8, ch - 8, *windowskin, Rect(128 - 8, 32 - 8, 8, 8), 255);

	// Background
	dst_rect.Set(8, 8, cw - 16, ch - 16);
	cursor1_bitmap->TiledBlit(8, 8, Rect(64 + 8, 8, 16, 16), *windowskin, dst_rect, 255);
	cursor2_bitmap->TiledBlit(8, 8, Rect(96 + 8, 8, 16, 16), *windowskin, dst_rect, 255);

	cursor1 = cursor1_bitmap;
	cursor2 = cursor2_bitmap;
}

void Window::Update() {
	if (active) {
		cursor_frame += 1;
		if (cursor_frame > 20) cursor_frame = 0;
		if (pause) {
			pause_frame += 1;
			if (pause_frame == 40) pause_frame = 0;
		}
	}

	if (animation_frames > 0) {
		// Open/Close Animation
		animation_frames -= 1;
		animation_count += animation_increment;
		if (closing && animation_frames <= 0) {
			visible = false;
			closing = false;
		}
	}
}

BitmapRef const& Window::GetWindowskin() const {
	return windowskin;
}
void Window::SetWindowskin(BitmapRef const& nwindowskin) {
	background_needs_refresh = true;
	frame_needs_refresh = true;
	cursor_needs_refresh = true;
	windowskin = nwindowskin;
}

BitmapRef Window::GetContents() const {
	return contents;
}
void Window::SetContents(BitmapRef const& ncontents) {
	contents = ncontents;
}

bool Window::GetStretch() const {
	return stretch;
}
void Window::SetStretch(bool nstretch) {
	if (stretch != nstretch) background_needs_refresh = true;
	stretch = nstretch;
}

Rect const& Window::GetCursorRect() const {
	return cursor_rect;
}
void Window::SetCursorRect(Rect const& ncursor_rect) {
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
	if (z != nz) Graphics::UpdateZCallback();
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
}

int Window::GetBackOpacity() const {
	return back_opacity;
}
void Window::SetBackOpacity(int nback_opacity) {
	back_opacity = nback_opacity;
}

int Window::GetContentsOpacity() const {
	return contents_opacity;
}
void Window::SetContentsOpacity(int ncontents_opacity) {
	contents_opacity = ncontents_opacity;
}

DrawableType Window::GetType() const {
	return type;
}
