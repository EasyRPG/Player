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
#include "scene_gameover.h"
#include "bitmap.h"
#include "cache.h"
#include "game_system.h"
#include "input.h"
#include "main_data.h"
#include "transition.h"
#include "player.h"

Scene_Gameover::Scene_Gameover() {
	type = Scene::Gameover;
}

void Scene_Gameover::Start() {
	if (!lcf::Data::system.gameover_name.empty()) {
		FileRequestAsync* request = AsyncHandler::RequestFile("GameOver", lcf::Data::system.gameover_name);
		request->SetGraphicFile(true);
		request_id = request->Bind(&Scene_Gameover::OnBackgroundReady, this);
		request->Start();
	}
	// Play gameover music
	Main_Data::game_system->BgmPlay(Main_Data::game_system->GetSystemBGM(Main_Data::game_system->BGM_GameOver));
}

void Scene_Gameover::vUpdate() {
	if (Input::IsTriggered(Input::DECISION)) {
		Scene::ReturnToTitleScene();
	}
}

void Scene_Gameover::OnBackgroundReady(FileRequestResult* result) {
	// Load Background Graphic
	background.reset(new Sprite());

	BitmapRef bitmapRef = Cache::Gameover(result->file);
	background->SetBitmap(bitmapRef);

	// If the sprite doesn't fill the screen, center it to support custom resolutions
	if (bitmapRef->GetWidth() < Player::screen_width) {
		background->SetX(Player::menu_offset_x);
	}
	if (bitmapRef->GetHeight() < Player::screen_height) {
		background->SetY(Player::menu_offset_y);
	}
}

void Scene_Gameover::TransitionIn(SceneType /* prev_scene */) {
	Transition::instance().InitShow(Transition::TransitionFadeIn, this, 80);
}

void Scene_Gameover::TransitionOut(SceneType /* next_scene */) {
	Transition::instance().InitErase(Transition::TransitionFadeOut, this, 80);
}
