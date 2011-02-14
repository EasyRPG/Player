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

	struct Battler {
		enum Side {
			Side_Ally,
			Side_Enemy
		};

		int side;
		int ID;
		Sprite* sprite;
		int gauge;
		int speed;
		static const int gauge_full = 10000;

		Battler(int side, int id) :
			side(side), ID(id), sprite(NULL), gauge(0) {}

		virtual Game_Battler* GetActor() = 0;
		virtual void CreateSprite() = 0;
	};

	struct Ally : public Battler {
		enum AnimationState {
			Idle = 1,
			RightHand,
			LeftHand,
			SkillUse,
			Dead,
			Damage,
			BadStatus,
			Defending,
			WalkingLeft,
			WalkingRight,
			Victory,
			Item
		};

		Ally(Game_Actor* game_actor, int id);

		Game_Battler* GetActor() { return game_actor; }
		const Game_Battler* GetActor() const { return game_actor; }
		void CreateSprite();
		void SetAnimState(int state);
		void UpdateAnim(int cycle);

		Game_Actor* game_actor;
		const RPG::Actor* rpg_actor;
		int sprite_frame;
		std::string sprite_file;
		int anim_state;
		bool defending;
	};

	struct Enemy : public Battler {
		Enemy(const RPG::TroopMember* member, int id);

		Game_Battler* GetActor() { return game_enemy; }
		void CreateSprite();
		void Transform(int enemy_id);

		Game_Enemy* game_enemy;
		const RPG::TroopMember* member;
		const RPG::Enemy* rpg_enemy;
		int fade;
		bool defending;
		bool charged;
		bool escaped;
	};

	struct FloatText {
		FloatText(int x, int y, int color, const std::string& text, int duration);
		~FloatText();
		int duration;
		Sprite* sprite;
	};

	class Animation : public Drawable {
	public:
		Animation(int x, int y, const RPG::Animation* animation);
		~Animation();

		void Draw(int z_order);
		unsigned long GetId() const;
		int GetZ() const;
		DrawableType GetType() const;

		void Setup();
		void Update(int frame);

	protected:
		int x;
		int y;
		const RPG::Animation* animation;
		int frame;
		bool initialized;
		bool large;
		BitmapScreen* screen;
		unsigned long ID;
		ZObj* zobj;
	};

	class Action {
	public:
		virtual bool operator()() = 0;
		virtual ~Action();
	};

	class WaitAction : public Action {
	public:
		WaitAction(int duration) :
			duration(duration) {}
		bool operator()();
	protected:
		int duration;
	};

	class SpriteAction : public Action {
	public:
		SpriteAction(Ally* ally, int anim_state) :
			ally(ally), anim_state(anim_state) {}
		bool operator()();
	protected:
		Ally* ally;
		int anim_state;
	};

	class AnimationAction : public Action {
	public:
		AnimationAction(const Sprite* target, const RPG::Animation* animation);
		AnimationAction(int x, int y, const RPG::Animation* animation);
		~AnimationAction();

		bool operator()();

	protected:
		int frame;
		int frames;
		Animation* animation;
	};

	class MoveAction : public Action {
	public:
		MoveAction(Sprite* sprite, int x0, int x1, int speed) :
			sprite(sprite), x0(x0), x1(x1), speed(speed) {}
		bool operator()();
	protected:
		Sprite* sprite;
		int x0;
		int x1;
		int speed;
	};

	class CommandAction : public Action {
	public:
		CommandAction(void (Scene_Battle::*func)()) :
			func(func) {}
		bool operator()();
	protected:
		void (Scene_Battle::*func)();
	};

	class CommandAction1 : public Action {
	public:
		CommandAction1(void (Scene_Battle::*func)(void*), void* param) :
			func(func), param(param) {}
		bool operator()();
	protected:
		void (Scene_Battle::*func)(void*);
		void* param;
	};

private:
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
	std::deque<Action*> actions;
	int skill_id;

	Window_Help* help_window;
	Window_BattleOption* options_window;
	Window_BattleStatus* status_window;
	Window_BattleCommand* command_window;
	Window_BattleItem* item_window;
	Window_BattleSkill* skill_window;
	Background* background;

	const RPG::Troop* troop;
	std::vector<Ally> allies;
	std::vector<Enemy> enemies;

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
	void SetAnimState(Ally& ally, int state);
	void UpdateAnimState(Ally& ally, int default_state = Ally::Idle);
	void Restart(Ally& ally, int state = Ally::Idle);
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
	void UseItem();
	void UseSkill();

	void UseItemAlly(Ally& ally, const RPG::Item& item, Ally* target);
	void UseSkillAlly(Battler& ally, const RPG::Skill& skill, Battler* target);
	void UseSkillEnemy(Battler& ally, const RPG::Skill& skill, Battler* target);
	int SkillAnimation(const RPG::Skill& skill, const Ally& ally);

	bool EnemyActionValid(const RPG::EnemyAction& action, Enemy* enemy);
	const RPG::EnemyAction* ChooseEnemyAction(Enemy* enemy);
	void EnemyAction(Enemy* enemy);
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
	void EnemySkillAlly(Enemy& ally, const RPG::Skill& skill, Ally* target);
	void EnemySkillEnemy(Enemy& ally, const RPG::Skill& skill, Enemy* target);
	Sprite* EnemySkillTarget();

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
};

#endif
