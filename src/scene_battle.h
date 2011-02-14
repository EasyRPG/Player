/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

#ifndef _SCENE_BATTLE_H_
#define _SCENE_BATTLE_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
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
#include "window_battleitem.h"
#include "window_battleskill.h"
#include "window_battleoption.h"
#include "window_battlecommand.h"
#include "window_battlestatus.h"
#include "battle_battler.h"
#include "battle_animation.h"

namespace Battle {
class Action;
class SpriteAction;
}

////////////////////////////////////////////////////////////
/// Scene_Battle class.
/// Manages the battles.
////////////////////////////////////////////////////////////
class Scene_Battle : public Scene {

public:
	Scene_Battle();
	~Scene_Battle();

	void Start();
	void Update();
	void Terminate();

	enum State {
		State_Options,
		State_Battle,
		State_AutoBattle,
		State_Command,
		State_Item,
		State_Skill,
		State_TargetEnemy,
		State_TargetAlly,
		State_AllyAction,
		State_EnemyAction,
		State_Victory,
		State_Defeat
	};

	struct FloatText {
		FloatText(int x, int y, int color, const std::string& text, int duration);
		~FloatText();
		int duration;
		Sprite* sprite;
	};

private:
	friend class Battle::SpriteAction;

	static const int turn_length = 333; // frames

	State state;
	State target_state;
	bool auto_battle;
	int cycle;
	int active_enemy;
	int active_ally;
	int target_enemy;
	int target_ally;
	int action_timer;
	int attack_state;
	int turn_fragments;
	int message_timer;
	const RPG::EnemyAction* enemy_action;
	std::deque<Battle::Action*> actions;
	int skill_id;

	Window_Help* help_window;
	Window_BattleOption* options_window;
	Window_BattleStatus* status_window;
	Window_BattleCommand* command_window;
	Window_BattleItem* item_window;
	Window_BattleSkill* skill_window;
	Background* background;

	const RPG::Troop* troop;
	std::vector<Battle::Ally> allies;
	std::vector<Battle::Enemy> enemies;

	Sprite *ally_cursor;
	Sprite *enemy_cursor;
	std::vector<FloatText*> floaters;

	void AlliesCentroid(int& x, int& y);
	void EnemiesCentroid(int& x, int& y);

	void CreateSprites();
	void CreateCursors();
	void CreateWindows();

	void SetState(State state);
	void Message(const std::string& msg, bool pause = true);
	void Floater(const Sprite* ref, int color, const std::string& text, int duration);
	void Floater(const Sprite* ref, int color, int value, int duration);
	void SetAnimState(Battle::Ally& ally, int state);
	void UpdateAnimState(Battle::Ally& ally, int default_state = Battle::Ally::Idle);
	void Restart(Battle::Ally& ally, int state = Battle::Ally::Idle);
	void Restart();

	void Command();
	void Escape();
	void Special();
	void Attack();
	void Defend();
	void Item();
	void Skill();
	void ItemSkill(const RPG::Item& item);
	void Skill(const RPG::Skill& skill);
	void TargetDone();
	void BeginAttack();
	void BeginItem();
	void BeginSkill();
	void DoAttack();
	void DoItem();
	void DoSkill();

	int SkillAnimation(const RPG::Skill& skill, const Battle::Ally& ally);

	void EnemyAction(Battle::Enemy* enemy);
	void EnemyActionBasic();
	void EnemyActionSkill();

	void EnemyAttack(void* target = NULL);
	void EnemyDefend();
	void EnemyObserve();
	void EnemyCharge();
	void EnemyDestruct();
	void EnemySkill();
	void EnemyTransform();
	void EnemyEscape();
	void EnemyActionDone();

	void ProcessActions();
	void ProcessInput();
	void ChooseEnemy();
	void DoAuto();
	void UpdateCursors();
	void UpdateAttack();
	void UpdateSprites();
	void UpdateFloaters();

	int GetActiveActor();
	int GetTargetActor();
	bool HaveCorpse();
	void CheckWin();
	void CheckLose();

	// battle_algorithms.cpp

	void AttackEnemy(Battle::Ally& ally, Battle::Enemy& enemy);
	void UseItem(Battle::Ally& ally, const RPG::Item& item, Battle::Ally* ally_target);
	void UseItemAlly(Battle::Ally& ally, const RPG::Item& item, Battle::Ally* target);
	void UseSkill(Battle::Ally& ally, const RPG::Skill& skill);
	void UseSkillAlly(Battle::Battler& ally, const RPG::Skill& skill, Battle::Battler* target);
	void UseSkillEnemy(Battle::Battler& ally, const RPG::Skill& skill, Battle::Battler* target);

	bool EnemyActionValid(const RPG::EnemyAction& action, Battle::Enemy* enemy);
	const RPG::EnemyAction* ChooseEnemyAction(Battle::Enemy* enemy);
	void EnemyAttackAlly(Battle::Enemy& enemy, Battle::Ally& ally);
	void EnemySkill(Battle::Enemy& enemy, const RPG::Skill& skill);
	void EnemySkillAlly(Battle::Enemy& ally, const RPG::Skill& skill, Battle::Ally* target);
	void EnemySkillEnemy(Battle::Enemy& ally, const RPG::Skill& skill, Battle::Enemy* target);
};

#endif
