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

	teleport_from_other_scene = true;

	// Called here instead of Scene Load, otherwise wrong graphic stack
	// is used.
	if (from_save) {
		Main_Data::game_screen->CreatePicturesFromSave();
	}

	Player::FrameReset();

	PreUpdate();
	// FIXME: Handle transitions requested on the first frame of a new game by PreUpdate!!
	if (Game_Temp::transition_processing) {
		Game_Temp::transition_processing = false;
		Game_Temp::transition_erase = false;
		Game_Temp::transition_type = Transition::TransitionNone;
	}

	if (Main_Data::game_player->IsPendingTeleport()) {
		StartPendingTeleport();
		return;
	}
	// Call any requested scenes when transition is done.
	async_continuation = [&]() { UpdateSceneCalling(); };
}

void Scene_Map::Continue() {
	teleport_from_other_scene = true;
	if (Game_Temp::battle_calling) {
		// Came from battle
		Game_System::BgmPlay(Main_Data::game_data.system.before_battle_music);
	}
	else {
		Game_Map::PlayBgm();
	}

	// Player cast Escape / Teleport from menu
	if (Main_Data::game_player->IsPendingTeleport()) {
		FinishPendingTeleport();
		return;
	}

	spriteset->Update();
}

void Scene_Map::Resume() {
	Game_Temp::battle_calling = false;
	teleport_from_other_scene = false;
}

void Scene_Map::TransitionIn() {
	// Teleport already setup a transition.
	if (Graphics::IsTransitionPending()) {
		return;
	}
	if (Game_Temp::battle_calling) {
		Graphics::GetTransition().Init((Transition::TransitionType)Game_System::GetTransition(Game_System::Transition_EndBattleShow), this, 32);
	} else if (Game_Temp::transition_menu) {
		Game_Temp::transition_menu = false;
		Scene::TransitionIn();
	} else {
		Graphics::GetTransition().Init(Transition::TransitionFadeIn, this, 32);
	}
}

void Scene_Map::TransitionOut() {
	if (Game_Temp::battle_calling) {
		Graphics::GetTransition().Init((Transition::TransitionType)Game_System::GetTransition(Game_System::Transition_BeginBattleErase), this, 32, true);
		Graphics::GetTransition().AppendBefore(Color(255, 255, 255, 255), 12, 2);
	}
	else if (Scene::instance && Scene::instance->type == Scene::Gameover) {
		Graphics::GetTransition().Init(Transition::TransitionFadeOut, this, 32, true);
	}
	else {
		Scene::TransitionOut();
	}
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
	Game_Map::Update();
	spriteset->Update();
	message_window->Update();

	// On platforms with async loading (emscripten) graphical assets loaded this frame
	// may require us to wait for them to download before we can start the transitions.
	if (IsAsyncPending()) {
		async_continuation = [this]() { UpdateStage2(); };
		return;
	}

	UpdateStage2();
}

void Scene_Map::UpdateStage2() {
	if (!Game_Temp::transition_processing) {
		UpdateStage3();
		return;
	}

	Game_Temp::transition_processing = false;

	// Do the transition and then finish the update routine.
	// FIXME: This behavior is incomplete as the update loop has to be
	// resumed at exactly the right time. In particular if a parallel
	// event requests a transition, we have to continue running the interpreter
	// and all stages of the update loop afterwards.
	// This will be fixed later.

	Graphics::GetTransition().Init(Game_Temp::transition_type, this, 32, Game_Temp::transition_erase);
	// Unless its an instant transition, we must wait for it to finish before we can proceed.
	if (IsAsyncPending()) {
		async_continuation = [this]() { UpdateStage3(); };
		return;
	}
	UpdateStage3();
}

void Scene_Map::UpdateStage3() {
	if (Main_Data::game_player->IsPendingTeleport()) {
		StartPendingTeleport();
		return;
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

	bool force_menu_calling = false;
	if (Player::debug_flag) {
		// ESC-Menu calling can be force called when TestPlay mode is on and cancel is pressed 5 times while holding SHIFT
		if (Input::IsPressed(Input::SHIFT)) {
			if (Input::IsTriggered(Input::CANCEL)) {
				debug_menuoverwrite_counter++;
				if (debug_menuoverwrite_counter >= 5) {
					force_menu_calling = true;
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

	if (!Main_Data::game_player->IsMoving() || HasRequestedScene() || force_menu_calling) {
		auto call = GetRequestedScene();

		if (Main_Data::game_data.party_location.menu_calling || force_menu_calling) {
			call = Scene::Menu;
		}
		if (Game_Temp::battle_calling) {
			call = Scene::Battle;
		}

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
			default:
				break;
		}
	}
}

void Scene_Map::StartPendingTeleport() {
	const auto& tt = Main_Data::game_player->GetTeleportTarget();

	FileRequestAsync* request = Game_Map::RequestMap(tt.GetMapId());
	request->SetImportantFile(true);
	request->Start();

	if (!Graphics::IsTransitionErased()) {
		Graphics::GetTransition().Init((Transition::TransitionType)Game_System::GetTransition(Game_System::Transition_TeleportErase), this, 32, true);
	}

	if (IsAsyncPending()) {
		async_continuation = [&]() { FinishPendingTeleport(); };
		return;
	}

	FinishPendingTeleport();
}

void Scene_Map::FinishPendingTeleport() {
	Main_Data::game_player->PerformTeleport();
	Game_Map::PlayBgm();

	spriteset.reset(new Spriteset_Map());

	PreUpdate();
	// FIXME: Handle transitions requested on the preupdate frame after a teleport!
	if (Game_Temp::transition_processing) {
		Game_Temp::transition_processing = false;
		Game_Temp::transition_erase = false;
		Game_Temp::transition_type = Transition::TransitionNone;
	}

	if (Main_Data::game_player->IsPendingTeleport()) {
		StartPendingTeleport();
		return;
	}

	// Event forced the screen to erased, so we're done here.
	if (Game_Temp::transition_erase) {
		UpdateSceneCalling();
		return;
	}

	if (teleport_from_other_scene) {
		Graphics::GetTransition().Init(Transition::TransitionFadeIn, this, 32, false);
	} else {
		Graphics::GetTransition().Init((Transition::TransitionType)Game_System::GetTransition(Game_System::Transition_TeleportShow), this, 32, false);
	}

	// Call any requested scenes when transition is done.
	async_continuation = [&]() { UpdateSceneCalling(); };
}

// Scene calling stuff.

void Scene_Map::CallBattle() {
	Game_Temp::battle_calling = true;
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
	Main_Data::game_data.party_location.menu_calling = false;
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
