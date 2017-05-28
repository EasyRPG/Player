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
		LoopState_LoopAnimation,
		LoopState_WaitAfterFinish
	};

	/**
	 * Constructor.
	 *
	 * @param character game battler to display
	 */
	Sprite_Battler(Game_Battler* battler);

	~Sprite_Battler() override;

	/**
	 * Updates sprite state.
	 */
	void Update();

	Game_Battler* GetBattler() const;

	void SetBattler(Game_Battler* new_battler);

	void SetAnimationState(int state, LoopState loop = LoopState_LoopAnimation);
	void SetAnimationLoop(LoopState loop);

	/**
	 * Updates the current animation state of the actor depending on his state.
	 */
	void DetectStateChange();

	/**
	 * Returns true when the actor is in it's default state (Depending on
	 * conditions)
	 *
	 * @return Whether state is default
	 */
	bool IsIdling();

	void Flash(int duration) override;
	void Flash(Color color, int duration) override;

	bool GetVisible() const override;
	void SetVisible(bool visible) override;

	int GetWidth() const override;
	int GetHeight() const override;

protected:
	void CreateSprite();
	void DoIdleAnimation();
	void OnMonsterSpriteReady(FileRequestResult* result);
	void OnBattlercharsetReady(FileRequestResult* result, int battler_index);

	std::string sprite_name;
	int hue = 0;
	Game_Battler* battler;
	BitmapRef graphic;
	int anim_state = AnimationState_Idle;
	int cycle = 0;
	std::string sprite_file;
	int sprite_frame = -1;
	int fade_out = 255;
	int flash_counter = 0;
	LoopState loop_state = LoopState_DefaultAnimationAfterFinish;
	bool old_hidden = false;
	std::unique_ptr<BattleAnimation> animation;
	// false when a newly set animation didn't loop once
	bool idling = true;

	FileRequestBinding request_id;
};

#endif
