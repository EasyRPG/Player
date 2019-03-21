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
#include "transition.h"
#include "audio.h"
#include "input.h"
#include "screen.h"
#include "scene_load.h"
#include "player.h"
#include "output.h"

Scene_Map::Scene_Map(bool from_save) :
	from_save(from_save) {
	type = Scene::Map;
}

Scene_Map::~Scene_Map() {
	Game_Temp::transition_menu = false;
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

	PreUpdate();
	HandleTeleportPreUpdateLoop();
	call_scenes_on_transition_in = true;
}

void Scene_Map::Continue() {
	if (called_battle) {
		// Came from battle
		Game_System::BgmPlay(Main_Data::game_data.system.before_battle_music);
	}
	else {
		Game_Map::PlayBgm();
	}
	spriteset->Update();
}

void Scene_Map::Resume() {
	called_battle = false;

	if (call_scenes_on_transition_in) {
		UpdateSceneCalling();
		call_scenes_on_transition_in = false;
	}
}

void Scene_Map::TransitionIn() {
	if (Main_Data::game_player->IsPendingTeleport()) {
		// Escape / Teleport spell from menu lands here.
		HandleTeleportPreUpdateLoop();
		// Escape / Teleport spell always uses fade.
		Player::TransitionShow(Transition::TransitionFadeIn, 32, this);
		return;
	}

	if (called_battle) {
		if (!screen_erased_by_event) {
			Player::TransitionShow((Transition::TransitionType)Game_System::GetTransition(Game_System::Transition_EndBattleShow), 32, this);
		}
	} else if (Game_Temp::transition_menu) {
		Game_Temp::transition_menu = false;
		Scene::TransitionIn();
	} else {
		Player::TransitionShow(Transition::TransitionFadeIn, 32, this);
	}
}

void Scene_Map::TransitionOut() {
	if (called_battle) {
		Player::TransitionEraseBattle((Transition::TransitionType)Game_System::GetTransition(Game_System::Transition_BeginBattleErase), 32, this);
		return;
	}

	screen_erased_by_event = false;
	if (Scene::instance && Scene::instance->type == Scene::Gameover) {
		Player::TransitionErase(Transition::TransitionFadeOut, 32, this);
		return;
	}
	Scene::TransitionOut();
}

void Scene_Map::DrawBackground() {
	if (spriteset->RequireBackground(GetGraphicsState().drawable_list)) {
		DisplayUi->CleanDisplay();
	}
}

void Scene_Map::PreUpdate() {
	Game_Map::Update(true);
	spriteset->Update();
}

void Scene_Map::Update() {
	Main_Data::game_party->UpdateTimers();

	Main_Data::game_screen->Update();
	Game_Map::Update();

	if (Graphics::IsTransitionErased()) {
		screen_erased_by_event = true;
	}

	spriteset->Update();
	message_window->Update();

	bool did_teleport = HandleTeleportPreUpdateLoop();

	if (did_teleport && !screen_erased_by_event) {
		Player::TransitionShow((Transition::TransitionType)Game_System::GetTransition(Game_System::Transition_TeleportShow), 32, this);
	}

	UpdateSceneCalling();
}

void Scene_Map::UpdateSceneCalling() {
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
					SetRequestedScene(Menu);
					debug_menuoverwrite_counter = 0;
				}
			}
		} else {
			debug_menuoverwrite_counter = 0;
		}

		if (Input::IsTriggered(Input::DEBUG_MENU)) {
			SetRequestedScene(Debug);
		}
		else if (Input::IsTriggered(Input::DEBUG_SAVE)) {
			SetRequestedScene(Save);
		}
	}

	auto call = GetRequestedScene();

	SetRequestedScene(Null);
	switch (call) {
		case Scene::Menu:
			CallMenu();
			break;
		case Scene::Shop:
			CallShop();
			break;
		case Scene::Name:
			CallName();
			break;
		case Scene::Save:
			CallSave();
			break;
		case Scene::Load:
			CallLoad();
			break;
		case Scene::Battle:
			CallBattle();
			break;
		case Scene::Gameover:
			CallGameover();
			break;
		case Scene::Debug:
			CallDebug();
			break;
		default:
			break;
	}
}

void Scene_Map::HandleTeleport() {
	const auto& tt = Main_Data::game_player->GetTeleportTarget();

	FileRequestAsync* request = Game_Map::RequestMap(tt.GetMapId());
	request->SetImportantFile(true);
	request->Start();

	if (!Graphics::IsTransitionErased()) {
		Player::TransitionErase((Transition::TransitionType)Game_System::GetTransition(Game_System::Transition_TeleportErase), 32, this);
	}

	Main_Data::game_player->PerformTeleport();
	Game_Map::PlayBgm();

	spriteset.reset(new Spriteset_Map());
}

int Scene_Map::HandleTeleportPreUpdateLoop() {
	int i = 0;
	while (Main_Data::game_player->IsPendingTeleport()) {
		HandleTeleport();
		PreUpdate();
		++i;

		if (i > 10000) {
			Output::Error("Infinite teleport event loop detected!");
		}
	}
	return i;
}

// Scene calling stuff.

void Scene_Map::CallBattle() {
	called_battle = true;
	Main_Data::game_data.system.before_battle_music = Game_System::GetCurrentBGM();
	Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_BeginBattle));
	Game_System::BgmPlay(Game_System::GetSystemBGM(Game_System::BGM_Battle));

	Scene::Push(Scene_Battle::Create());
}

void Scene_Map::CallShop() {
	Game_Temp::transition_menu = true;

	Scene::Push(std::make_shared<Scene_Shop>());
}

void Scene_Map::CallName() {
	Game_Temp::transition_menu = true;

	Scene::Push(std::make_shared<Scene_Name>());
}

void Scene_Map::CallMenu() {
	Game_Temp::transition_menu = true;

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
	Game_Temp::transition_menu = true;

	Scene::Push(std::make_shared<Scene_Save>());
}

void Scene_Map::CallLoad() {
	Game_Temp::transition_menu = true;

	Scene::Push(std::make_shared<Scene_Load>());
}

void Scene_Map::CallDebug() {
	if (Player::debug_flag) {
		Game_Temp::transition_menu = true;
		Scene::Push(std::make_shared<Scene_Debug>());
	}
}

void Scene_Map::CallGameover() {
	Scene::Push(std::make_shared<Scene_Gameover>());
}
