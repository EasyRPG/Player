//////////////////////////////////////////////////////////////////////////////////
/// This file is part of EasyRPG Player.
/// 
/// EasyRPG Player is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
/// 
/// EasyRPG Player is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
/// 
/// You should have received a copy of the GNU General Public License
/// along with EasyRPG Player.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////////////////

#ifndef _WINDOW_H_
#define _WINDOW_H_

////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include "drawable.h"
#include "bitmap.h"

////////////////////////////////////////////////////////////
/// Window class
////////////////////////////////////////////////////////////
class Window : public Drawable{
public:
    Window();
    ~Window();

    void Draw();

    void Update();
    Bitmap* GetWindowskin();
    void SetWindowskin(Bitmap* nwindowskin);
    Bitmap* GetContents();
    void SetContents(Bitmap* ncontents);
    bool GetStretch();
    void SetStretch(bool nstretch);
    Rect GetCursorRect();
    void SetCursorRect(Rect ncursor_rect);
    bool GetActive();
    void SetActive(bool nactive);
    bool GetVisible();
    void SetVisible(bool nvisible);
    bool GetPause();
    void SetPause(bool npause);
    int GetX();
    void SetX(int nx);
    int GetY();
    void SetY(int ny);
    int GetWidth();
    void SetWidth(int nwidth);
    int GetHeight();
    void SetHeight(int nheight);
    int GetZ();
    void SetZ(int nz);
    int GetOx();
    void SetOx(int nox);
    int GetOy();
    void SetOy(int noy);
    int GetOpacity();
    void SetOpacity(int nopacity);
    int GetBackOpacity();
    void SetBackOpacity(int nback_opacity);
    int GetContentsOpacity();
    void SetContentsOpacity(int ncontents_opacity);

    unsigned long GetId();

protected:
    unsigned long _id;
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
    Bitmap* background;
    Bitmap* frame;
    Bitmap* cursor1;
    Bitmap* cursor2;

    void RefreshBackground();
    void RefreshFrame();
    void RefreshCursor();
    bool background_needs_refresh;
    bool frame_needs_refresh;
    bool cursor_needs_refresh;
    int cursor_frame;
    int pause_frame;
};

#endif
