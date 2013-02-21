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
#include "battle_interface.h"
#include <boost/scoped_ptr.hpp>

namespace Battle {
class Action;
class SpriteAction;
}

////////////////////////////////////////////////////////////
/// Scene_Battle class.
/// Manages the battles.
////////////////////////////////////////////////////////////
class Scene_Battle : public Scene, public Battle_Interface {

public:
	Scene_Battle();
	~Scene_Battle();

	void Start();
	void Update();

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
		int duration;
		boost::scoped_ptr<Sprite> sprite;
	};

private:
	friend class Battle::SpriteAction;

	State state;
	State target_state;
	bool auto_battle;
	int cycle;
	int attack_state;
	int message_timer;
	const RPG::EnemyAction* enemy_action;
	std::deque<EASYRPG_SHARED_PTR<Battle::Action> > actions;
	int skill_id;
	int pending_command;

	boost::scoped_ptr<Window_Help> help_window;
	boost::scoped_ptr<Window_BattleOption> options_window;
	boost::scoped_ptr<Window_BattleStatus> status_window;
	boost::scoped_ptr<Window_BattleCommand> command_window;
	boost::scoped_ptr<Window_BattleItem> item_window;
	boost::scoped_ptr<Window_BattleSkill> skill_window;
	boost::scoped_ptr<Background> background;

	EASYRPG_SHARED_PTR<BattleAnimation> animation;
	std::deque<EASYRPG_SHARED_PTR<BattleAnimation> > animations;

	std::vector<EASYRPG_SHARED_PTR<FloatText> > floaters;

	boost::scoped_ptr<Sprite> ally_cursor, enemy_cursor;

	void CreateCursors();
	void CreateWindows();

	void Message(const std::string& msg, bool pause = true);
	void Floater(const Sprite* ref, int color, const std::string& text, int duration);
	void Floater(const Sprite* ref, int color, int value, int duration);
	void ShowAnimation(int animation_id, bool allies, Battle::Ally* ally, Battle::Enemy* enemy, bool wait);
	void UpdateAnimations();
	bool IsAnimationWaiting();

	void SetState(State state);
	void SetAnimState(Battle::Ally& ally, int state);
	void UpdateAnimState();
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

	void EnemyAction();
	void EnemyActionBasic();
	void EnemyActionSkill();

	static void EnemyActionDone(void* param);

	void ProcessActions();
	void ProcessInput();
	void ChooseEnemy();
	void DoAuto();

	void UpdateBackground();
	void UpdateCursors();
	void UpdateAttack();
	void UpdateSprites();
	void UpdateFloaters();

	void CheckWin();
	void CheckLose();
	void CheckAbort();
	void CheckFlee();

	// battle_algorithms.cpp

	void AttackEnemy(Battle::Ally& ally, Battle::Enemy& enemy);
	void UseItem(Battle::Ally& ally, const RPG::Item& item);
	void UseItemAlly(Battle::Ally& ally, const RPG::Item& item, Battle::Ally& target);
	void UseSkill(Battle::Ally& ally, const RPG::Skill& skill);
	void UseSkillAlly(Battle::Battler& ally, const RPG::Skill& skill, Battle::Battler& target);
	void UseSkillEnemy(Battle::Battler& ally, const RPG::Skill& skill, Battle::Battler& target);

	bool EnemyActionValid(const RPG::EnemyAction& action, Battle::Enemy& enemy);
	const RPG::EnemyAction* ChooseEnemyAction(Battle::Enemy& enemy);
	void EnemyAttackAlly(Battle::Enemy& enemy, Battle::Ally& ally);
	void EnemySkill(Battle::Enemy& enemy, const RPG::Skill& skill);
};

#endif
