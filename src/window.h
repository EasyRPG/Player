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

#ifndef _WINDOW_H_
#define _WINDOW_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "drawable.h"
#include "bitmap.h"
#include "zobj.h"

////////////////////////////////////////////////////////////
/// Window class
////////////////////////////////////////////////////////////
class Window : public Drawable {
public:
	Window();
	virtual ~Window();

	void Draw(int z_order);

	void Update();
	Bitmap* GetWindowskin() const;
	void SetWindowskin(Bitmap* nwindowskin);
	Bitmap* GetContents() const;
	void SetContents(Bitmap* ncontents);
	bool GetStretch() const;
	void SetStretch(bool nstretch);
	Rect GetCursorRect() const;
	void SetCursorRect(Rect ncursor_rect);
	bool GetActive() const;
	void SetActive(bool nactive);
	bool GetVisible() const;
	void SetVisible(bool nvisible);
	bool GetPause() const;
	void SetPause(bool npause);
	int GetX() const;
	void SetX(int nx);
	int GetY() const;
	void SetY(int ny);
	int GetWidth() const;
	void SetWidth(int nwidth);
	int GetHeight() const;
	void SetHeight(int nheight);
	int GetZ() const;
	void SetZ(int nz);
	int GetOx() const;
	void SetOx(int nox);
	int GetOy() const;
	void SetOy(int noy);
	int GetOpacity() const;
	void SetOpacity(int nopacity);
	int GetBackOpacity() const;
	void SetBackOpacity(int nback_opacity);
	int GetContentsOpacity() const;
	void SetContentsOpacity(int ncontents_opacity);
	void SetAnimation(int frames);

	unsigned long GetId() const;
	DrawableType GetType() const;

protected:
	DrawableType type;
	unsigned long ID;
	ZObj* zobj;
	Bitmap* windowskin;
	Bitmap* contents;
	bool stretch;
	Rect cursor_rect;
	bool active;
	bool visible;
	bool pause;
	int x;
	int y;
	int width;
	int height;
	int z;
	int ox;
	int oy;
	int opacity;
	int back_opacity;
	int contents_opacity;

private:
	BitmapScreen* windowskin_screen;
	BitmapScreen* contents_screen;
	BitmapScreen* background;
	BitmapScreen* frame_down;
	BitmapScreen* frame_up;
	BitmapScreen* frame_left;
	BitmapScreen* frame_right;
	BitmapScreen* cursor1;
	BitmapScreen* cursor2;

	void RefreshBackground();
	void RefreshFrame();
	void RefreshCursor();

	bool background_needs_refresh;
	bool frame_needs_refresh;
	bool cursor_needs_refresh;

	int cursor_frame;
	int pause_frame;
	int animation_frames;
	double animation_count;
	double animation_increment;
};

#endif
