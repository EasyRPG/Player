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
#include <algorithm>
#include <sstream>
#include <vector>
#include "graphics.h"
#include "bitmap_screen.h"
#include "cache.h"
#include "baseui.h"
#include "drawable.h"
#include "time.h"
#include "util_macro.h"
#include "player.h"

////////////////////////////////////////////////////////////
namespace Graphics {
	bool fps_on_screen;
	uint32 drawable_id;

	void InternUpdate1(bool reset = false);
	void InternUpdate2(bool reset = false);
	void UpdateTitle();
	void DrawFrame();
	void DrawOverlay();

	bool overlay_visible;
	int fps;
	int framerate;
	int framecount;
	int fps_mode;
	uint32 timer_wait;

	void UpdateTransition();

	BitmapScreen* frozen_screen;
	BitmapScreen* black_screen;
	BitmapScreen* screen1;
	BitmapScreen* screen2;
	bool frozen;
	TransitionType transition_type;
	int transition_duration;
	int transition_frame;
	bool screen_erased;

	uint32 drawable_creation;

	struct State {
		State() : zlist_dirty(false) {}
		std::map<uint32, Drawable*> drawable_map;
		std::list<ZObj*> zlist;
		bool zlist_dirty;
	};
	State* state;
	std::vector<State*> stack;
	void Push();
	void Pop();

	bool SortZObj(const ZObj* first, const ZObj* second);
}

////////////////////////////////////////////////////////////
void Graphics::Init() {
	overlay_visible = true;
	fps_on_screen = false;
	fps = 0;
	framerate = DEFAULT_FPS;
	framecount = 0;
	fps_mode = 0;
	timer_wait = 0;
	frozen_screen = BitmapScreen::CreateBitmapScreen();

	black_screen = BitmapScreen::CreateBitmapScreen();
	Bitmap* black_bitmap = Bitmap::CreateBitmap(DisplayUi->GetWidth(), DisplayUi->GetHeight(), Color());
	black_screen->SetBitmap(black_bitmap, true);

	frozen = false;
	drawable_creation = 0;
	drawable_id = 0;
	state = new State();
	screen_erased = false;
}

////////////////////////////////////////////////////////////
void Graphics::Quit() {
	std::map<uint32, Drawable*>::iterator it;
	std::map<uint32, Drawable*> drawable_map_temp = state->drawable_map;

	for (it = drawable_map_temp.begin(); it != drawable_map_temp.end(); it++) {
		delete it->second;
	}

	state->drawable_map.clear();

	std::list<ZObj*>::iterator it_zlist;
	for (it_zlist = state->zlist.begin(); it_zlist != state->zlist.end(); it_zlist++) {
		delete *it_zlist;
	}

	state->zlist.clear();

	if (frozen_screen) {
		delete frozen_screen;
		frozen_screen = NULL;
	}

	if (black_screen) {
		delete black_screen;
		black_screen = NULL;
	}

	Cache::Clear();
}

////////////////////////////////////////////////////////////
void Graphics::Update() {
	if (frozen) return;

	switch(fps_mode) {
		case 1:
			InternUpdate2();
			return;
		default:
			InternUpdate1();
	}
}

////////////////////////////////////////////////////////////
void Graphics::InternUpdate1(bool reset) {
	// FIXME: This method needs more comments.
	static const double framerate_interval = 1000.0 / framerate;
	static uint32 current_time = 0;
	static double last_time = 0;
	static double wait_frames = 0.0;
	static double cycles_leftover = 0.0;
	static uint32 frames = 0;
	static uint32 next_fps_time = Time::Get() + 1000;

	if (reset) {
		last_time = Time::Get();
		next_fps_time = (uint32)last_time + 1000;
		frames = 0;
		return;
	}

	if (wait_frames >= 1) {
		wait_frames -= 1;
		return;
	}
	
	for (;;) {
		current_time = Time::Get();

		if ((current_time - last_time) >= framerate_interval) {
			cycles_leftover = wait_frames;
			wait_frames = ((double)current_time - last_time) / framerate_interval - cycles_leftover;
			last_time += current_time - last_time - cycles_leftover;

			DrawFrame();

			framecount++;
			frames++;

			if (current_time >= next_fps_time) {
				next_fps_time += 1000;
				fps = frames;
				frames = 0;

				UpdateTitle();
			}

			break;

		} else {
			Time::Sleep((uint32)(framerate_interval - (current_time - last_time)));
		}
	}
}

