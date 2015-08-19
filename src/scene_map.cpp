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

Scene_Map::Scene_Map(bool from_save) :
	from_save(from_save) {
	type = Scene::Map;
}

void Scene_Map::Start() {
	spriteset.reset(new Spriteset_Map());
	message_window.reset(new Window_Message(0, SCREEN_TARGET_HEIGHT - 80, SCREEN_TARGET_WIDTH, 80));

	// Draw background to prevent System graphic shining through
	background.reset(new Sprite());
	background->SetBitmap(Bitmap::Create(DisplayUi->GetWidth(), DisplayUi->GetHeight(), Color(0, 0, 0, 255)));
	background->SetZ(-10000);

	screen.reset(new Screen());
	weather.reset(new Weather());

	// Called here instead of Scene Load, otherwise wrong graphic stack
	// is used.
	if (from_save) {
		Main_Data::game_screen->CreatePicturesFromSave();
	}

	Player::FrameReset();
}

Scene_Map::~Scene_Map() {
	Main_Data::game_screen->Reset();
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
	if (Game_Temp::battle_calling) {
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

void Scene_Map::Update() {
	if (Main_Data::game_player->IsTeleporting()) {
		FinishTeleportPlayer();
	}

	Game_Map::GetInterpreter().Update();

	Main_Data::game_party->UpdateTimers();

	Game_Map::Update();
	Main_Data::game_player->Update();
	Main_Data::game_screen->Update();
	spriteset->Update();
	message_window->Update();

	StartTeleportPlayer();

	if (Game_Temp::gameover) {
		Game_Temp::gameover = false;
		Scene::Push(EASYRPG_MAKE_SHARED<Scene_Gameover>());
	}

	if (Game_Temp::to_title) {
		Game_Temp::to_title = false;
		Scene::PopUntil(Scene::Title);
	}

	if (Game_Message::visible)
		return;

	// ESC-Menu calling
	if (Input::IsTriggered(Input::CANCEL)) {
		// Prevent calling when disabled or the main interpreter is waiting
		if (Game_System::GetAllowMenu() &&
			!Game_Map::GetInterpreter().IsWaiting() &&
			!Game_Map::IsPanWaiting()) {
			Game_Temp::menu_calling = true;
		}
	}

	if (Player::debug_flag) {
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

		if (Game_Temp::battle_calling) {
			CallBattle();
			return;
		}

		if (Game_Temp::transition_processing) {
			Game_Temp::transition_processing = false;

			Graphics::Transition(Game_Temp::transition_type, 32, Game_Temp::transition_erase);
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

	Game_Map::Update();

	if (autotransition) {
		Graphics::Transition((Graphics::TransitionType)Game_System::GetTransition(Game_System::Transition_TeleportShow), 32, false);
	}
}

// Scene calling stuff.

void Scene_Map::CallBattle() {
	Main_Data::game_data.system.before_battle_music = Main_Data::game_data.system.current_music;

	Scene::Push(Scene_Battle::Create());
}

void Scene_Map::CallShop() {
	Game_Temp::shop_calling = false;

	Scene::Push(EASYRPG_MAKE_SHARED<Scene_Shop>());
}

void Scene_Map::CallName() {
	Game_Temp::name_calling = false;

	Scene::Push(EASYRPG_MAKE_SHARED<Scene_Name>());
}

void Scene_Map::CallMenu() {
	Game_Temp::menu_calling = false;

	Game_System::SePlay(Main_Data::game_data.system.decision_se);

	// TODO: Main_Data::game_player->Straighten();

	Scene::Push(EASYRPG_MAKE_SHARED<Scene_Menu>());

	/*
	FIXME:
	The intention was that you can still exit the game with ESC when the menu
	is disabled. But this conflicts with parallel events listening for ESC.
	else {
		Scene::Push(EASYRPG_MAKE_SHARED<Scene_End>());
	}*/
}

void Scene_Map::CallSave() {
	Game_Temp::save_calling = false;

	Scene::Push(EASYRPG_MAKE_SHARED<Scene_Save>());
}

void Scene_Map::CallDebug() {
	if (Player::debug_flag) {
		Scene::Push(EASYRPG_MAKE_SHARED<Scene_Debug>());
	}
}
