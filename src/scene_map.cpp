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
#include "scene_save.h"
#include "scene_debug.h"
#include "main_data.h"
#include "game_map.h"
#include "game_message.h"
#include "game_party.h"
#include "game_player.h"
#include "game_system.h"
#include "game_screen.h"
#include "game_pictures.h"
#include <lcf/rpg/system.h>
#include "player.h"
#include "transition.h"
#include "audio.h"
#include "input.h"
#include "screen.h"
#include "scene_load.h"
#include "output.h"

static bool GetRunForegroundEvents(TeleportTarget::Type tt) {
	switch (tt) {
		case TeleportTarget::eForegroundTeleport:
			return true;
		case TeleportTarget::eParallelTeleport:
		case TeleportTarget::eSkillTeleport:
		case TeleportTarget::eAsyncQuickTeleport:
			break;
	}
	return false;
}

Scene_Map::Scene_Map(bool from_save)
	: from_save(from_save)
{
	type = Scene::Map;

	SetUseSharedDrawables(true);

	// New Game and Load Game always have a delay, so it set it by default in constructor.
	SetDelayFrames(Scene::kStartGameDelayFrames);
}

Scene_Map::~Scene_Map() {
}

void Scene_Map::Start() {
	Scene_Debug::ResetPrevIndices();
	spriteset.reset(new Spriteset_Map());
	message_window.reset(new Window_Message(0, SCREEN_TARGET_HEIGHT - 80, SCREEN_TARGET_WIDTH, 80));

	Game_Message::SetWindow(message_window.get());

	// Called here instead of Scene Load, otherwise wrong graphic stack
	// is used.
	if (from_save) {
		auto current_music = Game_System::GetCurrentBGM();
		Game_System::BgmStop();
		Game_System::BgmPlay(current_music);
	} else {
		Game_Map::PlayBgm();
	}

	Main_Data::game_screen->InitGraphics();
	Main_Data::game_pictures->InitGraphics();
	Game_Clock::ResetFrame(Game_Clock::now());

	Start2(MapUpdateAsyncContext());
}

void Scene_Map::Start2(MapUpdateAsyncContext actx) {
	PreUpdate(actx);

	if (actx.IsActive()) {
		OnAsyncSuspend([this,actx]() { Start2(actx); }, actx.GetAsyncOp(), true);
		return;
	}

	if (Main_Data::game_player->IsPendingTeleport()) {
		TeleportParams tp;
		auto tt = Main_Data::game_player->GetTeleportTarget().GetType();
		tp.run_foreground_events = GetRunForegroundEvents(tt);
		tp.erase_screen = false;
		tp.use_default_transition_in = true;
		tp.defer_recursive_teleports = false;
		StartPendingTeleport(tp);
		return;
	}

	// We do the start game fade in transition here instead of TransitionIn callback,
	// in order to make async logic work properly.
	auto& transition = Transition::instance();
	if (transition.IsErasedNotActive()) {
		transition.InitShow(Transition::TransitionFadeIn, this);
	}

	// Call any requested scenes when transition is done.
	AsyncNext([this]() { UpdateSceneCalling(); });
}

void Scene_Map::Continue(SceneType prev_scene) {
	Game_Message::SetWindow(message_window.get());

	if (prev_scene == Scene::Battle) {
		Game_Map::OnContinueFromBattle();
	}

	// Player cast Escape / Teleport from menu
	if (Main_Data::game_player->IsPendingTeleport()) {
		auto tt = Main_Data::game_player->GetTeleportTarget().GetType();
		TeleportParams tp;
		tp.run_foreground_events = GetRunForegroundEvents(tt);
		tp.erase_screen = false;
		tp.use_default_transition_in = true;
		tp.defer_recursive_teleports = (tt == TeleportTarget::eSkillTeleport);
		FinishPendingTeleport(tp);
		return;
	}

	UpdateGraphics();
}

