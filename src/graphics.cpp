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
#include <algorithm>
#include <sstream>
#include <vector>
#include <list>

#include "graphics.h"
#include "bitmap.h"
#include "cache.h"
#include "baseui.h"
#include "drawable.h"
#include "input.h"
#include "util_macro.h"
#include "output.h"
#include "player.h"
#include "fps_overlay.h"
#include "message_overlay.h"

namespace Graphics {
	void UpdateTitle();
	void DrawFrame();

	int framerate;

	void UpdateTransition();

	BitmapRef frozen_screen;
	BitmapRef black_screen;
	BitmapRef screen1;
	BitmapRef screen2;
	TransitionType transition_type;
	int transition_duration;
	int transition_frames_left;
	int transition_frame;
	bool screen_erased;

	uint32_t next_fps_time;

	struct State {
		State() {}
		std::list<Drawable*> drawable_list;
		bool zlist_dirty = false;
		bool draw_background = true;
	};

	std::shared_ptr<State> state;
	std::vector<std::shared_ptr<State> > stack;
	std::shared_ptr<State> global_state;

	bool SortDrawableList(const Drawable* first, const Drawable* second);

	std::unique_ptr<MessageOverlay> message_overlay;
	std::unique_ptr<FpsOverlay> fps_overlay;
}

unsigned SecondToFrame(float const second) {
	return(second * Graphics::framerate);
}

void Graphics::Init() {
	frozen_screen = BitmapRef();
	screen_erased = false;
	transition_frames_left = 0;

	state.reset(new State());
	global_state.reset(new State());

	// Is a drawable, must be init after state
	message_overlay.reset(new MessageOverlay());
	fps_overlay.reset(new FpsOverlay());

	next_fps_time = 0;
}

void Graphics::Quit() {
	state->drawable_list.clear();
	global_state->drawable_list.clear();

	frozen_screen.reset();
	black_screen.reset();

	fps_overlay.reset();
	message_overlay.reset();

	Cache::Clear();
}

void Graphics::Update(bool time_left) {
	if (next_fps_time == 0) {
		next_fps_time = DisplayUi->GetTicks() + 1000;
	}

	// Calculate fps
	uint32_t current_time = DisplayUi->GetTicks();
	if (current_time >= next_fps_time) {
		// 1 sec over
		next_fps_time += 1000;

		if (fps_overlay->GetFps() == 0) {
			Output::Debug("Framerate is 0 FPS!");
			DrawFrame();
		}

		next_fps_time = current_time + 1000;

		fps_overlay->ResetCounter();

		UpdateTitle();
	}

	// Render next frame
	if (time_left) {
		fps_overlay->AddFrame();

		DrawFrame();
	}

	fps_overlay->Update();
	fps_overlay->AddUpdate();

	message_overlay->Update();
}

void Graphics::UpdateTitle() {
	if (DisplayUi->IsFullscreen()) return;
#ifdef EMSCRIPTEN
	return;
#endif

	std::stringstream title;
	if (!Player::game_title.empty()) {
		title << Player::game_title << " - ";
	}
	title << GAME_TITLE;

	if (Player::fps_flag) {
		title << " - " << fps_overlay->GetFpsString();
	}

	DisplayUi->SetTitle(title.str());
}

void Graphics::DrawFrame() {
	if (transition_frames_left > 0) {
		UpdateTransition();

		for (Drawable* drawable : global_state->drawable_list) {
			drawable->Draw();
		}

		DisplayUi->UpdateDisplay();
		return;
	}

	if (screen_erased) {
		DisplayUi->CleanDisplay();
		return;
	}

	if (state->zlist_dirty) {
		state->drawable_list.sort(SortDrawableList);
		state->zlist_dirty = false;
	}

	if (global_state->zlist_dirty) {
		global_state->drawable_list.sort(SortDrawableList);
		global_state->zlist_dirty = false;
	}

	if (state->draw_background) {
		DisplayUi->AddBackground();
	}

	for (Drawable* drawable : state->drawable_list) {
		drawable->Draw();
	}

	for (Drawable* drawable : global_state->drawable_list) {
		drawable->Draw();
	}

	DisplayUi->UpdateDisplay();
}

BitmapRef Graphics::SnapToBitmap() {
	if (state->draw_background) {
		DisplayUi->AddBackground();
	}

	for (Drawable* drawable : state->drawable_list) {
		drawable->Draw();
	}

	for (Drawable* drawable : global_state->drawable_list) {
		drawable->Draw();
	}

	return DisplayUi->CaptureScreen();
}