////////////////////////////////////////////////////////////
void Graphics::InternUpdate2(bool reset) {
	// FIXME: This method needs more comments. Why two InternUpdates?
	static const int MAXIMUM_FRAME_RATE = framerate;
	static const int MINIMUM_FRAME_RATE = max(framerate / 4, 1);
	static const int MAX_CYCLES_PER_FRAME = MAXIMUM_FRAME_RATE / MINIMUM_FRAME_RATE;
	static const double UPDATE_INTERVAL = 1.0 / MAXIMUM_FRAME_RATE;
	static double last_frame_time = 0.0;
	static double cycles_leftover = 0.0;
	static double current_time = 0.0;
	static double update_iterations = 0.0;
	static bool start = true;
	static int frames = 0;
	static uint32 next_fps_time = Time::Get() + 1000; 

	if (reset) {
		start = true;
		frames = 0;
		next_fps_time = Time::Get() + 1000;
		return;
	}

	for (;;) {
		if (start) {
			current_time = Time::Get() / 1000.0;
			update_iterations = (current_time - last_frame_time) + cycles_leftover;

			/*if (update_iterations > (MAX_CYCLES_PER_FRAME * UPDATE_INTERVAL)) {
				update_iterations = (MAX_CYCLES_PER_FRAME * UPDATE_INTERVAL);
			}*/
			start = false;
		}

		if (update_iterations > UPDATE_INTERVAL) {
			update_iterations -= UPDATE_INTERVAL;

			framecount++;

			return;
		}

		start = true;
		cycles_leftover = update_iterations;
		last_frame_time = current_time;

		DrawFrame();
		
		frames++;

		if (Time::Get() >= next_fps_time) {
			next_fps_time += 1000;
			fps = frames;
			frames = 0;

			UpdateTitle();
		}
	}
}

////////////////////////////////////////////////////////////
void Graphics::UpdateTitle() {
	if (DisplayUi->IsFullscreen()) return;

	std::stringstream title;
	title << GAME_TITLE;

	if (!fps_on_screen) {
		title << " - FPS " << fps;
	}

	DisplayUi->SetTitle(title.str());
}

////////////////////////////////////////////////////////////
void Graphics::DrawFrame() {
	if (transition_duration > 0) {
		UpdateTransition();
		return;
	}
	if (screen_erased) return;

	if (state->zlist_dirty) {
		state->zlist.sort(SortZObj);
		state->zlist_dirty = false;
	}

	DisplayUi->CleanDisplay();

	std::list<ZObj*>::iterator it_zlist;
	for (it_zlist = state->zlist.begin(); it_zlist != state->zlist.end(); it_zlist++) {
		state->drawable_map[(*it_zlist)->GetId()]->Draw((*it_zlist)->GetZ());
	}

	if (overlay_visible) {
		DrawOverlay();
	}

	DisplayUi->UpdateDisplay();
}

////////////////////////////////////////////////////////////
void Graphics::DrawOverlay() {
	if (Graphics::fps_on_screen) {
		std::stringstream text;
		text << "FPS: " << fps;
		DisplayUi->DrawScreenText(text.str());
	}
}

////////////////////////////////////////////////////////////
Bitmap* Graphics::SnapToBitmap() {
	DisplayUi->BeginScreenCapture();

	std::list<ZObj*>::iterator it_zlist;
	for (it_zlist = state->zlist.begin(); it_zlist != state->zlist.end(); it_zlist++) {
		state->drawable_map[(*it_zlist)->GetId()]->Draw((*it_zlist)->GetZ());
	}

	return DisplayUi->EndScreenCapture();
}

////////////////////////////////////////////////////////////
void Graphics::Freeze() {
	frozen_screen->SetBitmap(SnapToBitmap(), true);
	frozen = true;
}

////////////////////////////////////////////////////////////
void Graphics::Transition(TransitionType type, int duration, bool erase) {
	if (erase && screen_erased) return;

	if (type != TransitionNone) {
		transition_type = type;
		transition_frame = 0;
		transition_duration = type == TransitionErase ? 1 : duration;

		if (state->zlist_dirty) {
			state->zlist.sort(SortZObj);
			state->zlist_dirty = false;
		}

		if (!frozen) Freeze();

		if (erase) {
			screen1 = frozen_screen;

			screen2 = black_screen;
		} else {
			screen2 = frozen_screen;

			if (screen_erased)
				screen1 = black_screen;
			else 
				screen1 = screen2;
		}

		for (int i = 1; i <= transition_duration; i++) {
			Player::Update();
			InternUpdate1();
		}
	}

	if (!erase) frozen_screen->SetBitmap(NULL);

	frozen = false;
	screen_erased = erase;

	transition_duration = 0;

	FrameReset();
}

