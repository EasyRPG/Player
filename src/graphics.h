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

#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <list>
#include "SDL.h"
#include "bitmap.h"
#include "drawable.h"

////////////////////////////////////////////////////////////
/// Graphics namespace
////////////////////////////////////////////////////////////
namespace Graphics {
    void Init();
    void TimerWait();
    void TimerContinue();
    void Update();
    void DrawFrame();
    void Freeze();
    void Transition(int type, int time);
    void FrameReset();
    void Wait(int duration);
    Bitmap* SnapToBitmap();
    int GetFrameCount();
    void SetFrameCount(int nframecount);

    bool SortDrawable(Drawable* &first, Drawable* &second);
    void RemoveDrawable(unsigned long ID);

    extern int fps;
    extern int framerate;
    extern int framecount;
    extern double framerate_interval;
    extern unsigned long ID;
    extern unsigned long last_ticks;
    extern unsigned long last_ticks_wait;
    extern unsigned long next_ticks_fps;

    extern std::list<Drawable*> drawable_list;
    extern std::list<Drawable*>::iterator it_drawable_list;
}

#endif