void Scene_Map::UpdateGraphics() {
	spriteset->Update();
	Main_Data::game_screen->UpdateGraphics();
	Main_Data::game_pictures->UpdateGraphics(false);
}

void Scene_Map::TransitionIn(SceneType prev_scene) {
	auto& transition = Transition::instance();

	// Teleport already setup a transition.
	if (transition.IsActive()) {
		return;
	}

	// If an event erased the screen, don't transition in.
	if (screen_erased_by_event) {
		return;
	}

	if (prev_scene == Scene::Battle) {
		transition.InitShow(Game_System::GetTransition(Game_System::Transition_EndBattleShow), this);
		return;
	}

	Scene::TransitionIn(prev_scene);
}

void Scene_Map::Suspend(SceneType next_scene) {
	if (next_scene == Scene::Title) {
		Game_System::BgmStop();
	}
}

void Scene_Map::TransitionOut(SceneType next_scene) {
	auto& transition = Transition::instance();

	if (next_scene != Scene::Battle
			&& next_scene != Scene::Debug) {
		screen_erased_by_event = false;
	}

	if (next_scene == Scene::Debug) {
		transition.InitErase(Transition::TransitionCutOut, this);
		return;
	}

	if (next_scene == Scene::Battle) {
		if (!transition.IsErasedNotActive()) {
			auto tt = Game_System::GetTransition(Game_System::Transition_BeginBattleErase);
			if (tt == Transition::TransitionNone) {
				// If transition type is none, RPG_RT flashes and then waits 40 frames before starting the battle.
				transition.InitErase(Transition::TransitionCutOut, this, 40);
			} else {
				transition.InitErase(tt, this);
			}
			transition.PrependFlashes(31, 31, 31, 31, 10, 2);
		} else {
			// If screen is already erased, RPG_RT does nothing for 40 frames.
			transition.InitErase(Transition::TransitionNone, this, 40);
		}
		return;
	}
	if (next_scene == Scene::Gameover) {
		transition.InitErase(Transition::TransitionFadeOut, this);
		return;
	}
	Scene::TransitionOut(next_scene);
}

void Scene_Map::DrawBackground(Bitmap& dst) {
	if (spriteset->RequireClear(GetDrawableList())) {
		dst.Clear();
	}
}

void Scene_Map::PreUpdate(MapUpdateAsyncContext& actx) {
	Game_Map::Update(actx, true);
	UpdateGraphics();
}

void Scene_Map::PreUpdateForegroundEvents(MapUpdateAsyncContext& actx) {
	Game_Map::UpdateForegroundEvents(actx);
	UpdateGraphics();
}

void Scene_Map::Update() {
	if (activate_inn) {
		UpdateInn();
		return;
	}
	MapUpdateAsyncContext actx;
	UpdateStage1(actx);
}

void Scene_Map::UpdateStage1(MapUpdateAsyncContext actx) {
	Game_Map::Update(actx);
	UpdateGraphics();

	// Waiting for async operation from map update.
	if (actx.IsActive()) {
		OnAsyncSuspend([this,actx]() { UpdateStage1(actx); }, actx.GetAsyncOp(), false);
		return;
	}

	// On platforms with async loading (emscripten) graphical assets loaded this frame
	// may require us to wait for them to download before we can start the transitions.
	AsyncNext([this]() { UpdateStage2(); });
}

void Scene_Map::UpdateStage2() {
	if (Main_Data::game_player->IsPendingTeleport()) {
		const auto tt = Main_Data::game_player->GetTeleportTarget().GetType();
		TeleportParams tp;
		tp.run_foreground_events = GetRunForegroundEvents(tt);
		tp.erase_screen = true;
		tp.use_default_transition_in = false;
		tp.defer_recursive_teleports = false;

		StartPendingTeleport(tp);
		return;
	}
	UpdateSceneCalling();
}

