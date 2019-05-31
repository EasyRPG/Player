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
}

void Scene_Map::Start() {
	Scene_Debug::ResetPrevIndices();
	spriteset.reset(new Spriteset_Map());
	message_window.reset(new Window_Message(0, SCREEN_TARGET_HEIGHT - 80, SCREEN_TARGET_WIDTH, 80));

	// Called here instead of Scene Load, otherwise wrong graphic stack
	// is used.
	if (from_save) {
		Main_Data::game_screen->CreatePicturesFromSave();
	}

	Player::FrameReset();

	Start2(MapUpdateAsyncContext());
}

void Scene_Map::Start2(MapUpdateAsyncContext actx) {
	PreUpdate(actx);

	if (actx.IsActive()) {
		OnAsyncSuspend([this,actx]() { Start2(actx); }, true);
		return;
	}

	if (Main_Data::game_player->IsPendingTeleport()) {
		StartPendingTeleport(true, true);
		return;
	}
	// Call any requested scenes when transition is done.
	async_continuation = [&]() { UpdateSceneCalling(); };
}

void Scene_Map::Continue(SceneType prev_scene) {
	if (prev_scene == Scene::Battle) {
		// Came from battle
		Game_System::BgmPlay(Main_Data::game_data.system.before_battle_music);
		return;
	}

	Game_Map::PlayBgm();

	// Player cast Escape / Teleport from menu
	if (Main_Data::game_player->IsPendingTeleport()
			&& Main_Data::game_player->GetTeleportTarget().GetType() == TeleportTarget::eSkillTeleport) {
		FinishPendingTeleport(true, true);
		return;
	}

	spriteset->Update();
}

static bool IsMenuScene(Scene::SceneType scene) {
	switch (scene) {
		case Scene::Shop:
		case Scene::Name:
		case Scene::Menu:
		case Scene::Save:
		case Scene::Load:
		case Scene::Debug:
		case Scene::Skill:
		case Scene::Item:
		case Scene::Teleport:
			return true;
		default:
			break;
	}
	return false;
}

void Scene_Map::TransitionIn(SceneType prev_scene) {
	// Teleport already setup a transition.
	if (Graphics::IsTransitionPending()) {
		return;
	}

	// If an event erased the screen, don't transition in.
	if (screen_erased_by_event) {
		return;
	}

	if (prev_scene == Scene::Battle) {
		Graphics::GetTransition().Init((Transition::TransitionType)Game_System::GetTransition(Game_System::Transition_EndBattleShow), this, 32);
		return;
	}

	if (IsMenuScene(prev_scene)) {
		Scene::TransitionIn(prev_scene);
		return;
	}

	Graphics::GetTransition().Init(Transition::TransitionFadeIn, this, 32);
}

void Scene_Map::TransitionOut(SceneType next_scene) {
	if (next_scene != Scene::Battle
			&& next_scene != Scene::Debug) {
		screen_erased_by_event = false;
	}

	if (next_scene == Scene::Battle) {
		Graphics::GetTransition().Init((Transition::TransitionType)Game_System::GetTransition(Game_System::Transition_BeginBattleErase), this, 32, true);
		Graphics::GetTransition().AppendBefore(Color(255, 255, 255, 255), 12, 2);
		return;
	}
	if (next_scene == Scene::Gameover) {
		Graphics::GetTransition().Init(Transition::TransitionFadeOut, this, 32, true);
		return;
	}
	Scene::TransitionOut(next_scene);
}

void Scene_Map::DrawBackground() {
	if (spriteset->RequireBackground(GetGraphicsState().drawable_list)) {
		DisplayUi->CleanDisplay();
	}
}

void Scene_Map::PreUpdate(MapUpdateAsyncContext& actx) {
	Game_Map::Update(actx, true);
	spriteset->Update();
}

void Scene_Map::Update() {
	MapUpdateAsyncContext actx;
	UpdateStage1(actx);
}

void Scene_Map::UpdateStage1(MapUpdateAsyncContext actx) {
	Game_Map::Update(actx);
	spriteset->Update();

	// Waiting for async operation from map update.
	if (actx.IsActive()) {
		OnAsyncSuspend([this,actx]() { UpdateStage1(actx); }, false);
		return;
	}

	message_window->Update();

	// On platforms with async loading (emscripten) graphical assets loaded this frame
	// may require us to wait for them to download before we can start the transitions.
	AsyncNext([this]() { UpdateStage2(); });
}

void Scene_Map::UpdateStage2() {
	if (Main_Data::game_player->IsPendingTeleport()) {
		StartPendingTeleport(false, false);
		return;
	}
	UpdateSceneCalling();
}