void Graphics::UpdateTransition() {
	// FIXME: Comments. Pleeeease. screen1, screen2?
	int w = DisplayUi->GetWidth();
	int h = DisplayUi->GetHeight();

	transition_frame++;

	int percentage = transition_frame * 100 / transition_duration;

	switch (transition_type) {
	case TransitionFadeIn:
		screen1->BlitScreen(0, 0);
		screen2->SetOpacityEffect(255 * percentage / 100);
		screen2->BlitScreen(0, 0);
		break;
	case TransitionFadeOut:
		screen1->BlitScreen(0, 0);
		screen2->SetOpacityEffect(255 * percentage / 100);
		screen2->BlitScreen(0, 0);
		break;
	case TransitionRandomBlocks:
		break;
	case TransitionRandomBlocksUp:
		break;
	case TransitionRandomBlocksDown:
		break;
	case TransitionBlindOpen:
		for (int i = 0; i < h / 8; i++) {
			screen1->BlitScreen(0, i * 8, Rect(0, i * 8, w, 8 - 8 * percentage / 100));
			screen2->BlitScreen(0, i * 8 + 8 - 8 * percentage / 100, Rect(0, i * 8 + 8 - 8 * percentage / 100, w, 8 * percentage / 100));
		}
		break;
	case TransitionBlindClose:
		for (int i = 0; i < h / 8; i++) {
			screen1->BlitScreen(0, i * 8 + 8 * percentage / 100, Rect(0, i * 8 + 8 * percentage / 100, w, 8 - 8 * percentage / 100));
			screen2->BlitScreen(0, i * 8, Rect(0, i * 8, w, 8 * percentage / 100));
		}
		break;
	case TransitionVerticalStripesIn:
	case TransitionVerticalStripesOut:
		for (int i = 0; i < h / 6 + 1 - h / 6 * percentage / 100; i++) {
			screen1->BlitScreen(0, i * 6 + 3, Rect(0, i * 6 + 3, w, 3));
			screen1->BlitScreen(0, h - i * 6, Rect(0, h - i * 6, w, 3));
		}
		for (int i = 0; i < h / 6 * percentage / 100; i++) {
			screen2->BlitScreen(0, i * 6, Rect(0, i * 6, w, 3));
			screen2->BlitScreen(0, h - 3 - i * 6, Rect(0, h - 3 - i * 6, w, 3));
		}
		break;
	case TransitionHorizontalStripesIn:
	case TransitionHorizontalStripesOut:
		for (int i = 0; i < w / 8 + 1 - w / 8 * percentage / 100; i++) {
			screen1->BlitScreen(i * 8 + 4, 0, Rect(i * 8 + 4, 0, 4, h));
			screen1->BlitScreen(w  - i * 8, 0, Rect(w - i * 8, 0, 4, h));
		}
		for (int i = 0; i < w / 8 * percentage / 100; i++) {
			screen2->BlitScreen(i * 8, 0, Rect(i * 8, 0, 4, h));
			screen2->BlitScreen(w - 4 - i * 8, 0, Rect(w - 4 - i * 8, 0, 4, h));
		}
		break;
	case TransitionBorderToCenterIn:
	case TransitionBorderToCenterOut:
		screen2->BlitScreen(0, 0);
		screen1->BlitScreen((w / 2) * percentage / 100, (h / 2) * percentage / 100, Rect((w / 2) * percentage / 100, (h / 2) * percentage / 100, w - w * percentage / 100, h - h * percentage / 100));
		break;
	case TransitionCenterToBorderIn:
	case TransitionCenterToBorderOut:
		screen1->BlitScreen(0, 0);
		screen2->BlitScreen(w / 2 - (w / 2) * percentage / 100, h / 2 - (h / 2) * percentage / 100, Rect(w / 2 - (w / 2) * percentage / 100, h / 2 - (h / 2) * percentage / 100, w * percentage / 100, h * percentage / 100));
		break;
	case TransitionScrollUpIn:
	case TransitionScrollUpOut:
		screen1->BlitScreen(0, -h * percentage / 100);
		screen2->BlitScreen(0, h - h * percentage / 100);
		break;
	case TransitionScrollDownIn:
	case TransitionScrollDownOut:
		screen1->BlitScreen(0, h * percentage / 100);
		screen2->BlitScreen(0, -h + h * percentage / 100);
		break;
	case TransitionScrollLeftIn:
	case TransitionScrollLeftOut:
		screen1->BlitScreen(-w * percentage / 100, 0);
		screen2->BlitScreen(w - w * percentage / 100, 0);
		break;
	case TransitionScrollRightIn:
	case TransitionScrollRightOut:
		screen1->BlitScreen(w * percentage / 100, 0);
		screen2->BlitScreen(-w + w * percentage / 100, 0);
		break;
	case TransitionVerticalCombine:
		screen1->BlitScreen(0, (h / 2) * percentage / 100, Rect(0, (h / 2) * percentage / 100, w, h - h * percentage / 100));
		screen2->BlitScreen(0, -h / 2 + (h / 2) * percentage / 100, Rect(0, 0, w, h / 2));
		screen2->BlitScreen(0, h - (h / 2) * percentage / 100, Rect(0, h / 2, w, h / 2));
		break;
	case TransitionVerticalDivision:
		screen1->BlitScreen(0, -(h / 2) * percentage / 100, Rect(0, 0, w, h / 2));
		screen1->BlitScreen(0, h / 2 + (h / 2) * percentage / 100, Rect(0, h / 2, w, h / 2));
		screen2->BlitScreen(0, h / 2 - (h / 2) * percentage / 100, Rect(0, h / 2 - (h / 2) * percentage / 100, w, h * percentage / 100));
		break;
	case TransitionHorizontalCombine:
		screen1->BlitScreen((w / 2) * percentage / 100, 0, Rect((w / 2) * percentage / 100, 0, w - w * percentage / 100, h));
		screen2->BlitScreen(- w / 2 + (w / 2) * percentage / 100, 0, Rect(0, 0, w / 2, h));
		screen2->BlitScreen(w - (w / 2) * percentage / 100, 0, Rect(w / 2, 0, w / 2, h));
		break;
	case TransitionHorizontalDivision:
		screen1->BlitScreen(-(w / 2) * percentage / 100, 0, Rect(0, 0, w / 2, h));
		screen1->BlitScreen(w / 2 + (w / 2) * percentage / 100, 0, Rect(w / 2, 0, w / 2, h));
		screen2->BlitScreen(w / 2 - (w / 2) * percentage / 100, 0, Rect(w / 2 - (w / 2) * percentage / 100, 0, w * percentage / 100, h));
		break;
	case TransitionCrossCombine:
		screen1->BlitScreen((w / 2) * percentage / 100, 0, Rect((w / 2) * percentage / 100, 0, w - w * percentage / 100, (h / 2) * percentage / 100));
		screen1->BlitScreen((w / 2) * percentage / 100, h - (h / 2) * percentage / 100, Rect((w / 2) * percentage / 100, h - (h / 2) * percentage / 100, w - w * percentage / 100, (h / 2) * percentage / 100));
		screen1->BlitScreen(0, (h / 2) * percentage / 100, Rect(0, (h / 2) * percentage / 100, w, h - h * percentage / 100));
		screen2->BlitScreen(- w / 2 + (w / 2) * percentage / 100, -h / 2 + (h / 2) * percentage / 100, Rect(0, 0, w / 2, h / 2));
		screen2->BlitScreen(w - (w / 2) * percentage / 100, -h / 2 + (h / 2) * percentage / 100, Rect(w / 2, 0, w / 2, h / 2));
		screen2->BlitScreen(w - (w / 2) * percentage / 100, h - (h / 2) * percentage / 100, Rect(w / 2, h / 2, w / 2, h / 2));
		screen2->BlitScreen(- w / 2 + (w / 2) * percentage / 100, h - (h / 2) * percentage / 100, Rect(0, h / 2, w / 2, h / 2));
		break;
	case TransitionCrossDivision:
		screen1->BlitScreen(-(w / 2) * percentage / 100, -(h / 2) * percentage / 100, Rect(0, 0, w / 2, h / 2));
		screen1->BlitScreen(w / 2 + (w / 2) * percentage / 100, -(h / 2) * percentage / 100, Rect(w / 2, 0, w / 2, h / 2));
		screen1->BlitScreen(w / 2 + (w / 2) * percentage / 100, h / 2 + (h / 2) * percentage / 100, Rect(w / 2, h / 2, w / 2, h / 2));
		screen1->BlitScreen(-(w / 2) * percentage / 100, h / 2 + (h / 2) * percentage / 100, Rect(0, h / 2, w / 2, h / 2));
		screen2->BlitScreen(w / 2 - (w / 2) * percentage / 100, 0, Rect(w / 2 - (w / 2) * percentage / 100, 0, w * percentage / 100, h / 2 - (h / 2) * percentage / 100));
		screen2->BlitScreen(w / 2 - (w / 2) * percentage / 100, h / 2 + (h / 2) * percentage / 100, Rect(w / 2 - (w / 2) * percentage / 100, h / 2 + (h / 2) * percentage / 100, w * percentage / 100, h / 2 + (h / 2) * percentage / 100));
		screen2->BlitScreen(0, h / 2 - (h / 2) * percentage / 100, Rect(0, h / 2 - (h / 2) * percentage / 100, w, h * percentage / 100));
		break;
	case TransitionZoomIn:
		break;
	case TransitionZoomOut:
		break;
	case TransitionMosaicIn:
		break;
	case TransitionMosaicOut:
		break;
	case TransitionWaveIn:
		break;
	case TransitionWaveOut:
		break;
	default:
		DisplayUi->CleanDisplay();
		break;
	}

	DisplayUi->UpdateDisplay();
}

