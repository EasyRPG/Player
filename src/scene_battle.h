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
#include "scene.h"
#include "background.h"
#include "window_help.h"
#include "window_battleitem.h"
#include "window_skill.h"
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

	enum Target {
		Target_Enemy,
		Target_Enemies,
		Target_Self,
		Target_Ally,
		Target_Allies
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

		Ally(Game_Actor* game_actor) :
			game_actor(game_actor),
			rpg_actor(&Data::actors[game_actor->GetId() - 1]),
			sprite(NULL),
			anim_state(Idle),
			gauge(0) {}

		void CreateSprite();

		Game_Actor* game_actor;
		const RPG::Actor* rpg_actor;
		Sprite* sprite;
		int anim_state;
		int gauge;
		static const int gauge_full = 500;
	};

	struct Enemy {
		Enemy(const RPG::TroopMember* member) :
			member(member),
			enemy(&Data::enemies[member->ID - 1]),
			sprite(NULL),
			visible(!member->invisible) {}

		void CreateSprite();

		const RPG::TroopMember* member;
		const RPG::Enemy* enemy;
		Sprite* sprite;
		bool visible;
	};

private:
	State state;
	State target_state;
	int cycle;
	int active_ally;
	int target_enemy;
	int target_ally;

	Window_Help* help_window;
	Window_BattleCommand* options_window;
	Window_BattleStatus* status_window;
	Window_BattleCommand* command_window;
	Window_BattleItem* item_window;
	Window_Skill* skill_window;
	Background* background;

	const RPG::Troop* troop;
	std::vector<Ally> allies;
	std::vector<Enemy> enemies;

	Sprite *ally_cursor;
	Sprite *enemy_cursor;

	void CreateSprites();
	void CreateCursors();
	void CreateWindows();

	void SetState(State state);
	void Message(const std::string& msg);
	void UpdateAnimState(Ally& ally, int default_state = Ally::Idle);
	void Restart(Ally& ally, int state = Ally::Idle);

	void Attack();
	void Defend();
	void UseItem();
	void UseSkill();
	void UseItemSkill(Ally& ally, const RPG::Item& item);
	void UseSkill(Ally& ally, const RPG::Skill& skill);
	void TargetDone();
	void UseItemDone();
	void UseSkillDone();
	void UseItem(Ally& ally, const RPG::Item& item,
				 Target target, Ally* target_ally = NULL, Enemy* target_enemy = NULL);
	void UseSkill(Ally& ally, const RPG::Skill& skill,
				  Target target, Ally* target_ally = NULL, Enemy* target_enemy = NULL);


	int GetActiveActor();
	bool HaveCorpse();
};

#endif
