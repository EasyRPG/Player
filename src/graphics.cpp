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
#include <memory>
#include <sstream>
#include <chrono>

#include "graphics.h"
#include "cache.h"
#include "player.h"
#include "fps_overlay.h"
#include "message_overlay.h"
#include "transition.h"
#include "scene.h"
#include "drawable_mgr.h"
#include "baseui.h"
#include "game_clock.h"
#include "game_screen.h"
#include "main_data.h"
#include "game_map.h"


using namespace std::chrono_literals;

namespace Graphics {
	void UpdateTitle();

	std::shared_ptr<Scene> current_scene;

	std::unique_ptr<MessageOverlay> message_overlay;
	std::unique_ptr<FpsOverlay> fps_overlay;

	std::string window_title_key;
}

void Graphics::Init() {
	Scene::Push(std::make_shared<Scene>());
	UpdateSceneCallback();

	message_overlay = std::make_unique<MessageOverlay>();
	fps_overlay = std::make_unique<FpsOverlay>();
}

void Graphics::Quit() {
	fps_overlay.reset();
	message_overlay.reset();

	Cache::ClearAll();

	Scene::PopUntil(Scene::Null);
	Scene::Pop();
}

void Graphics::Update() {
	fps_overlay->SetDrawFps(DisplayUi->RenderFps());

	//Update Graphics:
	if (fps_overlay->Update()) {
		UpdateTitle();
	}
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

	auto min_z = std::numeric_limits<Drawable::Z_t>::min();
	auto max_z = std::numeric_limits<Drawable::Z_t>::max();
	if (transition.IsActive()) {
		min_z = transition.GetZ();
	} else if (transition.IsErasedNotActive()) {
		min_z = transition.GetZ() + 1;
		dst.Clear();
	}
	LocalDraw(dst, min_z, max_z);
}

static Drawable::Z_t GetZForManiacLayer(int layer) {
	if (layer <= 0) return std::numeric_limits<Drawable::Z_t>::min();
	if (layer >= 10) return std::numeric_limits<Drawable::Z_t>::max();

	// Layer 9 (Windows) includes Message Text (Overlay)
	if (layer == 9) return Priority_Overlay;

	// For layers 1-8, the range ends just before the start of the *next* logical layer group.
	Drawable::Z_t next_layer_start = 0;
	switch (layer) {
	case 1: next_layer_start = Priority_TilesetBelow; break;    // Panorama
	case 2: next_layer_start = Priority_EventsBelow; break;     // Lower Chipset
	case 3: next_layer_start = Priority_Player; break;          // Events Below Hero
	case 4: next_layer_start = Priority_TilesetAbove; break;    // Hero / Events Same Level
	case 5: next_layer_start = Priority_EventsFlying; break;    // Upper Chipset
	case 6: next_layer_start = Priority_PictureNew; break;      // Events Above Hero
	case 7: next_layer_start = Priority_BattleAnimation; break; // Pictures (All priorities)
	case 8: next_layer_start = Priority_Window; break;          // Animations
	}

	return next_layer_start - 1;
}

void Graphics::LocalDraw(Bitmap& dst, Drawable::Z_t min_z, Drawable::Z_t max_z) {
	auto& drawable_list = DrawableMgr::GetLocalList();

	// Maniac Zoom Handling
	// Check if game_screen exists to prevent crash during init/cleanup
	// Also ensure we are in a Map or Battle scene
	if (Main_Data::game_screen && current_scene &&
		(current_scene->type == Scene::Map || current_scene->type == Scene::Battle))
	{
		int zoom_layer = Main_Data::game_screen->GetZoomLayer();

		if (zoom_layer > 0) {
			Drawable::Z_t threshold_z = GetZForManiacLayer(zoom_layer);

			// Only intervene if the zoom layer is within the current drawing range
			if (threshold_z >= min_z) {
				static BitmapRef zoom_buffer;
				// Ensure intermediate buffer exists and matches screen size
				if (!zoom_buffer || zoom_buffer->GetWidth() != dst.GetWidth() || zoom_buffer->GetHeight() != dst.GetHeight()) {
					zoom_buffer = Bitmap::Create(dst.GetWidth(), dst.GetHeight(), true);
				}

				// 1. Prepare Buffer
				if (min_z == std::numeric_limits<Drawable::Z_t>::min()) {
					// If rendering from the bottom, fill background to prevent trails.
					dst.Fill(Color(0, 0, 0, 255));

					// Draw the scene background (e.g. Panorama or color) into the buffer
					current_scene->DrawBackground(*zoom_buffer);
				}
				else {
					zoom_buffer->Clear();
				}

				// 2. Draw layers *affected* by zoom into the buffer
				// We clip max_z to threshold_z
				drawable_list.Draw(*zoom_buffer, min_z, std::min(max_z, threshold_z));

				// 3. Apply Zoom Transform
				double rate = Main_Data::game_screen->GetZoomRate();
				int cx = Main_Data::game_screen->GetZoomX();
				int cy = Main_Data::game_screen->GetZoomY();

				dst.ZoomOpacityBlit(cx, cy, cx, cy, *zoom_buffer, zoom_buffer->GetRect(), rate, rate, Opacity::Opaque());

				// 4. Continue rendering remaining layers normally (on top of the zoomed image)
				// Adjust min_z to start after the threshold
				if (max_z > threshold_z) {
					drawable_list.Draw(dst, threshold_z + 1, max_z);
				}

				return;
			}
		}
	}

	// Standard Rendering (No Zoom or Game_Screen not ready)
	if (!drawable_list.empty() && min_z == std::numeric_limits<Drawable::Z_t>::min()) {
		current_scene->DrawBackground(dst);
	}

	drawable_list.Draw(dst, min_z, max_z);
}

std::shared_ptr<Scene> Graphics::UpdateSceneCallback() {
	auto prev_scene = current_scene;
	current_scene = Scene::instance;

	if (current_scene) {
		if (prev_scene) {
			prev_scene->Suspend(current_scene->type);
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

