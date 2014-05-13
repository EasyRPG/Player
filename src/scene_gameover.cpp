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

Scene_Gameover::Scene_Gameover() {
	type = Scene::Gameover;
}

void Scene_Gameover::Start() {
	// Load Background Graphic
	background.reset(new Sprite());
	background->SetBitmap(Cache::Gameover(Data::system.gameover_name));
	// Play gameover music
	Game_System::BgmPlay(Game_System::GetSystemBGM(Game_System::BGM_GameOver));
}

void Scene_Gameover::Update() {
	if (Input::IsTriggered(Input::DECISION)) {
		Scene::PopUntil(Scene::Title);
	}
}
