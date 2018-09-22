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

#ifndef EP_SCENE_BATTLE_H
#define EP_SCENE_BATTLE_H

// Headers
#include <deque>
#include "rpg_troopmember.h"
#include "rpg_actor.h"
#include "rpg_enemy.h"

#include "battle_animation.h"
#include "drawable.h"
#include "game_actor.h"
#include "game_enemy.h"
#include "scene.h"
#include "spriteset_battle.h"
#include "window_help.h"
#include "window_item.h"
#include "window_skill.h"
#include "window_command.h"
#include "window_battleoption.h"
#include "window_battlecommand.h"
#include "window_battlestatus.h"
#include "window_message.h"

namespace Battle {
class Action;
class SpriteAction;
}

class Game_Battler;

constexpr int option_command_mov = 76;
constexpr int option_command_time = 8;

/**
 * Scene_Battle class.
 * Manages the battles.
 */
class Scene_Battle : public Scene {

public:
	static std::shared_ptr<Scene_Battle> Create();

	~Scene_Battle() override;

	void Start() override;
	void Update() override;

	void TransitionIn() override;
	void TransitionOut() override;
	void DrawBackground() override;

	enum State {
		/** Battle has started (Display encounter message) */
		State_Start,
		/** Menu with Battle, Auto Battle and Escape Options */
		State_SelectOption,
		/** Selects next actor who has to move */
		State_SelectActor,
		/** Auto battle command selected */
		State_AutoBattle,
		/** Menu with abilities of current Actor (e.g. Command, Item, Skill and Defend) */
		State_SelectCommand,
		/** Item selection is active */
		State_SelectItem,
		/** Skill selection menu is active */
		State_SelectSkill,
		/** Player selects enemy target */
		State_SelectEnemyTarget,
		/** Player selects allied target */
		State_SelectAllyTarget,
		/** Battle Running */
		State_Battle,
		/** Battle ended with a victory */
		State_Victory,
		/** Battle ended with a defeat */
		State_Defeat,
		/** Escape command selected */
		State_Escape
	};

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

protected:
	Scene_Battle();

	friend class Battle::SpriteAction;

	virtual void InitBattleTest();

	virtual void CreateUi();

	virtual void ProcessActions() = 0;
	virtual void ProcessInput() = 0;

	virtual void SetState(Scene_Battle::State new_state) = 0;

	void NextTurn(Game_Battler* battler);

	bool IsWindowMoving();

	virtual void EnemySelected();
	virtual void AllySelected();
	virtual void AttackSelected();
	virtual void DefendSelected();
	virtual void ItemSelected();
	virtual void SkillSelected();

	virtual void AssignSkill(const RPG::Skill* skill);

	/**
	 * Executed when selection an action (normal, skill, item, ...) and
	 * (if needed) choosing an attack target was finished. 
	 *
	 * @param for_battler Battler whose action was selected.
	 */
	virtual void ActionSelectedCallback(Game_Battler* for_battler);

	/**
	 * Convenience function, sets the animation state of the target if it has
	 * a valid battler sprite, does nothing otherwise.
	 *
	 * @param target Battler whose anim state is changed
	 * @param new_state new animation state
	 */
	virtual void SetAnimationState(Game_Battler* target, int new_state);

	void UpdateBattlerAction(Game_Battler* battler);
	void CreateEnemyAction(Game_Enemy* enemy, const RPG::EnemyAction* action);
	void CreateEnemyActionBasic(Game_Enemy* enemy, const RPG::EnemyAction* action);
	void CreateEnemyActionSkill(Game_Enemy* enemy, const RPG::EnemyAction* action);

	void RemoveCurrentAction();
	// Variables
	State state;
	State previous_state;
	bool auto_battle;
	int cycle;
	int attack_state;
	int message_timer;
	const RPG::EnemyAction* enemy_action;
	std::deque<std::shared_ptr<Battle::Action> > actions;
	int skill_id;
	int pending_command;

	int actor_index;
	Game_Actor* active_actor;

	/** Displays Fight, Autobattle, Flee */
	std::unique_ptr<Window_Command> options_window;
	/** Displays list of enemies */
	std::unique_ptr<Window_Command> target_window;
	/** Displays Attack, Defense, Magic, Item */
	std::unique_ptr<Window_Command> command_window;
	std::unique_ptr<Window_Item> item_window;
	std::unique_ptr<Window_Skill> skill_window;
	std::unique_ptr<Window_Help> help_window;
	/** Displays allies status */
	std::unique_ptr<Window_BattleStatus> status_window;
	std::unique_ptr<Window_Message> message_window;

	std::deque<Game_Battler*> battle_actions;

	const RPG::Item* skill_item;
};

#endif
