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

#include "scene_debug_picture.h"
#include "input.h"
#include "player.h"
#include "game_system.h"
#include "main_data.h"

Scene_DebugPicture::Scene_DebugPicture() {
	type = Scene::DebugPicture;
}

void Scene_DebugPicture::Start() {
	// Make list window narrow (just IDs) to maximize info space
	int list_w = 64;

	list_window = std::make_unique<Window_DebugPictureList>(
		Player::menu_offset_x,
		Player::menu_offset_y,
		list_w,
		MENU_HEIGHT
	);

	info_window = std::make_unique<Window_DebugPictureInfo>(
		Player::menu_offset_x + list_w,
		Player::menu_offset_y,
		MENU_WIDTH - list_w,
		MENU_HEIGHT
	);

	list_window->SetActive(true);
	list_window->SetIndex(0);

	// Initialize info window with first item
	info_window->SetPictureId(list_window->GetPictureId());
}

void Scene_DebugPicture::vUpdate() {
	list_window->Update();
	info_window->Update();

	// Update info window based on selection
	if (list_window->GetActive()) {
		// Always refresh info window to see real-time coordinate updates
		info_window->SetPictureId(list_window->GetPictureId());
		info_window->Refresh();
	}

	if (Input::IsTriggered(Input::CANCEL)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
		Scene::Pop();
	}
}
