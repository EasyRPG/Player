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

#ifndef EP_SCENE_BATTLE_RPG2K_H
#define EP_SCENE_BATTLE_RPG2K_H

// Headers
#include "scene_battle.h"
#include "game_enemy.h"

#include "window_command.h"
#include "window_battlemessage.h"
#include "game_battlealgorithm.h"

namespace Game_Message {
class PendingMessage;
};

/**
 * Scene_Battle class.
 * Manages the battles.
 */
class Scene_Battle_Rpg2k : public Scene_Battle {
public:
	/** The return value from a scene action state machine callback */
	enum class SceneActionReturn {
		/** Return from Update() and wait until the next frame */
		eWaitTillNextFrame,
		/** Continue processing this frame, unless CheckWait() etc.. requires us to block */
		eContinueThisFrame,
	};

	/** The return value from a battle action state machine callback */
	enum class BattleActionReturn {
		/** The battle action is not yet finished */
		eContinue,
		/** The battle action is finished */
		eFinished,
	};

	enum BattleActionState {
		/**
		 * Called once
		 * Flashes enemy sprite and handles states.
		 */
		BattleActionState_Begin,
		/**
		 * Called once
		 * Handles the start messages
		 */
		BattleActionState_Usage,
		/**
		 * Called once
		 * Handles the animations
		 */
		BattleActionState_Animation,
		/**
		 * Called once
		 * Handles the animations when skill is reflected (easyrpg extension)
		 */
		BattleActionState_AnimationReflect,
		/**
		 * Called once per target.
		 * Used to execute the algorithm.
		 */
		BattleActionState_Execute,
		/**
		 * Called once per target.
		 * Used to display critical hit message.
		 */
		BattleActionState_Critical,
		/**
		 * Called once per target.
		 * Used to apply the new conditions, play an optional battle animation and sound, and print the second line of a technique.
		 */
		BattleActionState_Apply,
		/**
		 * Called once per target.
		 * Used to handle action failure.
		 */
		BattleActionState_Failure,
		/**
		 * Called once per target.
		 * Used to handle damage.
		 */
		BattleActionState_Damage,
		/**
		* Called repeatedly.
		* Used for hp healing, sp, atk, def, api, agi, to push and pop each message.
		*/
		BattleActionState_Params,
		/**
		* Called repeatedly.
		* Used for states.
		*/
		BattleActionState_States,
		/**
		* Called repeatedly.
		* Used for attribute shifts.
		*/
		BattleActionState_Attributes,
		/**
		 * Called once per target if killed by damage.
		 * Action treating whether the enemy died or not.
		 */
		BattleActionState_DeathDamage,
		/**
		 * Called once per target if killed by a state..
		 * Action treating whether the enemy died or not.
		 */
		BattleActionState_DeathState,
		/**
		 * Called once per target.
		 * It finishes the action and checks whether to repeat it if there is another target to hit.
		 */
		BattleActionState_Finished
	};

public:
	explicit Scene_Battle_Rpg2k(const BattleArgs& args);
	~Scene_Battle_Rpg2k() override;

	void Start() override;
	void vUpdate() override;

protected:
	bool UpdateBattleState();
	void SetState(State new_state) override;

	void NextTurn();

	void CreateUi() override;
	void CreateEnemySprites();

	void CreateBattleTargetWindow();
	void CreateBattleCommandWindow();
	void RefreshTargetWindow();

	bool CheckBattleEndConditions();
	bool RefreshEventsAndCheckBattleEnd();

	void ResetWindows(bool make_invisible);
	void SetCommandWindows(int x);
	void MoveCommandWindows(int x, int frames);
	void RefreshCommandWindow();

	void ActionSelectedCallback(Game_Battler* for_battler) override;

	/**
	 * Adds a message about the gold received into
	 * Game_Message::texts.
	 *
	 * @param money Number of gold to display.
	 */
	void PushGoldReceivedMessage(PendingMessage& pm, int money);

	/**
	 * Adds a message about the experience received into
	 * Game_Message::texts.
	 *
	 * @param exp Number of experience to display.
	 */
	void PushExperienceGainedMessage(PendingMessage& pm, int exp);

	/**
	 * Adds messages about the items obtained after the battle
	 * into Game_Message::texts.
	 *
	 * @param drops Vector of item IDs
	 */
	void PushItemRecievedMessages(PendingMessage& pm, std::vector<int> drops);

	void OptionSelected();
	void CommandSelected();
	void SubskillSelected2k3(int command);
	void SpecialSelected2k3();

	void SelectNextActor(bool auto_battle);
	void SelectPreviousActor();

	void CreateExecutionOrder();
	void CreateEnemyActions();

	void SetSceneActionSubState(int substate);

	// SceneAction State Machine Driver
	SceneActionReturn ProcessSceneAction();

	// SceneAction State Machine Handlers
	SceneActionReturn ProcessSceneActionStart();
	SceneActionReturn ProcessSceneActionFightAutoEscape();
	SceneActionReturn ProcessSceneActionActor();
	SceneActionReturn ProcessSceneActionAutoBattle();
	SceneActionReturn ProcessSceneActionCommand();
	SceneActionReturn ProcessSceneActionItem();
	SceneActionReturn ProcessSceneActionSkill();
	SceneActionReturn ProcessSceneActionEnemyTarget();
	SceneActionReturn ProcessSceneActionAllyTarget();
	SceneActionReturn ProcessSceneActionBattle();
	SceneActionReturn ProcessSceneActionVictory();
	SceneActionReturn ProcessSceneActionDefeat();
	SceneActionReturn ProcessSceneActionEscape();

	bool CheckBattleEndAndScheduleEvents();

	void SetBattleActionState(BattleActionState state);
	void SetBattleActionSubState(int substate, bool reset_index = true);

	// BattleAction State Machine Driver
	BattleActionReturn ProcessBattleAction(Game_BattleAlgorithm::AlgorithmBase* action);

	// BattleAction State Machine Handlers
	BattleActionReturn ProcessBattleActionBegin(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionUsage(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionAnimation(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionAnimationReflect(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionAnimationImpl(Game_BattleAlgorithm::AlgorithmBase* action, bool reflect);
	BattleActionReturn ProcessBattleActionExecute(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionCritical(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionApply(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionFailure(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionDamage(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionParamEffects(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionStateEffects(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionAttributeEffects(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionFinished(Game_BattleAlgorithm::AlgorithmBase* action);

	std::vector<std::string> GetBattleCommandNames2k3(const Game_Actor* actor);

	void ProcessBattleActionDeath(Game_BattleAlgorithm::AlgorithmBase* action);

	void SetWait(int min_wait, int max_wait);
	void SetWaitForUsage(Game_BattleAlgorithm::Type type, int anim_frames);
	bool CheckWait();

	std::unique_ptr<Window_BattleMessage> battle_message_window;
	std::vector<std::string> battle_result_messages;
	std::vector<std::string>::iterator battle_result_messages_it;
	std::shared_ptr<Game_BattleAlgorithm::AlgorithmBase> pending_battle_action = {};
	int actor_index = 0;
	int scene_action_substate = 0;
	int battle_action_state = BattleActionState_Begin;
	int battle_action_substate = 0;
	int battle_action_start_index = 0;
	int battle_action_results_index = 0;
	int battle_action_dmg_index = 0;
	std::string pending_message;
	int battle_action_substate_index = 0;

	int select_target_flash_count = 0;

	int battle_action_wait = 0;
	int battle_action_min_wait = 0;

	bool message_box_got_visible = false;
	bool resume_from_debug_scene = false;
};

#endif
