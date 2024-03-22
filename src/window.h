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

#ifndef EP_WINDOW_H
#define EP_WINDOW_H

// Headers
#include "system.h"
#include "drawable.h"
#include "rect.h"

class Scene;

/**
 * Window class.
 */
class Window : public Drawable {
public:
	enum class WindowType {
		Selectable, // Currently the only type that matters (for mouse selection)
		Unknown
	};

	Window(Scene* parent, WindowType type = WindowType::Unknown, Drawable::Flags flags = Drawable::Flags::Default);
	virtual ~Window();

	void Draw(Bitmap& dst) override;

	virtual void Update();
	BitmapRef const& GetWindowskin() const;
	void SetWindowskin(BitmapRef const& nwindowskin);
	BitmapRef GetContents() const;
	void SetContents(BitmapRef const& ncontents);
	bool GetStretch() const;
	void SetStretch(bool nstretch);
	Rect const& GetCursorRect() const;
	void SetCursorRect(Rect const& ncursor_rect);
	bool GetActive() const;
	void SetActive(bool nactive);
	bool GetPause() const;
	void SetPause(bool npause);
	bool GetUpArrow() const;
	void SetUpArrow(bool nup_arrow);
	bool GetDownArrow() const;
	void SetDownArrow(bool ndown_arrow);
	bool GetLeftArrow() const;
	void SetLeftArrow(bool nleft_arrow);
	bool GetRightArrow() const;
	void SetRightArrow(bool nright_arrow);
	int GetX() const;
	void SetX(int nx);
	int GetY() const;
	void SetY(int ny);
	int GetWidth() const;
	void SetWidth(int nwidth);
	int GetHeight() const;
	void SetHeight(int nheight);
	int GetOx() const;
	void SetOx(int nox);
	int GetOy() const;
	void SetOy(int noy);
	int GetBorderX() const;
	void SetBorderX(int nox);
	int GetBorderY() const;
	void SetBorderY(int noy);
	int GetOpacity() const;
	void SetOpacity(int nopacity);
	int GetFrameOpacity() const;
	void SetFrameOpacity(int nframe_opacity);
	int GetBackOpacity() const;
	void SetBackOpacity(int nback_opacity);
	int GetContentsOpacity() const;
	void SetContentsOpacity(int ncontents_opacity);
	void SetOpenAnimation(int frames);
	void SetCloseAnimation(int frames);
	Scene* GetScene() const;
	void SetScene(Scene* scene);
	WindowType GetType() const;

	bool IsOpening() const;
	bool IsClosing() const;
	bool IsOpeningOrClosing() const;

protected:
	virtual bool IsSystemGraphicUpdateAllowed() const;

	unsigned long ID;
	BitmapRef windowskin, contents;
	bool stretch = true;
	Rect cursor_rect;
	bool active = true;
	bool closing = false;
	bool up_arrow = false;
	bool down_arrow = false;
	bool left_arrow = false;
	bool right_arrow = false;
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
	int ox = 0;
	int oy = 0;
	int border_x = 8;
	int border_y = 8;
	int opacity = 255;
	int frame_opacity = 255;
	int back_opacity = 255;
	int contents_opacity = 255;
	Scene* scene = nullptr;

private:
	BitmapRef
		background, frame_down,
		frame_up, frame_left, frame_right, cursor1, cursor2;

	void RefreshBackground();
	void RefreshFrame();
	void RefreshCursor();

	bool background_needs_refresh;
	bool frame_needs_refresh;
	bool cursor_needs_refresh;
	bool pause = false;

	int cursor_frame = 0;
	int pause_frame = 0;
	int animation_frames = 0;
	double animation_count = 0.0;
	double animation_increment = 0.0;
	WindowType type = WindowType::Unknown;
};

inline bool Window::IsOpening() const {
	return animation_frames > 0 && !closing;
}

inline bool Window::IsClosing() const {
	return animation_frames > 0 && closing;
}

inline bool Window::IsOpeningOrClosing() const {
	return animation_frames > 0;
}

inline BitmapRef const& Window::GetWindowskin() const {
	return windowskin;
}

inline BitmapRef Window::GetContents() const {
	return contents;
}

inline void Window::SetContents(BitmapRef const& ncontents) {
	contents = ncontents;
}

inline bool Window::GetStretch() const {
	return stretch;
}

inline Rect const& Window::GetCursorRect() const {
	return cursor_rect;
}

inline bool Window::GetActive() const {
	return active;
}

inline void Window::SetActive(bool nactive) {
	active = nactive;
}

inline bool Window::GetPause() const {
	return pause;
}

inline void Window::SetPause(bool npause) {
	pause = npause;
	pause_frame = 0;
}

inline bool Window::GetUpArrow() const {
	return up_arrow;
}

inline void Window::SetUpArrow(bool nup_arrow) {
	up_arrow = nup_arrow;
}

inline bool Window::GetDownArrow() const {
	return down_arrow;
}

inline void Window::SetDownArrow(bool ndown_arrow) {
	down_arrow = ndown_arrow;
}

inline bool Window::GetLeftArrow() const {
	return left_arrow;
}

inline void Window::SetLeftArrow(bool nleft_arrow) {
	left_arrow = nleft_arrow;
}

inline bool Window::GetRightArrow() const {
	return right_arrow;
}

inline void Window::SetRightArrow(bool nright_arrow) {
	right_arrow = nright_arrow;
}

inline int Window::GetX() const {
	return x;
}

inline void Window::SetX(int nx) {
	x = nx;
}

inline int Window::GetY() const {
	return y;
}

inline void Window::SetY(int ny) {
	y = ny;
}

inline int Window::GetWidth() const {
	return width;
}

inline int Window::GetHeight() const {
	return height;
}

inline int Window::GetOx() const {
	return ox;
}

inline void Window::SetOx(int nox) {
	ox = nox;
}

inline int Window::GetOy() const {
	return oy;
}

inline void Window::SetOy(int noy) {
	oy = noy;
}

inline int Window::GetBorderX() const {
	return border_x;
}

inline void Window::SetBorderX(int x) {
	border_x = x;
}

inline int Window::GetBorderY() const {
	return border_y;
}

inline void Window::SetBorderY(int y) {
	border_y = y;
}

inline int Window::GetOpacity() const {
	return opacity;
}

inline void Window::SetOpacity(int nopacity) {
	opacity = nopacity;
}

inline int Window::GetFrameOpacity() const {
	return frame_opacity;
}

inline void Window::SetFrameOpacity(int nframe_opacity) {
	frame_opacity = nframe_opacity;
}

inline int Window::GetBackOpacity() const {
	return back_opacity;
}

inline void Window::SetBackOpacity(int nback_opacity) {
	back_opacity = nback_opacity;
}

inline int Window::GetContentsOpacity() const {
	return contents_opacity;
}

inline void Window::SetContentsOpacity(int ncontents_opacity) {
	contents_opacity = ncontents_opacity;
}

inline bool Window::IsSystemGraphicUpdateAllowed() const {
	return !IsClosing();
}

inline Window::WindowType Window::GetType() const {
	return type;
}

#endif
