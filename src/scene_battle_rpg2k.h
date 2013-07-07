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

#ifndef _SCENE_BATTLE_RPG2K_H_
#define _SCENE_BATTLE_RPG2K_H_

// Headers
#include <deque>
#include "rpg_troopmember.h"
#include "rpg_actor.h"
#include "rpg_enemy.h"
#include "game_actor.h"
#include "game_enemy.h"
#include "scene.h"
#include "background.h"
#include "drawable.h"
#include "zobj.h"
#include "window_help.h"
#include "window_item.h"
#include "window_skill.h"
#include "window_battleoption.h"
#include "window_message.h"

#include "window_command.h"
#include "window_battlestatus_rpg2k.h"
#include "window_battlemessage.h"
#include "battle_battler.h"
#include "battle_animation.h"
#include "battle_interface.h"
#include <boost/scoped_ptr.hpp>

namespace Battle {
class Action;
class SpriteAction;
}

namespace Game_BattleAlgorithm {
	class AlgorithmBase;
}

typedef EASYRPG_SHARED_PTR<Game_BattleAlgorithm::AlgorithmBase> BattleAlgorithmRef;

/**
 * Scene_Battle class.
 * Manages the battles.
 */
class Scene_Battle_Rpg2k : public Scene_Battle {

public:
	Scene_Battle_Rpg2k();
	~Scene_Battle_Rpg2k();

	void Start();
	void Update();
	void Terminate();

	enum BattleActionState {
		/**
		 * Called once at the beginning of the Action.
		 * Used to execute the algorithm to play an optional battle animation.
		 */
		BattleActionState_Start,
		/**
		 * Used to apply the new conditions that were caused.
		 * Called once for each condition.
		 */
		BattleActionState_Result,
		/**
		 * Action execution finished (no function is called here)
		 */
		BattleActionState_Finished
	};

protected:
	void InitBattleTest();
	void CreateWindows();
	void SetState(State new_state);

	bool CheckWin();
	bool CheckLose();
	bool CheckAbort();
	bool CheckFlee();

private:
	/** Displays Fight, Autobattle, Flee */
	boost::scoped_ptr<Window_Command> options_window;
	/** Displays list of enemies */
	boost::scoped_ptr<Window_Command> target_window;
	/** Displays Attack, Defense, Magic, Item */
	boost::scoped_ptr<Window_Command> command_window;
	boost::scoped_ptr<Window_Item> item_window;
	boost::scoped_ptr<Window_Skill> skill_window;
	boost::scoped_ptr<Window_Help> help_window;
	/** Displays allies status */
	boost::scoped_ptr<Window_BattleStatus_Rpg2k> status_window;
	boost::scoped_ptr<Window_BattleMessage> battle_message_window;
	boost::scoped_ptr<Window_Message> message_window;

	void CreateBattleOptionWindow();
	void CreateBattleTargetWindow();
	void CreateBattleCommandWindow();
	void CreateBattleMessageWindow();

	void RefreshCommandWindow();

	void ProcessActions();
	bool ProcessBattleAction(Game_BattleAlgorithm::AlgorithmBase* action);
	void ProcessInput();
	void OptionSelected();
	void CommandSelected();

	void AttackSelected();
	void DefendSelected();
	void ItemSelected();
	void SkillSelected();
	void EnemySelected();

	void SelectNextActor();
	void SelectPreviousActor();

	void CreateExecutionOrder();
	void CreateEnemyActions();

	void NextTurn();

	int actor_index;
	Game_Actor* active_actor;

	bool DisplayMonstersInMessageWindow();

	std::deque<BattleAlgorithmRef> battle_actions;
	std::vector<std::string> battle_result_messages;
	std::vector<std::string>::iterator battle_result_messages_it;
	int battle_action_wait;
	int battle_action_state;
};

#endif
