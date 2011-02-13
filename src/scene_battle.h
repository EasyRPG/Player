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

	struct Ally {
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

		void CreateSprite();
		void SetAnimState(int state);
		void UpdateAnim(int cycle);

		int ID;
		Game_Actor* game_actor;
		const RPG::Actor* rpg_actor;
		int sprite_frame;
		std::string sprite_file;
		Sprite* sprite;
		int anim_state;
		int speed;
		int gauge;
		bool defending;
		static const int gauge_full = 10000;
	};

	struct Enemy {
		Enemy(const RPG::TroopMember* member, int id);

		void CreateSprite();
		void Transform(int enemy_id);

		int ID;
		Game_Enemy* game_enemy;
		const RPG::TroopMember* member;
		const RPG::Enemy* rpg_enemy;
		Sprite* sprite;
		int fade;
		int speed;
		int gauge;
		bool defending;
		bool charged;
		static const int gauge_full = 10000;
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
		SpriteAction(Ally* ally, int anim_state, int duration) :
			ally(ally), anim_state(anim_state), duration(duration) {}
		bool operator()();
	protected:
		Ally* ally;
		int anim_state;
		int duration;
	};

	class AnimationAction : public Action {
	public:
		AnimationAction(const Sprite* target, const RPG::Animation* animation);
		~AnimationAction();

		bool operator()();

		void Draw(int z_order);
		unsigned long GetId() const;
		int GetZ() const;
		DrawableType GetType() const;

	protected:
		int frame;
		int frames;
		const Sprite* target;
		Animation* animation;
	};

	class MoveAction : public Action {
	public:
		MoveAction(Ally* ally, int anim_state, int x0, int x1, int speed) :
			ally(ally), anim_state(anim_state), x0(x0), x1(x1), speed(speed) {}
		bool operator()();
	protected:
		Ally* ally;
		int anim_state;
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
	std::vector<Action*> actions;
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
	void UseSkillAlly(Ally& ally, const RPG::Skill& skill, Ally* target);
	void UseSkillEnemy(Ally& ally, const RPG::Skill& skill, Enemy* target);

	bool EnemyActionValid(const RPG::EnemyAction& action, Enemy* enemy);
	const RPG::EnemyAction* ChooseEnemyAction(Enemy* enemy);
	void EnemyAction(Enemy* enemy);
	void EnemyActionDone();
	void EnemyAttack(void* target = NULL);

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
