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

/**
 * Scene_Battle class.
 * Manages the battles.
 */
class Scene_Battle_Rpg2k : public Scene_Battle {
public:
	enum BattleActionState {
		/**
		 * Called once
		 * Flashes enemy sprite and handles states.
		 */
		BattleActionState_Begin,
		/**
		 * Called once
		 * Handles first start message
		 */
		BattleActionState_Usage1,
		/**
		 * Called once
		 * Handles second start message
		 */
		BattleActionState_Usage2,
		/**
		 * Called once
		 * Handles the animation
		 */
		BattleActionState_Animation,
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
		* Used for the results, to push and pop each message.
		*/
		BattleActionState_Results,
		/**
		 * Called once per target if killed.
		 * Action treating whether the enemy died or not.
		 */
		BattleActionState_Death,
		/**
		 * Called once per target.
		 * It finishes the action and checks whether to repeat it if there is another target to hit.
		 */
		BattleActionState_Finished
	};



public:
	Scene_Battle_Rpg2k();
	~Scene_Battle_Rpg2k() override;

	void Update() override;

protected:
	void InitBattleTest() override;
	void SetState(State new_state) override;

	void NextTurn();

	void CreateUi() override;

	void CreateBattleTargetWindow();
	void CreateBattleCommandWindow();

	bool CheckWin();
	bool CheckLose();
	bool CheckResultConditions();

	void RefreshCommandWindow();

	void ProcessActions() override;

	bool ProcessBattleAction(Game_BattleAlgorithm::AlgorithmBase* action);
	void ProcessInput() override;

	/**
	 * Adds a message about the gold received into
	 * Game_Message::texts.
	 *
	 * @param money Number of gold to display.
	 */
	void PushGoldReceivedMessage(int money);

	/**
	 * Adds a message about the experience received into
	 * Game_Message::texts.
	 *
	 * @param exp Number of experience to display.
	 */
	void PushExperienceGainedMessage(int exp);

	/**
	 * Adds messages about the items obtained after the battle
	 * into Game_Message::texts.
	 *
	 * @param drops Vector of item IDs
	 */
	void PushItemRecievedMessages(std::vector<int> drops);

	void OptionSelected();
	void CommandSelected();

	void Escape();

	void SelectNextActor();
	void SelectPreviousActor();

	void CreateExecutionOrder();
	void CreateEnemyActions();

	// Battle Start Handlers
	bool DisplayMonstersInMessageWindow();

	void SetBattleActionState(BattleActionState state);
	void SetBattleActionSubState(int substate, bool reset_index = true);

	/**
	 * Switch to the next action state, resetting the substate.
	 *
	 * @param state the state to change to
	 * @param action the action we're processing
	 * @return the return value of the state handler
	 * @post battle_action_substate is reset to 0
	 */
	bool ProcessNextAction(BattleActionState state, Game_BattleAlgorithm::AlgorithmBase* action);

	/**
	 * Switch to the next action substate
	 *
	 * @param substate the substate to change to
	 * @param action the action we're processing
	 * @param reset_index if true, reset the substate index
	 * @return the return value of the state handler
	 */
	bool ProcessNextSubState(int substate, Game_BattleAlgorithm::AlgorithmBase* action, bool reset_index = true);

	// BattleAction State Machine Handlers
	bool ProcessActionBegin(Game_BattleAlgorithm::AlgorithmBase* action);
	bool ProcessActionUsage1(Game_BattleAlgorithm::AlgorithmBase* action);
	bool ProcessActionUsage2(Game_BattleAlgorithm::AlgorithmBase* action);
	bool ProcessActionAnimation(Game_BattleAlgorithm::AlgorithmBase* action);
	bool ProcessActionExecute(Game_BattleAlgorithm::AlgorithmBase* action);
	bool ProcessActionCritical(Game_BattleAlgorithm::AlgorithmBase* action);
	bool ProcessActionApply(Game_BattleAlgorithm::AlgorithmBase* action);
	bool ProcessActionFailure(Game_BattleAlgorithm::AlgorithmBase* action);
	bool ProcessActionDamage(Game_BattleAlgorithm::AlgorithmBase* action);
	bool ProcessActionResults(Game_BattleAlgorithm::AlgorithmBase* action);
	bool ProcessActionDeath(Game_BattleAlgorithm::AlgorithmBase* action);
	bool ProcessActionFinished(Game_BattleAlgorithm::AlgorithmBase* action);

	void SetWait(int min_wait, int max_wait);
	void SetWaitForUsage(Game_BattleAlgorithm::Type type);
	bool CheckWait();

	std::unique_ptr<Window_BattleMessage> battle_message_window;
	std::vector<std::string> battle_result_messages;
	std::vector<std::string>::iterator battle_result_messages_it;
	bool battle_action_pending = false;
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

	bool encounter_message_first_monster = true;
	bool encounter_message_first_strike = false;
	bool message_box_got_visible = false;
	bool move_screen = false;
	bool first_strike = false;

};

#endif