void Scene_Map::UpdateSceneCalling() {
	if (Game_Message::visible)
		return;

	auto call = GetRequestedScene();
	SetRequestedScene(Null);

	if (call == Null && Player::debug_flag) {
		// ESC-Menu calling can be force called when TestPlay mode is on and cancel is pressed 5 times while holding SHIFT
		if (Input::IsPressed(Input::SHIFT)) {
			if (Input::IsTriggered(Input::CANCEL)) {
				debug_menuoverwrite_counter++;
				if (debug_menuoverwrite_counter >= 5) {
					call = Menu;
					debug_menuoverwrite_counter = 0;
				}
			}
		} else {
			debug_menuoverwrite_counter = 0;
		}

		if (call == Null) {
			if (Input::IsTriggered(Input::DEBUG_MENU)) {
				call = Debug;
			}
			else if (Input::IsTriggered(Input::DEBUG_SAVE)) {
				call = Save;
			}
		}
	}

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

void Scene_Map::StartPendingTeleport(bool use_default_transition, bool no_erase) {
	const auto& tt = Main_Data::game_player->GetTeleportTarget();

	FileRequestAsync* request = Game_Map::RequestMap(tt.GetMapId());
	request->SetImportantFile(true);
	request->Start();

	if (!Graphics::IsTransitionErased() && tt.GetType() != TeleportTarget::eVehicleHackTeleport && !no_erase) {
		Graphics::GetTransition().Init((Transition::TransitionType)Game_System::GetTransition(Game_System::Transition_TeleportErase), this, 32, true);
	}

	AsyncNext([this,use_default_transition]() { FinishPendingTeleport(use_default_transition, false); });
}

void Scene_Map::FinishPendingTeleport(bool use_default_transition, bool defer_recursive_teleports) {
	Main_Data::game_player->PerformTeleport();
	Game_Map::PlayBgm();

	spriteset.reset(new Spriteset_Map());
	FinishPendingTeleport2(MapUpdateAsyncContext(), use_default_transition, defer_recursive_teleports);
}

void Scene_Map::FinishPendingTeleport2(MapUpdateAsyncContext actx, bool use_default_transition, bool defer_recursive_teleports) {
	PreUpdate(actx);

	if (actx.IsActive()) {
		OnAsyncSuspend([=] { FinishPendingTeleport2(actx,use_default_transition,defer_recursive_teleports); }, true);
		return;
	}

	if (!defer_recursive_teleports) {
		// RPG_RT behavior - Escape and Teleport skills silently ignore any teleport commands
		// executed by events during pre-update frame and defer them until first frame.
		if (Main_Data::game_player->IsPendingTeleport()) {
			StartPendingTeleport(use_default_transition, true);
			return;
		}
	}

	// This logic was tested against RPG_RT and works this way ...
	if (use_default_transition && Graphics::IsTransitionErased()) {
		Graphics::GetTransition().Init(Transition::TransitionFadeIn, this, 32, false);
	} else if (!use_default_transition && !screen_erased_by_event) {
		Graphics::GetTransition().Init((Transition::TransitionType)Game_System::GetTransition(Game_System::Transition_TeleportShow), this, 32, false);
	}

	// Call any requested scenes when transition is done.
	AsyncNext([this]() { UpdateSceneCalling(); });
}

// Scene calling stuff.

void Scene_Map::CallBattle() {
	Main_Data::game_data.system.before_battle_music = Game_System::GetCurrentBGM();
	Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_BeginBattle));
	Game_System::BgmPlay(Game_System::GetSystemBGM(Game_System::BGM_Battle));

	Scene::Push(Scene_Battle::Create());
}

void Scene_Map::CallShop() {
	Scene::Push(std::make_shared<Scene_Shop>());
}

void Scene_Map::CallName() {
	Scene::Push(std::make_shared<Scene_Name>());
}

void Scene_Map::CallMenu() {
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
	Scene::Push(std::make_shared<Scene_Save>());
}

void Scene_Map::CallLoad() {
	Scene::Push(std::make_shared<Scene_Load>());
}

void Scene_Map::CallDebug() {
	if (Player::debug_flag) {
		Scene::Push(std::make_shared<Scene_Debug>());
	}
}

void Scene_Map::CallGameover() {
	Scene::Push(std::make_shared<Scene_Gameover>());
}

template <typename F>
void Scene_Map::AsyncNext(F&& f) {
	if (IsAsyncPending()) {
		async_continuation = std::forward<F>(f);
	} else {
		f();
	}
}

template <typename F>
void Scene_Map::OnAsyncSuspend(F&& f, bool is_preupdate) {
	if (CheckInterpreterExit()) {
		return;
	}

	if (Game_Temp::transition_processing) {
		Graphics::GetTransition().Init(Game_Temp::transition_type, this, 32, Game_Temp::transition_erase);
		if (!Game_Temp::transition_erase || !is_preupdate) {
			// RPG_RT behavior: EraseScreen commands performed during pre-update don't stick.
			screen_erased_by_event = Game_Temp::transition_erase;
		}

		Game_Temp::transition_processing = false;
		Game_Temp::transition_erase = false;
		Game_Temp::transition_type = Transition::TransitionNone;;
	}

	AsyncNext(std::forward<F>(f));
}