void Scene_Map::UpdateSceneCalling() {

	auto call = TakeRequestedScene();

	if (call == nullptr
			&& Player::debug_flag
			&& !Game_Message::IsMessageActive())
	{
		// ESC-Menu calling can be force called when TestPlay mode is on and cancel is pressed 5 times while holding SHIFT
		if (Input::IsPressed(Input::SHIFT)) {
			if (Input::IsTriggered(Input::CANCEL)) {
				debug_menuoverwrite_counter++;
				if (debug_menuoverwrite_counter >= 5) {
					call = std::make_shared<Scene_Menu>();
					debug_menuoverwrite_counter = 0;
				}
			}
		} else {
			debug_menuoverwrite_counter = 0;
		}

		if (call == nullptr) {
			if (Input::IsTriggered(Input::DEBUG_MENU)) {
				call = std::make_shared<Scene_Debug>();
			}
			else if (Input::IsTriggered(Input::DEBUG_SAVE)) {
				call = std::make_shared<Scene_Save>();
			}
		}
	}

	if (call != nullptr) {
		Scene::Push(std::move(call));
	}
}

void Scene_Map::StartPendingTeleport(TeleportParams tp) {
	auto& transition = Transition::instance();

	if (!transition.IsErasedNotActive() && tp.erase_screen) {
		transition.InitErase(Game_System::GetTransition(Game_System::Transition_TeleportErase), this);
	}

	AsyncNext([=]() { FinishPendingTeleport(tp); });
}

void Scene_Map::FinishPendingTeleport(TeleportParams tp) {
	auto old_map_id = Game_Map::GetMapId();
	Main_Data::game_player->PerformTeleport();

	if (Game_Map::GetMapId() != old_map_id) {
		spriteset.reset(new Spriteset_Map());
	}
	FinishPendingTeleport2(MapUpdateAsyncContext(), tp);
}

void Scene_Map::FinishPendingTeleport2(MapUpdateAsyncContext actx, TeleportParams tp) {
	PreUpdate(actx);

	if (actx.IsActive()) {
		OnAsyncSuspend([=] { FinishPendingTeleport2(actx, tp); }, actx.GetAsyncOp(), true);
		return;
	}

	if (!tp.defer_recursive_teleports) {
		// RPG_RT behavior - Escape and Teleport skills silently ignore any teleport commands
		// executed by events during pre-update frame and defer them until first frame.
		if (Main_Data::game_player->IsPendingTeleport()) {
			tp.erase_screen = false;
			StartPendingTeleport(tp);
			return;
		}
	}

	auto& transition = Transition::instance();

	// This logic was tested against RPG_RT and works this way ...
	if (tp.use_default_transition_in && transition.IsErasedNotActive()) {
		transition.InitShow(Transition::TransitionFadeIn, this);
	} else if (!tp.use_default_transition_in && !screen_erased_by_event) {
		transition.InitShow(Game_System::GetTransition(Game_System::Transition_TeleportShow), this);
	}

	// Call any requested scenes when transition is done.
	AsyncNext([=]() { FinishPendingTeleport3(actx, tp); });
}

void Scene_Map::FinishPendingTeleport3(MapUpdateAsyncContext actx, TeleportParams tp) {
	if (tp.run_foreground_events) {
		PreUpdateForegroundEvents(actx);

		if (actx.IsActive()) {
			OnAsyncSuspend([=] { FinishPendingTeleport3(actx, tp); }, actx.GetAsyncOp(), true);
			return;
		}

		if (!tp.defer_recursive_teleports) {
			// See comments about defer_recursive_teleports in FinishPendingTeleport2
			// Deferring in this block can actually never occur, since an Escape/Teleport won't ever
			// also trigger foreground events to run in pre-update. We leave the check in here for symmetry.
			if (Main_Data::game_player->IsPendingTeleport()) {
				tp.erase_screen = false;
				StartPendingTeleport(tp);
				return;
			}
		}
	}

	// Call any requested scenes when transition is done.
	AsyncNext([this]() { UpdateSceneCalling(); });
}

