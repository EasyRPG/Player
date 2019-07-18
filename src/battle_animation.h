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
#include "rpg_animation.h"
#include "drawable.h"
#include "sprite_battler.h"

struct FileRequestResult;

// BattleAnimation is responsible for playing an animation.
// It's an abstract class; there are derived classes below
// that can be used, depending on what is targeted.

class BattleAnimation : public Sprite {
public:
	BattleAnimation(const RPG::Animation& anim, bool only_sound = false, int cutoff = -1);

	DrawableType GetType() const override;

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
	bool ShouldOnlySound() const;

protected:
	virtual void SetFlash(int r, int g, int b, int p) = 0;
	virtual bool ShouldScreenFlash() const = 0;
	void DrawAt(int x, int y);
	void ProcessAnimationTiming(const RPG::AnimationTiming& timing);
	void ProcessAnimationFlash(const RPG::AnimationTiming& timing);
	void OnBattleSpriteReady(FileRequestResult* result);
	void OnBattle2SpriteReady(FileRequestResult* result);
	void UpdateScreenFlash();
	void UpdateTargetFlash();
	void UpdateFlashGeneric(int timing_idx, int& r, int& g, int& b, int& p);

	bool should_only_sound;
	const RPG::Animation& animation;
	int frame;
	int num_frames;
	int screen_flash_timing = -1;
	int target_flash_timing = -1;

	FileRequestBinding request_id;
};

// For playing animations against a character on the map.
class BattleAnimationChara : public BattleAnimation {
public:
	BattleAnimationChara(const RPG::Animation& anim, Game_Character& chara);
	~BattleAnimationChara() override;
	void Draw() override;
protected:
	virtual void SetFlash(int r, int g, int b, int p) override;
	bool ShouldScreenFlash() const override;
	Game_Character& character;
};

// For playing animations against a (group of) battlers in battle.
class BattleAnimationBattlers : public BattleAnimation {
public:
	BattleAnimationBattlers(const RPG::Animation& anim, Game_Battler& batt, bool flash = true, bool only_sound = false, int cutoff_frame = -1);
	BattleAnimationBattlers(const RPG::Animation& anim, const std::vector<Game_Battler*>& batts, bool flash = true, bool only_sound = false, int cutoff_frame = -1);
	~BattleAnimationBattlers() override;
	void Draw() override;
protected:
	virtual void SetFlash(int r, int g, int b, int p) override;
	bool ShouldScreenFlash() const override;
	std::vector<Game_Battler*> battlers;
	bool should_flash;
};

// For playing "Show on the entire map" animations.
class BattleAnimationGlobal : public BattleAnimation {
public:
	BattleAnimationGlobal(const RPG::Animation& anim);
	~BattleAnimationGlobal() override;
	void Draw() override;
protected:
	virtual void SetFlash(int r, int g, int b, int p) override;
	bool ShouldScreenFlash() const override;
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


#endif
