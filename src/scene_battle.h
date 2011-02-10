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
#include "window_item.h"
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
		Options,
		Battle,
		AutoBattle,
		Command,
		Target,
		Item,
		Skill
	};

	void SetState(State state);
	int GetActiveActor();

	struct Ally {
		enum AnimState {
			Idle,
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
			anim_state(0),
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
	int cycle;
	int active_enemy;

	Window_Help* help_window;
	Window_BattleCommand* options_window;
	Window_BattleStatus* status_window;
	Window_BattleCommand* command_window;
	Window_Item* item_window;
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

	void Attack();
	void Defend();
	void UseItem();
	void UseSkill();
};

#endif
