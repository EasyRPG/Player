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

#ifndef EP_SCENE_MAP_H
#define EP_SCENE_MAP_H

// Headers
#include "scene.h"
#include "spriteset_map.h"
#include "window_message.h"
#include "window_varlist.h"
#include "game_clock.h"
#include "game_map.h"

/**
 * Scene Map class.
 */
class Scene_Map: public Scene {
public:
	/**
	 * Constructor.
	 */
	explicit Scene_Map(int from_save_id);

	~Scene_Map();

	void Start() override;
	void Continue(SceneType prev_scene) override;
	void vUpdate() override;
	void TransitionIn(SceneType prev_scene) override;
	void TransitionOut(SceneType next_scene) override;
	void DrawBackground(Bitmap& dst) override;
	void OnTranslationChanged() override;

	std::unique_ptr<Spriteset_Map> spriteset;

private:
	enum TeleportTransitionRule {
		eTransitionNormal,
		eTransitionFade,
		eTransitionForceFade,
		eTransitionNone
	};

	void Start2(MapUpdateAsyncContext actx);

	struct TeleportParams {
		bool run_foreground_events = false;
		bool erase_screen = false;
		bool use_default_transition_in = false;
		bool defer_recursive_teleports = false;
	};
	void StartPendingTeleport(TeleportParams tp);
	void FinishPendingTeleport(TeleportParams tp);
	void FinishPendingTeleport2(MapUpdateAsyncContext actx, TeleportParams tp);
	void FinishPendingTeleport3(MapUpdateAsyncContext actx, TeleportParams tp);

	void PerformAsyncTeleport(TeleportTarget original_tt);

	void PreUpdate(MapUpdateAsyncContext& actx);
	void PreUpdateForegroundEvents(MapUpdateAsyncContext& actx);

	// Calls map update
	void UpdateStage1(MapUpdateAsyncContext actx);
	// Handles pending teleport and scene changes.
	void UpdateStage2();

	void UpdateSceneCalling();

	void StartInn();
	void UpdateInn();
	void FinishInn();

	template <typename F> void OnAsyncSuspend(F&& f, AsyncOp aop, bool is_preupdate);

	void UpdateGraphics() override;

	std::unique_ptr<Window_Message> message_window;

	int from_save_id = 0;
	bool screen_erased_by_event = false;

	AsyncContinuation map_async_continuation = {};
	lcf::rpg::Music music_before_inn = {};
	bool activate_inn = false;
	bool inn_started = false;
	Game_Clock::time_point inn_timer = {};
};

#endif
