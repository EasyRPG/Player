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

#ifndef EP_BATTLE_ANIMATION_H
#define EP_BATTLE_ANIMATION_H

// Headers
#include "game_battler.h"
#include "game_character.h"
#include "system.h"
#include <lcf/rpg/animation.h>
#include "drawable.h"
#include "sprite_battler.h"

struct FileRequestResult;

// BattleAnimation is responsible for playing an animation.
// It's an abstract class; there are derived classes below
// that can be used, depending on what is targeted.

class BattleAnimation : public Sprite {
public:
	/** Update the animation to the next animation **/
	void Update();

	/** @return the current timing frame (2x the number of frames in the underlying animation **/
	int GetFrame() const;

	/** @return the animation frame that is currently being displayed **/
	int GetRealFrame() const;

	/** @return the number of frames the animation will play for **/
	int GetFrames() const;

	/** @return the number of frames in the underlying animation **/
	int GetRealFrames() const;

	/**
	 * Set the current running frame
	 *
	 * @param frame the frame to set.
	 **/
	void SetFrame(int frame);

	/** @return true if the animation has finished **/
	bool IsDone() const;

	/** @return true if the animation only plays audio and doesn't display **/
	bool IsOnlySound() const;

	/**
	 * @return the animation cell width
	 */
	int GetAnimationCellWidth() const;

	/**
	 * @return the animation cell height
	 */
	int GetAnimationCellHeight() const;

	/**
	 * Set if the animation is inverted
	 *
	 * @param inverted if the animation is inverted
	 **/
	void SetInvert(bool inverted);

protected:
	BattleAnimation(const lcf::rpg::Animation& anim, bool only_sound = false, int cutoff = -1);

	virtual void FlashTargets(int r, int g, int b, int p) = 0;
	virtual void ShakeTargets(int str, int spd, int time) = 0;
	void DrawAt(Bitmap& dst, int x, int y);
	virtual void ProcessAnimationTiming(const lcf::rpg::AnimationTiming& timing);
	virtual void ProcessAnimationFlash(const lcf::rpg::AnimationTiming& timing);
	void OnBattleSpriteReady(FileRequestResult* result);
	void OnBattle2SpriteReady(FileRequestResult* result);
	virtual void UpdateScreenFlash();
	virtual void UpdateTargetFlash();
	void UpdateFlashGeneric(int timing_idx, int& r, int& g, int& b, int& p);

	const lcf::rpg::Animation& animation;
	int frame = 0;
	int num_frames = 0;
	int screen_flash_timing = -1;
	int target_flash_timing = -1;

	FileRequestBinding request_id;
	bool only_sound = false;
	bool invert = false;
};

// For playing animations on the map.
class BattleAnimationMap : public BattleAnimation {
public:
	BattleAnimationMap(const lcf::rpg::Animation& anim, Game_Character& target, bool global);
	void Draw(Bitmap& dst) override;
protected:
	void FlashTargets(int r, int g, int b, int p) override;
	void ShakeTargets(int str, int spd, int time) override;
	void DrawSingle(Bitmap& dst);
	void DrawGlobal(Bitmap& dst);

	Game_Character& target;
	bool global = false;
};

// For playing animations against a (group of) battlers in battle.
class BattleAnimationBattle : public BattleAnimation {
public:
	BattleAnimationBattle(const lcf::rpg::Animation& anim, std::vector<Game_Battler*> battlers, bool only_sound = false, int cutoff_frame = -1, bool set_invert = false);
	void Draw(Bitmap& dst) override;
protected:
	void FlashTargets(int r, int g, int b, int p) override;
	void ShakeTargets(int str, int spd, int time) override;
	std::vector<Game_Battler*> battlers;
};

class BattleAnimationBattler : public BattleAnimation {
public:
	BattleAnimationBattler(const lcf::rpg::Animation& anim, std::vector<Game_Battler*> battlers, bool only_sound = false, int cutoff_frame = -1, bool set_invert = false);
	void Draw(Bitmap& dst) override;
protected:
	void FlashTargets(int r, int g, int b, int p) override;
	void ShakeTargets(int str, int spd, int time) override;
	void ProcessAnimationTiming(const lcf::rpg::AnimationTiming& timing) override;
	void ProcessAnimationFlash(const lcf::rpg::AnimationTiming& timing) override;
	void UpdateScreenFlash() override;
	void UpdateTargetFlash() override;
	std::vector<Game_Battler*> battlers;
};

inline int BattleAnimation::GetFrame() const {
	return frame;
}

inline int BattleAnimation::GetRealFrame() const {
	return GetFrame() / 2;
}

inline int BattleAnimation::GetFrames() const {
	return num_frames;
}

inline int BattleAnimation::GetRealFrames() const {
	return animation.frames.size();
}

inline bool BattleAnimation::IsDone() const {
	return GetFrame() >= GetFrames();
}

inline bool BattleAnimation::IsOnlySound() const {
	return only_sound;
}

inline int BattleAnimation::GetAnimationCellWidth() const {
	return (animation.large ? 128 : 96);
}

inline int BattleAnimation::GetAnimationCellHeight() const {
	return (animation.large ? 128 : 96);
}

#endif
