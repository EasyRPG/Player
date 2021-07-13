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
#include <chrono>
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
#include "baseui.h"
#include "game_clock.h"

using namespace std::chrono_literals;

namespace Graphics {
	void UpdateTitle();

	int framerate;

	std::shared_ptr<Scene> current_scene;

	std::unique_ptr<MessageOverlay> message_overlay;
	std::unique_ptr<FpsOverlay> fps_overlay;

	std::string window_title_key;
}

unsigned SecondToFrame(float const second) {
	return(second * Graphics::framerate);
}

void Graphics::Init() {
	Scene::Push(std::make_shared<Scene>());
	UpdateSceneCallback();

	message_overlay.reset(new MessageOverlay());
	fps_overlay.reset(new FpsOverlay());
}

void Graphics::Quit() {
	fps_overlay.reset();
	message_overlay.reset();

	Cache::Clear();
}

void Graphics::Update() {
	BitmapRef disp = DisplayUi->GetDisplaySurface();
	fps_overlay->SetDrawFps(DisplayUi->RenderFps());

	//Update Graphics:
	if (fps_overlay->Update()) {
		UpdateTitle();
	}
	message_overlay->Update();
}

void Graphics::UpdateTitle() {
	if (DisplayUi->IsFullscreen()) {
		return;
	}

#ifdef EMSCRIPTEN
	return;
#else
	std::string fps;
	if (DisplayUi->ShowFpsOnTitle()) {
		fps += fps_overlay->GetFpsString();
	}

	if (window_title_key == (Player::game_title + fps)) {
		return;
	}

	std::stringstream title;
	if (!Player::game_title.empty()) {
		title << Player::game_title << " - ";
	}
	title << GAME_TITLE;

	if (DisplayUi->ShowFpsOnTitle()) {
		title << " - " << fps;
	}

	DisplayUi->SetTitle(title.str());

	window_title_key = (Player::game_title + fps);
#endif
}

void Graphics::Draw(Bitmap& dst) {
	auto& transition = Transition::instance();

	int min_z = std::numeric_limits<int>::min();
	int max_z = std::numeric_limits<int>::max();
	if (transition.IsActive()) {
		min_z = transition.GetZ();
	} else if (transition.IsErasedNotActive()) {
		min_z = transition.GetZ() + 1;
		dst.Clear();
	}
	LocalDraw(dst, min_z, max_z);
}

void Graphics::LocalDraw(Bitmap& dst, int min_z, int max_z) {
	auto& drawable_list = DrawableMgr::GetLocalList();

	if (!drawable_list.empty() && min_z == std::numeric_limits<int>::min()) {
		current_scene->DrawBackground(dst);
	}

	drawable_list.Draw(dst, min_z, max_z);
}

std::shared_ptr<Scene> Graphics::UpdateSceneCallback() {
	auto prev_scene = current_scene;
	current_scene = Scene::instance;

	if (current_scene) {
		if (prev_scene) {
			current_scene->TransferDrawablesFrom(*prev_scene);
		}
		DrawableMgr::SetLocalList(&current_scene->GetDrawableList());
	} else {
		DrawableMgr::SetLocalList(nullptr);
	}

	return prev_scene;
}

MessageOverlay& Graphics::GetMessageOverlay() {
	return *message_overlay;
}

