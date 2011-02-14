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

#ifndef _BATTLE_BATTLER_H_
#define _BATTLE_BATTLER_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "rpg_troopmember.h"
#include "rpg_actor.h"
#include "rpg_enemy.h"
#include "game_actor.h"
#include "game_enemy.h"
#include "sprite.h"

namespace Battle {

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

}

#endif