void Scene_Map::PerformAsyncTeleport(TeleportTarget original_tt) {
	Main_Data::game_player->PerformTeleport();
	Main_Data::game_player->ResetTeleportTarget(original_tt);

	spriteset.reset(new Spriteset_Map());

	AsyncNext(std::move(map_async_continuation));
}

template <typename F>
void Scene_Map::OnAsyncSuspend(F&& f, AsyncOp aop, bool is_preupdate) {
	if (CheckSceneExit(aop)) {
		return;
	}

	auto& transition = Transition::instance();

	if (aop.GetType() == AsyncOp::eEraseScreen) {
		auto tt = static_cast<Transition::Type>(aop.GetTransitionType());
		if (tt == Transition::TransitionNone) {
			// Emulates an RPG_RT bug where instantaneous transitions cause a
			// 30 frame pause and then make the screen black.
			transition.InitErase(Transition::TransitionCutOut, this, 30);
		} else {
			transition.InitErase(tt, this);
		}

		if (!is_preupdate) {
			// RPG_RT behavior: EraseScreen commands performed during pre-update don't stick.
			screen_erased_by_event = true;
		}
	}

	if (aop.GetType() == AsyncOp::eShowScreen) {
		auto tt = static_cast<Transition::Type>(aop.GetTransitionType());
		transition.InitShow(tt, this);
		screen_erased_by_event = false;
	}

	if (aop.GetType() == AsyncOp::eCallInn) {
		activate_inn = true;
		inn_started = false;
		music_before_inn = Game_System::GetCurrentBGM();
		map_async_continuation = std::forward<F>(f);

		Game_System::BgmFade(800);

		UpdateInn();
		return;
	}

	if (aop.GetType() == AsyncOp::eQuickTeleport) {
		map_async_continuation = std::forward<F>(f);

		// If there is already a real teleport pending we need to make sure it gets executed after
		// the async teleport.
		auto orig_tt = Main_Data::game_player->GetTeleportTarget();

		Main_Data::game_player->ReserveTeleport(aop.GetTeleportMapId(), aop.GetTeleportX(), aop.GetTeleportY(), -1, TeleportTarget::eAsyncQuickTeleport);

		AsyncNext([=]() { PerformAsyncTeleport(orig_tt); });
		return;
	}

	AsyncNext(std::forward<F>(f));
}

void Scene_Map::StartInn() {
	const lcf::rpg::Music& bgm_inn = Game_System::GetSystemBGM(Game_System::BGM_Inn);
	if (Game_System::IsStopMusicFilename(bgm_inn.name)) {
		FinishInn();
		return;
	}

	Game_System::BgmPlay(bgm_inn);
}

void Scene_Map::FinishInn() {
	// RPG_RT will always transition in, regardless of whether an EraseScreen command
	// was issued previously.
	screen_erased_by_event = false;

	auto& transition = Transition::instance();

	transition.InitShow(Transition::TransitionFadeIn, Scene::instance.get());
	Game_System::BgmPlay(music_before_inn);

	// Full heal
	std::vector<Game_Actor*> actors = Main_Data::game_party->GetActors();
	for (Game_Actor* actor : actors) {
		actor->FullHeal();
	}

	activate_inn = false;
	inn_started = false;
	AsyncNext(std::move(map_async_continuation));
}

void Scene_Map::UpdateInn() {
	// Allow message box to render during inn sequence.
	if (Game_Message::IsMessageActive()) {
		Game_Message::Update();
		return;
	}

	if (!inn_started) {
		Transition::instance().InitErase(Transition::TransitionFadeOut, Scene::instance.get());
		inn_started = true;

		AsyncNext([=]() { StartInn(); });
		return;
	}

	if (Audio().BGM_IsPlaying() && !Audio().BGM_PlayedOnce()) {
		return;
	}

	Game_System::BgmStop();
	FinishInn();
}
