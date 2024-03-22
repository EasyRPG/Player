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
#include "player.h"
#include "rect.h"
#include "util_macro.h"
#include "window.h"
#include "bitmap.h"
#include "drawable_mgr.h"
#include "scene.h"

constexpr int pause_animation_frames = 20;

Window::Window(Scene* parent, WindowType type, Drawable::Flags flags):
	type(type), Drawable(Priority_Window, flags)
{
	DrawableMgr::Register(this);
	SetScene(parent);
}

Window::~Window() {
	if (scene) {
		scene->RemoveWindow(this);
	}
}

void Window::SetOpenAnimation(int frames) {
	closing = false;
	SetVisible(true);

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
		SetVisible(false);
	}
}

Scene* Window::GetScene() const {
	return scene;
}

void Window::SetScene(Scene* scene) {
	if (this->scene) {
		this->scene->RemoveWindow(this);
	}

	this->scene = nullptr;
	if (scene) {
		scene->RegisterWindow(this);
	}
}

void Window::Draw(Bitmap& dst) {
	if (width <= 0 || height <= 0) return;
	if (x < -width || x > dst.GetWidth() || y < -height || y > dst.GetHeight()) return;

	if (windowskin) {
		if (width > 4 && height > 4 && (back_opacity * opacity / 255 > 0)) {
			if (background_needs_refresh) RefreshBackground();

			if (animation_frames > 0) {
				int ianimation_count = (int)animation_count;

				Rect src_rect(0, height / 2 - ianimation_count, width, ianimation_count * 2);

				dst.Blit(x, y + src_rect.y, *background, src_rect, back_opacity * opacity / 255);
			} else {
				dst.Blit(x, y, *background, background->GetRect(), back_opacity * opacity / 255);
			}
		}

		if (width > 0 && height > 0 && opacity > 0) {
			if (frame_needs_refresh) RefreshFrame();

			int fopacity = frame_opacity * opacity / 255;

			if (animation_frames > 0) {
				int ianimation_count = (int)animation_count;

				if (ianimation_count > 8) {
					Rect src_rect(0, height / 2 - ianimation_count, 8, ianimation_count * 2 - 16);

					if (frame_left) {
						dst.Blit(x, y + 8 + src_rect.y, *frame_left, src_rect, fopacity);
					}

					if (frame_right) {
						dst.Blit(x + width - 8, y + 8 + src_rect.y, *frame_right, src_rect, fopacity);
					}

					dst.Blit(x, y + height / 2 - ianimation_count, *frame_up, frame_up->GetRect(), fopacity);
					dst.Blit(x, y + height / 2 + ianimation_count - 8, *frame_down, frame_down->GetRect(), fopacity);
				} else {
					dst.Blit(x, y + height / 2 - ianimation_count, *frame_up, Rect(0, 0, width, ianimation_count), fopacity);
					dst.Blit(x, y + height / 2 , *frame_down, Rect(0, 8 - ianimation_count, width, ianimation_count), fopacity);
				}
			} else {
				dst.Blit(x, y, *frame_up, frame_up->GetRect(), fopacity);
				dst.Blit(x, y + height - 8, *frame_down, frame_down->GetRect(), fopacity);

				if (frame_left) {
					dst.Blit(x, y + 8, *frame_left, frame_left->GetRect(), fopacity);
				}

				if (frame_right) {
					dst.Blit(x + width - 8, y + 8, *frame_right, frame_right->GetRect(), fopacity);
				}
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
				dst.Blit(x + cursor_rect.x + border_x, y + cursor_rect.y + border_y, *cursor1, src_rect, 255);
			else
				dst.Blit(x + cursor_rect.x + border_x, y + cursor_rect.y + border_y, *cursor2, src_rect, 255);
		}
	}

	if (contents) {
		if (width > 2 * border_x && height > 2 * border_y &&
			-ox < width - 2 * border_x && -oy < height - 2 * border_y &&
			contents_opacity > 0 && animation_frames == 0) {
			Rect src_rect(-min(-ox, 0), -min(-oy, 0),
						  min(width - 2 * border_x, width - 2 * border_x + ox),
						  min(height - 2 * border_y, height - 2 * border_y + oy));

			dst.Blit(max(x + border_x, x + border_x - ox),
					  max(y + border_y, y + border_y - oy),
					  *contents, src_rect, contents_opacity);
		}
	}

	if ((pause && pause_frame < pause_animation_frames && animation_frames <= 0) || down_arrow) {
		Rect src_rect(40, 16, 16, 8);
		dst.Blit(x + width / 2 - 8, y + height - 8, *windowskin, src_rect, 255);
	}

	if (up_arrow) {
		Rect src_rect(40, 8, 16, 8);
		dst.Blit(x + width / 2 - 8, y, *windowskin, src_rect, 255);
	}

	if (right_arrow) {
		Rect src_rect(40, 16, 16, 8);
		dst.RotateZoomOpacityBlit(x + width - 8, y + height / 2 - 8, 16, 0, *windowskin, src_rect, -M_PI / 2, 1.0, 1.0, 255);
	}

	if (left_arrow) {
		Rect src_rect(40, 8, 16, 8);
		dst.RotateZoomOpacityBlit(x, y + height / 2 - 8, 16, 0, *windowskin, src_rect, -M_PI / 2, 1.0, 1.0, 255);
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
	src_rect = { 32 + 8, 0, 16, 8 };
	dst_rect = { 8, 0, max(width - 16, 1), 8 };
	up_bitmap->TiledBlit(8, 0, src_rect, *windowskin, dst_rect, 255);

	// Border Down
	src_rect = { 32 + 8, 32 - 8, 16, 8 };
	dst_rect = { 8, 0, max(width - 16, 1), 8 };
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

		// Border Left
		src_rect = { 32, 8, 8, 16 };
		dst_rect = { 0, 0, 8, height - 16 };
		left_bitmap->TiledBlit(0, 8, src_rect, *windowskin, dst_rect, 255);

		// Border Right
		src_rect = { 64 - 8, 8, 8, 16 };
		dst_rect = { 0, 0, 8, height - 16 };
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
	dst_rect = { 8, 0, cw - 16, 8 };
	cursor1_bitmap->TiledBlit(8, 0, Rect(64 + 8, 0, 16, 8), *windowskin, dst_rect, 255);
	cursor2_bitmap->TiledBlit(8, 0, Rect(96 + 8, 0, 16, 8), *windowskin, dst_rect, 255);

	// Border Down
	dst_rect = { 8, ch - 8, cw - 16, 8 };
	cursor1_bitmap->TiledBlit(8, 0, Rect(64 + 8, 32 - 8, 16, 8), *windowskin, dst_rect, 255);
	cursor2_bitmap->TiledBlit(8, 0, Rect(96 + 8, 32 - 8, 16, 8), *windowskin, dst_rect, 255);

	// Border Left
	dst_rect = { 0, 8, 8, ch - 16 };
	cursor1_bitmap->TiledBlit(0, 8, Rect(64, 8, 8, 16), *windowskin, dst_rect, 255);
	cursor2_bitmap->TiledBlit(0, 8, Rect(96, 8, 8, 16), *windowskin, dst_rect, 255);

	// Border Right
	dst_rect = { cw - 8, 8, 8, ch - 16 };
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
	dst_rect = { 8, 8, cw - 16, ch - 16 };
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
			pause_frame = (pause_frame + 1) % (pause_animation_frames * 2);
		}
	}

	if (animation_frames > 0) {
		// Open/Close Animation
		animation_frames -= 1;
		animation_count += animation_increment;
		if (closing && animation_frames <= 0) {
			SetVisible(false);
			closing = false;
		}
	}
}

void Window::SetWindowskin(BitmapRef const& nwindowskin) {
	if (windowskin == nwindowskin) {
		return;
	}
	background_needs_refresh = true;
	frame_needs_refresh = true;
	cursor_needs_refresh = true;
	windowskin = nwindowskin;
}

void Window::SetStretch(bool nstretch) {
	if (stretch != nstretch) background_needs_refresh = true;
	stretch = nstretch;
}

void Window::SetCursorRect(Rect const& ncursor_rect) {
	if (cursor_rect.width != ncursor_rect.width || cursor_rect.height != ncursor_rect.height) cursor_needs_refresh = true;
	cursor_rect = ncursor_rect;
}

void Window::SetWidth(int nwidth) {
	if (width != nwidth) {
		background_needs_refresh = true;
		frame_needs_refresh = true;
	}
	width = nwidth;
}

void Window::SetHeight(int nheight) {
	if (height != nheight) {
		background_needs_refresh = true;
		frame_needs_refresh = true;
	}
	height = nheight;
}