////////////////////////////////////////////////////////////
void Graphics::FrameReset() {
	switch(fps_mode) {
	case 1:
		InternUpdate2(true);
		return;
	default:
		InternUpdate1(true);
	}
}

////////////////////////////////////////////////////////////
void Graphics::Wait(int duration) {
	while(duration-- > 0) {
		Update();
	}
}

////////////////////////////////////////////////////////////
int Graphics::GetFrameCount() {
	return framecount;
}
void Graphics::SetFrameCount(int nframecount) {
	framecount = nframecount;
}

///////////////////////////////////////////////////////////
void Graphics::RegisterDrawable(uint32 ID, Drawable* drawable) {
	state->drawable_map[ID] = drawable;
}

void Graphics::RemoveDrawable(uint32 ID) {
	std::map<uint32, Drawable*>::iterator it = state->drawable_map.find(ID);
	state->drawable_map.erase(it);
}

///////////////////////////////////////////////////////////
ZObj* Graphics::RegisterZObj(int z, uint32 ID) {
	ZObj* zobj = new ZObj(z, drawable_creation++, ID);
	state->zlist.push_back(zobj);

	state->zlist_dirty = true;

	return zobj;
}

void Graphics::RegisterZObj(int z, uint32 ID, bool multiz) {
	ZObj* zobj = new ZObj(z, 999999, ID);
	state->zlist.push_back(zobj);

	state->zlist_dirty = true;
}