void Graphics::Freeze() {
	frozen_screen = SnapToBitmap();
}

void Graphics::Transition(TransitionType type, int duration, bool erase) {
	if (!black_screen) {
		black_screen = Bitmap::Create(DisplayUi->GetWidth(), DisplayUi->GetHeight(), Color(0, 0, 0, 255));
	}

	if (screen_erased && erase) {
		// Don't allow another erase when already erased
		return;
	}

	if (type != TransitionNone) {
		transition_type = type;
		transition_frame = 0;
		transition_duration = type == TransitionErase ? 1 : duration;
		transition_frames_left = transition_duration;

		if (state->zlist_dirty) {
			state->drawable_list.sort(SortDrawableList);
			state->zlist_dirty = false;
		}

		if (global_state->zlist_dirty) {
			global_state->drawable_list.sort(SortDrawableList);
			global_state->zlist_dirty = false;
		}

		Freeze();

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
	}

	screen_erased = erase;
}

bool Graphics::IsTransitionPending() {
	return transition_frames_left > 0;
}

void Graphics::UpdateTransition() {
	// FIXME: Comments. Pleeeease. screen1, screen2?
	BitmapRef dst = DisplayUi->GetDisplaySurface();
	int w = DisplayUi->GetWidth();
	int h = DisplayUi->GetHeight();

	transition_frame++;

	int percentage = transition_frame * 100 / transition_duration;

	transition_frames_left--;

	// Fallback to FadeIn/Out for not implemented transition types:
	// (Remove from here when implemented below)
	switch (transition_type) {
	case TransitionRandomBlocks:
	case TransitionRandomBlocksUp:
	case TransitionRandomBlocksDown:
	case TransitionZoomIn:
	case TransitionZoomOut:
	case TransitionMosaicIn:
	case TransitionMosaicOut:
	case TransitionWaveIn:
	case TransitionWaveOut:
		transition_type = TransitionFadeIn;
		break;
	default:
		break;
	}

	switch (transition_type) {
	case TransitionFadeIn:
	case TransitionFadeOut:
		dst->Blit(0, 0, *screen1, screen1->GetRect(), 255);
		dst->Blit(0, 0, *screen2, screen2->GetRect(), 255 * percentage / 100);
		break;
	case TransitionRandomBlocks:
		break;
	case TransitionRandomBlocksUp:
		break;
	case TransitionRandomBlocksDown:
		break;
	case TransitionBlindOpen:
		for (int i = 0; i < h / 8; i++) {
			dst->Blit(0, i * 8, *screen1, Rect(0, i * 8, w, 8 - 8 * percentage / 100), 255);
			dst->Blit(0, i * 8 + 8 - 8 * percentage / 100, *screen2, Rect(0, i * 8 + 8 - 8 * percentage / 100, w, 8 * percentage / 100), 255 * percentage / 100);
		}
		break;
	case TransitionBlindClose:
		for (int i = 0; i < h / 8; i++) {
			dst->Blit(0, i * 8 + 8 * percentage / 100, *screen1, Rect(0, i * 8 + 8 * percentage / 100, w, 8 - 8 * percentage / 100), 255);
			dst->Blit(0, i * 8, *screen2, Rect(0, i * 8, w, 8 * percentage / 100), 255);
		}
		break;
	case TransitionVerticalStripesIn:
	case TransitionVerticalStripesOut:
		for (int i = 0; i < h / 6 + 1 - h / 6 * percentage / 100; i++) {
			dst->Blit(0, i * 6 + 3, *screen1, Rect(0, i * 6 + 3, w, 3), 255);
			dst->Blit(0, h - i * 6, *screen1, Rect(0, h - i * 6, w, 3), 255);
		}
		for (int i = 0; i < h / 6 * percentage / 100; i++) {
			dst->Blit(0, i * 6, *screen2, Rect(0, i * 6, w, 3), 255);
			dst->Blit(0, h - 3 - i * 6, *screen2, Rect(0, h - 3 - i * 6, w, 3), 255);
		}
		break;
	case TransitionHorizontalStripesIn:
	case TransitionHorizontalStripesOut:
		for (int i = 0; i < w / 8 + 1 - w / 8 * percentage / 100; i++) {
			dst->Blit(i * 8 + 4, 0, *screen1, Rect(i * 8 + 4, 0, 4, h), 255);
			dst->Blit(w - i * 8, 0, *screen1, Rect(w - i * 8, 0, 4, h), 255);
		}
		for (int i = 0; i < w / 8 * percentage / 100; i++) {
			dst->Blit(i * 8, 0, *screen2, Rect(i * 8, 0, 4, h), 255);
			dst->Blit(w - 4 - i * 8, 0, *screen2, Rect(w - 4 - i * 8, 0, 4, h), 255);
		}
		break;
	case TransitionBorderToCenterIn:
	case TransitionBorderToCenterOut:
		dst->Blit(0,  0, *screen2, screen2->GetRect(), 255);
		dst->Blit((w / 2) * percentage / 100, (h / 2) * percentage / 100, *screen1, Rect((w / 2) * percentage / 100, (h / 2) * percentage / 100, w - w * percentage / 100, h - h * percentage / 100), 255);
		break;
	case TransitionCenterToBorderIn:
	case TransitionCenterToBorderOut:
		dst->Blit(0,  0, *screen1, screen1->GetRect(), 255);
		dst->Blit(w / 2 - (w / 2) * percentage / 100, h / 2 - (h / 2) * percentage / 100, *screen2, Rect(w / 2 - (w / 2) * percentage / 100, h / 2 - (h / 2) * percentage / 100, w * percentage / 100, h * percentage / 100), 255);
		break;
	case TransitionScrollUpIn:
	case TransitionScrollUpOut:
		dst->Blit(0,  -h * percentage / 100, *screen1, screen1->GetRect(), 255);
		dst->Blit(0,  h - h * percentage / 100, *screen2, screen2->GetRect(), 255);
		break;
	case TransitionScrollDownIn:
	case TransitionScrollDownOut:
		dst->Blit(0,  h * percentage / 100, *screen1, screen1->GetRect(), 255);
		dst->Blit(0,  -h + h * percentage / 100, *screen2, screen2->GetRect(), 255);
		break;
	case TransitionScrollLeftIn:
	case TransitionScrollLeftOut:
		dst->Blit(-w * percentage / 100,  0, *screen1, screen1->GetRect(), 255);
		dst->Blit(w - w * percentage / 100,  0, *screen2, screen2->GetRect(), 255);
		break;
	case TransitionScrollRightIn:
	case TransitionScrollRightOut:
		dst->Blit(w * percentage / 100,  0, *screen1, screen1->GetRect(), 255);
		dst->Blit(-w + w * percentage / 100,  0, *screen2, screen2->GetRect(), 255);
		break;
	case TransitionVerticalCombine:
		dst->Blit(0, (h / 2) * percentage / 100, *screen1, Rect(0, (h / 2) * percentage / 100, w, h - h * percentage / 100), 255);
		dst->Blit(0, -h / 2 + (h / 2) * percentage / 100, *screen2, Rect(0, 0, w, h / 2), 255);
		dst->Blit(0, h - (h / 2) * percentage / 100, *screen2, Rect(0, h / 2, w, h / 2), 255);
		break;
	case TransitionVerticalDivision:
		dst->Blit(0, -(h / 2) * percentage / 100, *screen1, Rect(0, 0, w, h / 2), 255);
		dst->Blit(0, h / 2 + (h / 2) * percentage / 100, *screen1, Rect(0, h / 2, w, h / 2), 255);
		dst->Blit(0, h / 2 - (h / 2) * percentage / 100, *screen2, Rect(0, h / 2 - (h / 2) * percentage / 100, w, h * percentage / 100), 255);
		break;
	case TransitionHorizontalCombine:
		dst->Blit((w / 2) * percentage / 100, 0, *screen1, Rect((w / 2) * percentage / 100, 0, w - w * percentage / 100, h), 255);
		dst->Blit(- w / 2 + (w / 2) * percentage / 100, 0, *screen2, Rect(0, 0, w / 2, h), 255);
		dst->Blit(w - (w / 2) * percentage / 100, 0, *screen2, Rect(w / 2, 0, w / 2, h), 255);
		break;
	case TransitionHorizontalDivision:
		dst->Blit(-(w / 2) * percentage / 100, 0, *screen1, Rect(0, 0, w / 2, h), 255);
		dst->Blit(w / 2 + (w / 2) * percentage / 100, 0, *screen1, Rect(w / 2, 0, w / 2, h), 255);
		dst->Blit(w / 2 - (w / 2) * percentage / 100, 0, *screen2, Rect(w / 2 - (w / 2) * percentage / 100, 0, w * percentage / 100, h), 255);
		break;
	case TransitionCrossCombine:
		dst->Blit((w / 2) * percentage / 100, 0, *screen1, Rect((w / 2) * percentage / 100, 0, w - w * percentage / 100, (h / 2) * percentage / 100), 255);
		dst->Blit((w / 2) * percentage / 100, h - (h / 2) * percentage / 100, *screen1, Rect((w / 2) * percentage / 100, h - (h / 2) * percentage / 100, w - w * percentage / 100, (h / 2) * percentage / 100), 255);
		dst->Blit(0, (h / 2) * percentage / 100, *screen1, Rect(0, (h / 2) * percentage / 100, w, h - h * percentage / 100), 255);
		dst->Blit(- w / 2 + (w / 2) * percentage / 100, -h / 2 + (h / 2) * percentage / 100, *screen2, Rect(0, 0, w / 2, h / 2), 255);
		dst->Blit(w - (w / 2) * percentage / 100, -h / 2 + (h / 2) * percentage / 100, *screen2, Rect(w / 2, 0, w / 2, h / 2), 255);
		dst->Blit(w - (w / 2) * percentage / 100, h - (h / 2) * percentage / 100, *screen2, Rect(w / 2, h / 2, w / 2, h / 2), 255);
		dst->Blit(- w / 2 + (w / 2) * percentage / 100, h - (h / 2) * percentage / 100, *screen2, Rect(0, h / 2, w / 2, h / 2), 255);
		break;
	case TransitionCrossDivision:
		dst->Blit(-(w / 2) * percentage / 100, -(h / 2) * percentage / 100, *screen1, Rect(0, 0, w / 2, h / 2), 255);
		dst->Blit(w / 2 + (w / 2) * percentage / 100, -(h / 2) * percentage / 100, *screen1, Rect(w / 2, 0, w / 2, h / 2), 255);
		dst->Blit(w / 2 + (w / 2) * percentage / 100, h / 2 + (h / 2) * percentage / 100, *screen1, Rect(w / 2, h / 2, w / 2, h / 2), 255);
		dst->Blit(-(w / 2) * percentage / 100, h / 2 + (h / 2) * percentage / 100, *screen1, Rect(0, h / 2, w / 2, h / 2), 255);
		dst->Blit(w / 2 - (w / 2) * percentage / 100, 0, *screen2, Rect(w / 2 - (w / 2) * percentage / 100, 0, w * percentage / 100, h / 2 - (h / 2) * percentage / 100), 255);
		dst->Blit(w / 2 - (w / 2) * percentage / 100, h / 2 + (h / 2) * percentage / 100, *screen2, Rect(w / 2 - (w / 2) * percentage / 100, h / 2 + (h / 2) * percentage / 100, w * percentage / 100, h / 2 + (h / 2) * percentage / 100), 255);
		dst->Blit(0, h / 2 - (h / 2) * percentage / 100, *screen2, Rect(0, h / 2 - (h / 2) * percentage / 100, w, h * percentage / 100), 255);
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
}

void Graphics::FrameReset() {
	next_fps_time = (uint32_t)DisplayUi->GetTicks() + 1000;
	fps_overlay->ResetCounter();
}

void Graphics::RegisterDrawable(Drawable* drawable) {
	if (drawable->IsGlobal()) {
		global_state->drawable_list.push_back(drawable);
	} else {
		state->drawable_list.push_back(drawable);
	}
	UpdateZCallback();
}

void Graphics::RemoveDrawable(Drawable* drawable) {
	std::list<Drawable*>::iterator it;
	if (drawable->IsGlobal()) {
		it = std::find(global_state->drawable_list.begin(), global_state->drawable_list.end(), drawable);
		if (it != global_state->drawable_list.end()) { global_state->drawable_list.erase(it); }
	} else {
		it = std::find(state->drawable_list.begin(), state->drawable_list.end(), drawable);
		if (it != state->drawable_list.end()) { state->drawable_list.erase(it); }
	}
}

void Graphics::UpdateZCallback() {
	state->zlist_dirty = true;
	global_state->zlist_dirty = true;
}

inline bool Graphics::SortDrawableList(const Drawable* first, const Drawable* second) {
	if (first->GetZ() < second->GetZ()) return true;
	return false;
}

void Graphics::Push(bool draw_background) {
	stack.push_back(state);
	state.reset(new State());
	state->draw_background = draw_background;
}

void Graphics::Pop() {
	if (stack.size() > 0) {
		state = stack.back();
		stack.pop_back();
	}
}

int Graphics::GetDefaultFps() {
	return DEFAULT_FPS;
}

MessageOverlay& Graphics::GetMessageOverlay() {
	return *message_overlay;
}
