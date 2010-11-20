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

#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <list>
#include "SDL.h"
#include "bitmap.h"
#include "drawable.h"
#include "zobj.h"

////////////////////////////////////////////////////////////
/// Graphics namespace
////////////////////////////////////////////////////////////
namespace Graphics {
	void Init();
	void Quit();
	void TimerWait();
	void TimerContinue();
	void Update();
	void DrawFrame();
	void Freeze();
	void FrameReset();
	void Wait(int duration);
	Bitmap* SnapToBitmap();
	int GetFrameCount();
	void SetFrameCount(int nframecount);
	void SetDefaultBackcolor(const SDL_Color& color);

	void RegisterDrawable(unsigned long ID, Drawable* drawable);
	void RemoveDrawable(unsigned long ID);

	bool SortZObj(ZObj &first, ZObj &second);
	void RegisterZObj(long z, unsigned long ID);
	void RegisterZObj(long z, unsigned long ID, bool multiz);
	void RemoveZObj(unsigned long ID);
	void UpdateZObj(unsigned long ID, long z);

	//////////////////////////////////
	/// Screen Transition Variables
	//////////////////////////////////
	enum TransitionType {
		FadeIn,
		FadeOut,
		NoTransition
	};

	void Transition(TransitionType type, int time, bool wait);
	void DoTransition();

	extern TransitionType actual_transition;

	extern SDL_Surface* fake_background;

	extern int transition_frames;
	extern int transition_current_frame;
	extern int transition_increment;

	extern bool is_in_transition_yet;

	//////////////////////////////////
	//////////////////////////////////

	extern int fps;
	extern int framerate;
	extern int framecount;
	extern double framerate_interval;
	extern unsigned long creation;
	extern unsigned long ID;
	extern unsigned long last_ticks;
	extern unsigned long last_ticks_wait;
	extern unsigned long next_ticks_fps;

	extern Uint32 default_backcolor;

	extern std::map<unsigned long, Drawable*> drawable_map;
	extern std::map<unsigned long, Drawable*>::iterator it_drawable_map;

	extern std::list<ZObj> zlist;
	extern std::list<ZObj>::iterator it_zlist;

}

#endif
