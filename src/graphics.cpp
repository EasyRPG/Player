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
#include <sstream>
#include <array>

#include "graphics.h"
#include "cache.h"
#include "output.h"
#include "player.h"
#include "fps_overlay.h"
#include "message_overlay.h"
#include "transition.h"
#include "scene.h"
#include "drawable.h"
#include "drawable_mgr.h"

namespace Graphics {
	void UpdateTitle();
	void LocalDraw(Bitmap& dst, int priority = Priority::Priority_Maximum);
	void GlobalDraw(Bitmap& dst, int priority = Priority::Priority_Maximum);

	int framerate;

	uint32_t next_fps_time;

	std::shared_ptr<Scene> current_scene;

	std::unique_ptr<Transition> transition;
	std::unique_ptr<MessageOverlay> message_overlay;
	std::unique_ptr<FpsOverlay> fps_overlay;
}

unsigned SecondToFrame(float const second) {
	return(second * Graphics::framerate);
}

void Graphics::Init() {
	Scene::Push(std::make_shared<Scene>());
	current_scene = Scene::instance;

	// Is a drawable, must be init after state
	transition.reset(new Transition());
	message_overlay.reset(new MessageOverlay());
	fps_overlay.reset(new FpsOverlay());

	next_fps_time = 0;
}

void Graphics::Quit() {
	DrawableMgr::GetGlobalList().Clear();

	transition.reset();
	fps_overlay.reset();
	message_overlay.reset();

	Cache::Clear();
}

void Graphics::Update() {
	//FPS:
	if (next_fps_time == 0) {
		next_fps_time = DisplayUi->GetTicks() + 1000;
	}

	BitmapRef disp = DisplayUi->GetDisplaySurface();

	uint32_t current_time = DisplayUi->GetTicks();
	if (current_time >= next_fps_time) {
		next_fps_time += 1000;

		if (fps_overlay->GetFps() == 0) {
			Output::Debug("Framerate is 0 FPS!");
			Draw(*disp);
			Player::FrameReset(current_time);
		} else {
			next_fps_time = current_time + 1000;
			fps_overlay->ResetCounter();
		}

		UpdateTitle();
	}

	//Update Graphics:
	fps_overlay->Update();
	fps_overlay->AddUpdate();
	message_overlay->Update();
	transition->Update();
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

void Graphics::Draw(Bitmap& dst) {
	fps_overlay->AddFrame();

	BitmapRef disp = DisplayUi->GetDisplaySurface();

	if (transition->IsErased()) {
		DisplayUi->CleanDisplay();
		GlobalDraw(dst);
		DisplayUi->UpdateDisplay();
		return;
	}
	LocalDraw(dst);
	GlobalDraw(dst);
	DisplayUi->UpdateDisplay();
}

void Graphics::LocalDraw(Bitmap& dst, int priority) {
	auto& drawable_list = DrawableMgr::GetLocalList();

	if (!drawable_list.empty())
		current_scene->DrawBackground();

	drawable_list.Draw(dst, priority);
}

void Graphics::GlobalDraw(Bitmap& dst, int priority) {
	auto& drawable_list = DrawableMgr::GetGlobalList();

	drawable_list.Draw(dst, priority);
}


BitmapRef Graphics::SnapToBitmap(int priority) {
	BitmapRef disp = DisplayUi->GetDisplaySurface();
	LocalDraw(*disp, priority);
	GlobalDraw(*disp, priority);
	return DisplayUi->CaptureScreen();
}

bool Graphics::IsTransitionPending() {
	return (transition ? transition->IsActive() : false);
}

bool Graphics::IsTransitionErased() {
	return (transition ? transition->IsErased() : false);
}

void Graphics::FrameReset(uint32_t start_ticks) {
	next_fps_time = start_ticks + 1000;
	fps_overlay->ResetCounter();
}

void Graphics::UpdateSceneCallback() {
	current_scene = Scene::instance;
	if (current_scene) {
		DrawableMgr::SetLocalList(&current_scene->GetDrawableList());
	} else {
		DrawableMgr::SetLocalList(nullptr);
	}
}

int Graphics::GetDefaultFps() {
	return DEFAULT_FPS;
}

MessageOverlay& Graphics::GetMessageOverlay() {
	return *message_overlay;
}

Transition& Graphics::GetTransition() {
	return *transition;
}
