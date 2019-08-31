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

#ifndef EP_SCENE_BATTLE_RPG2K3_H
#define EP_SCENE_BATTLE_RPG2K3_H

// Headers
#include "scene_battle.h"
#include "async_handler.h"
#include "window_actorsp.h"

/**
 * Scene_Battle class.
 * Manages the battles.
 */
class Scene_Battle_Rpg2k3 : public Scene_Battle {
public:
	enum BattleActionState {
		/**
		 * 1st action, called repeatedly.
		 * Handles healing of conditions that get auto removed after X turns.
		 */
		BattleActionState_ConditionHeal,
		/**
		 * 2nd action, called once.
		 * Used to execute the algorithm and print the first start line.
		 */
		BattleActionState_Execute,
		/**
		 * 3rd action, called once.
		 * Used to apply the new conditions, play an optional battle animation and sound, and print the second line of a technique.
		 */
		BattleActionState_Apply,
		/**
		* 4th action, called repeatedly.
		* Used for the results, concretely wait a few frames and pop the messages.
		*/
		BattleActionState_ResultPop,
		/**
		 * 5th action, called repeatedly.
		 * Used to push the message results, effects and advances the messages. If it finishes, it calls Death. If not, it calls ResultPop
		 */
		BattleActionState_ResultPush,
		/**
		 * 6th action, called once.
		 * Action treating whether the enemy died or not.
		 */
		BattleActionState_Death,
		/**
		 * 7th action, called once.
		 * It finishes the action and checks whether to repeat it if there is another target to hit.
		 */
		BattleActionState_Finished
	};



public:
	Scene_Battle_Rpg2k3();
	~Scene_Battle_Rpg2k3() override;

	void Update() override;

protected:
	void OnSystem2Ready(FileRequestResult* result);
	void SetupSystem2Graphics();
	void CreateUi() override;
	void InitBattleTest() override;

	void CreateBattleTargetWindow();
	void CreateBattleCommandWindow();

	void UpdateCursors();
	void DrawFloatText(int x, int y, int color, const std::string& text);

	void RefreshCommandWindow();

	void SetState(Scene_Battle::State new_state) override;

	bool CheckWin();
	bool CheckLose();
	bool CheckResultConditions();

	void ProcessActions() override;
	bool ProcessBattleAction(Game_BattleAlgorithm::AlgorithmBase* action);
	void ProcessInput() override;

	void OptionSelected();
	void CommandSelected();
	void AttackSelected() override;
	void SubskillSelected();
	void SpecialSelected();

	void Escape();

	void SelectNextActor();

	void ActionSelectedCallback(Game_Battler* for_battler) override;

	void ShowNotification(const std::string& text);

	std::unique_ptr<Sprite> ally_cursor, enemy_cursor;

	struct FloatText {
		std::shared_ptr<Sprite> sprite;
		int remaining_time = 30;
	};

	std::vector<FloatText> floating_texts;

	int battle_action_wait;
	int battle_action_state;
	bool battle_action_need_event_refresh = true;
	int combo_repeat = 1;
	bool play_reflected_anim = false;

	std::unique_ptr<Window_BattleStatus> enemy_status_window;
	std::unique_ptr<Window_ActorSp> sp_window;

	std::vector<Game_Battler*> targets;

	int select_target_flash_count = 0;

	FileRequestBinding request_id;
	bool battle_action_pending = false;
};

#endif
