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

#ifndef _SPRITE_BATTLER_H_
#define _SPRITE_BATTLER_H_

// Headers
#include "sprite.h"
#include "game_battler.h"
#include <boost/scoped_ptr.hpp>
#include "async_handler.h"

class BattleAnimation;

/**
 * Sprite_Battler class, used for battle sprites
 */
class Sprite_Battler : public Sprite {
public:
	enum AnimationState {
		AnimationState_Idle = 1,
		AnimationState_RightHand,
		AnimationState_LeftHand,
		AnimationState_SkillUse,
		AnimationState_Dead,
		AnimationState_Damage,
		AnimationState_BadStatus,
		AnimationState_Defending,
		AnimationState_WalkingLeft,
		AnimationState_WalkingRight,
		AnimationState_Victory,
		AnimationState_Item
	};

	enum LoopState {
		LoopState_DefaultAnimationAfterFinish,
		LoopState_LoopAnimation
	};

	/**
	 * Constructor.
	 *
	 * @param character game battler to display
	 */
	Sprite_Battler(Game_Battler* battler);

	~Sprite_Battler();

	/**
	 * Updates sprite state.
	 */
	void Update();

	Game_Battler* GetBattler() const;

	void SetBattler(Game_Battler* new_battler);

	void SetAnimationState(int state, LoopState loop = LoopState_LoopAnimation);

	/**
	 * Returns true when the actor is in it's default state (Depending on
	 * conditions)
	 *
	 * @return Whether state is default
	 */
	bool IsIdling();

protected:
	void CreateSprite();
	void OnMonsterSpriteReady(FileRequestResult* result);
	void OnBattlercharsetReady(FileRequestResult* result, int battler_index);

	std::string sprite_name;
	int hue;

	Game_Battler* battler;
	BitmapRef graphic;
	int anim_state;
	int cycle;
	std::string sprite_file;
	int sprite_frame;
	int fade_out;
	int flash_counter;
	LoopState loop_state;
	bool old_hidden;
	boost::scoped_ptr<BattleAnimation> animation;
	// false when a newly set animation didn't loop once
	bool idling;

	FileRequestBinding request_id;
};

#endif
