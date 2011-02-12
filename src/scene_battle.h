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
		State_TargetAlly
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

		Ally(Game_Actor* game_actor);

		void CreateSprite();

		Game_Actor* game_actor;
		const RPG::Actor* rpg_actor;
		Sprite* sprite;
		int anim_state;
		int gauge;
		static const int gauge_full = 500;
	};

	struct Enemy {
		Enemy(const RPG::TroopMember* member);

		void CreateSprite();

		Game_Enemy* game_enemy;
		const RPG::TroopMember* member;
		const RPG::Enemy* rpg_enemy;
		Sprite* sprite;
		int fade;
	};

	struct FloatText {
		FloatText(int x, int y, int color, const std::string& text, int duration);
		~FloatText();
		int duration;
		Sprite* sprite;
	};

private:
	State state;
	State target_state;
	int cycle;
	int active_ally;
	int target_enemy;
	int target_ally;

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
	void Message(const std::string& msg);
	void Floater(const Sprite* ref, int color, const std::string& text, int duration);
	void Floater(const Sprite* ref, int color, int value, int duration);
	void UpdateAnimState(Ally& ally, int default_state = Ally::Idle);
	void Restart(Ally& ally, int state = Ally::Idle);

	void Command();
	void Escape();
	void Special();
	void Attack();
	void Defend();
	void UseItem();
	void UseSkill();
	void UseItemSkill(Ally& ally, const RPG::Item& item);
	void UseSkill(Ally& ally, const RPG::Skill& skill);
	void TargetDone();
	void UseItemDone();
	void UseSkillDone();
	void UseItem(Ally& ally, const RPG::Item& item, Ally* target_ally);
	void UseSkill(Ally& ally, const RPG::Skill& skill,
				  Ally* target_ally, Enemy* target_enemy);
	void UseItemAlly(Ally& ally, const RPG::Item& item, Ally* target);
	void UseSkillAlly(Ally& ally, const RPG::Skill& skill, Ally* target);
	void UseSkillEnemy(Ally& ally, const RPG::Skill& skill, Enemy* target);
	void ProcessInput();
	void UpdateCursors();
	void UpdateSprites();
	void UpdateFloaters();

	int GetActiveActor();
	int GetTargetActor();
	bool HaveCorpse();
	void CheckWin();
	void CheckLose();
};

#endif