///////////////////////////////////////////////////////////
void Graphics::RemoveZObj(uint32 ID) {
	RemoveZObj(ID, false);
}

void Graphics::RemoveZObj(uint32 ID, bool multiz) {
	std::vector<std::list<ZObj*>::iterator> to_erase;

	std::list<ZObj*>::iterator it_zlist;
	for (it_zlist = state->zlist.begin(); it_zlist != state->zlist.end(); it_zlist++) {
		if ((*it_zlist)->GetId() == ID) {
			delete *it_zlist;
			to_erase.push_back(it_zlist);
			if (!multiz) break;
		}
	}

	for (uint i = 0; i < to_erase.size(); i++) {
		state->zlist.erase(to_erase[i]);
	}
}

///////////////////////////////////////////////////////////
void Graphics::UpdateZObj(ZObj* zobj, int z) {
	zobj->SetZ(z);
	state->zlist_dirty = true;
}

///////////////////////////////////////////////////////////
inline bool Graphics::SortZObj(const ZObj* first, const ZObj* second) {
	if (first->GetZ() < second->GetZ()) return true;
	else if (first->GetZ() > second->GetZ()) return false;
	else return first->GetCreation() < second->GetCreation();
}

///////////////////////////////////////////////////////////
void Graphics::Push() {
	stack.push_back(state);
	state = new State();
}

///////////////////////////////////////////////////////////
void Graphics::Pop() {
	if (stack.size() > 0) {
		delete state;
		state = stack.back();
		stack.pop_back();
	}
}