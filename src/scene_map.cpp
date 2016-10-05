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
#include "scene_map.h"
#include "scene_menu.h"
#include "scene_title.h"
#include "scene_end.h"
#include "scene_name.h"
#include "scene_shop.h"
#include "scene_save.h"
#include "scene_battle.h"
#include "scene_debug.h"
#include "main_data.h"
#include "game_map.h"
#include "game_message.h"
#include "game_party.h"
#include "game_player.h"
#include "game_system.h"
#include "game_temp.h"
#include "rpg_system.h"
#include "player.h"
#include "graphics.h"
#include "audio.h"
#include "input.h"
#include "screen.h"
#include "scene_load.h"

Scene_Map::Scene_Map(bool from_save) :
	from_save(from_save) {
	type = Scene::Map;
}

void Scene_Map::Start() {
	spriteset.reset(new Spriteset_Map());
	message_window.reset(new Window_Message(0, SCREEN_TARGET_HEIGHT - 80, SCREEN_TARGET_WIDTH, 80));

	// Called here instead of Scene Load, otherwise wrong graphic stack
	// is used.
	if (from_save) {
		Main_Data::game_screen->CreatePicturesFromSave();
	}

	Player::FrameReset();
	Game_Map::Update(true);
}

Scene_Map::~Scene_Map() {
	Main_Data::Cleanup();
}

void Scene_Map::Continue() {
	if (Game_Temp::battle_calling) {
		// Came from battle
		Game_System::BgmPlay(Main_Data::game_data.system.before_battle_music);
	}
	else {
		Game_Map::PlayBgm();
	}
}

void Scene_Map::Resume() {
	Game_Temp::battle_calling = false;
}

void Scene_Map::TransitionIn() {
	if (Main_Data::game_player->IsTeleporting()) {
		// Comes from the teleport scene
		// Teleport will handle fade-in
		return;
	} else if (Game_Temp::battle_calling) {
		Graphics::Transition((Graphics::TransitionType)Game_System::GetTransition(Game_System::Transition_EndBattleShow), 32);
	}
	else {
		Graphics::Transition(Graphics::TransitionFadeIn, 32);
	}
}

void Scene_Map::TransitionOut() {
	if (Game_Temp::battle_calling) {
		Graphics::Transition((Graphics::TransitionType)Game_System::GetTransition(Game_System::Transition_BeginBattleErase), 32, true);
	}
	else {
		Scene::TransitionOut();
	}
}

bool Scene_Map::DrawBackground() {
	return false;
}

void Scene_Map::Update() {
	if (Game_Temp::transition_processing) {
		Game_Temp::transition_processing = false;

		Graphics::Transition(Game_Temp::transition_type, 32, Game_Temp::transition_erase);
	}

	if (auto_transition) {
		auto_transition = false;

		if (!auto_transition_erase) {
			// Fade Out not handled here but in StartTeleportPlayer because otherwise
			// emscripten hangs before fading out when doing async loading...
			Graphics::Transition((Graphics::TransitionType)Game_System::GetTransition(Game_System::Transition_TeleportShow), 32, false);
			return;
		}
	}

	// Async loading note:
	// Fade In must be done before finish teleport, otherwise chipset is not
	// loaded and renders black while fading -> ugly

	if (!Game_Map::IsTeleportDelayed() && Main_Data::game_player->IsTeleporting()) {
		FinishTeleportPlayer();
		return;
	}
	// The delay is only needed for one frame to execute pending transitions,
	// the interpreters continue on the old map afterwards
	Game_Map::SetTeleportDelayed(false);

	Main_Data::game_party->UpdateTimers();

	Game_Map::Update();
	Main_Data::game_screen->Update();
	spriteset->Update();
	message_window->Update();

	StartTeleportPlayer();

	if (Game_Temp::gameover) {
		Game_Temp::gameover = false;
		Scene::Push(std::make_shared<Scene_Gameover>());
	}

	if (Game_Temp::to_title) {
		Game_Temp::to_title = false;
		Scene::PopUntil(Scene::Title);
	}

	if (Game_Message::visible)
		return;

	if (Player::debug_flag) {
		// ESC-Menu calling can be force called when TestPlay mode is on and cancel is pressed 5 times while holding SHIFT
		if (Input::IsPressed(Input::SHIFT)) {
			if (Input::IsTriggered(Input::CANCEL)) {
				debug_menuoverwrite_counter++;
				if (debug_menuoverwrite_counter >= 5) {
					Game_Temp::menu_calling = true;
					debug_menuoverwrite_counter = 0;
				}
			}
		} else {
			debug_menuoverwrite_counter = 0;
		}

		if (Input::IsTriggered(Input::DEBUG_MENU)) {
			CallDebug();
		}
		else if (Input::IsTriggered(Input::DEBUG_SAVE)) {
			CallSave();
		}
	}

	if (!Main_Data::game_player->IsMoving()) {
		if (Game_Temp::menu_calling) {
			CallMenu();
			return;
		}

		if (Game_Temp::name_calling) {
			CallName();
			return;
		}

		if (Game_Temp::shop_calling) {
			CallShop();
			return;
		}

		if (Game_Temp::save_calling) {
			CallSave();
			return;
		}

		if (Game_Temp::load_calling) {
			CallLoad();
			return;
		}

		if (Game_Temp::battle_calling) {
			CallBattle();
			return;
		}
	}
}

void Scene_Map::StartTeleportPlayer() {
	if (!Main_Data::game_player->IsTeleporting())
		return;
	bool const autotransition = !Game_Temp::transition_erase;

	if (autotransition) {
		Graphics::Transition((Graphics::TransitionType)Game_System::GetTransition(Game_System::Transition_TeleportErase), 32, true);
	}
}

void Scene_Map::FinishTeleportPlayer() {
	bool const autotransition = !Game_Temp::transition_erase;

	Main_Data::game_player->PerformTeleport();
	Game_Map::PlayBgm();

	spriteset.reset(new Spriteset_Map());

	Game_Map::Update(true);

	if (autotransition) {
		auto_transition = true;
		auto_transition_erase = false;
	}
}

// Scene calling stuff.

void Scene_Map::CallBattle() {
	Main_Data::game_data.system.before_battle_music = Game_System::GetCurrentBGM();
	Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_BeginBattle));

	Scene::Push(Scene_Battle::Create());
}

void Scene_Map::CallShop() {
	Game_Temp::shop_calling = false;

	Scene::Push(std::make_shared<Scene_Shop>());
}

void Scene_Map::CallName() {
	Game_Temp::name_calling = false;

	Scene::Push(std::make_shared<Scene_Name>());
}

void Scene_Map::CallMenu() {
	Game_Temp::menu_calling = false;

	Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));

	// TODO: Main_Data::game_player->Straighten();

	Scene::Push(std::make_shared<Scene_Menu>());

	/*
	FIXME:
	The intention was that you can still exit the game with ESC when the menu
	is disabled. But this conflicts with parallel events listening for ESC.
	else {
		Scene::Push(std::make_shared<Scene_End>());
	}*/
}

void Scene_Map::CallSave() {
	Game_Temp::save_calling = false;

	Scene::Push(std::make_shared<Scene_Save>());
}

void Scene_Map::CallLoad() {
	Game_Temp::load_calling = false;

	Scene::Push(std::make_shared<Scene_Load>());
}

void Scene_Map::CallDebug() {
	if (Player::debug_flag) {
		Scene::Push(std::make_shared<Scene_Debug>());
	}
}
