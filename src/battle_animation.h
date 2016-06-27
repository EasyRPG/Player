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

#ifndef _BATTLE_ANIMATION_H_
#define _BATTLE_ANIMATION_H_

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
	BattleAnimation(const RPG::Animation& anim);

	DrawableType GetType() const override;

	void Update();
	int GetFrame() const;
	int GetFrames() const;
	void SetFrame(int);
	bool IsDone() const;

protected:
	virtual void SetFlash(Color c) = 0;
	virtual bool ShouldScreenFlash() const = 0;
	void DrawAt(int x, int y);
	void RunTimedSfx();
	void ProcessAnimationTiming(const RPG::AnimationTiming& timing);
	void OnBattleSpriteReady(FileRequestResult* result);
	void OnBattle2SpriteReady(FileRequestResult* result);

	const RPG::Animation& animation;
	int frame;
	bool frame_update;
	bool large;

	FileRequestBinding request_id;
};

// For playing animations against a character on the map.
class BattleAnimationChara : public BattleAnimation {
public:
	BattleAnimationChara(const RPG::Animation& anim, Game_Character& chara);
	~BattleAnimationChara() override;
	void Draw() override;
protected:
	void SetFlash(Color c) override;
	bool ShouldScreenFlash() const override;
	Game_Character& character;
};

// For playing animations against a (group of) battlers in battle.
class BattleAnimationBattlers : public BattleAnimation {
public:
	BattleAnimationBattlers(const RPG::Animation& anim, Game_Battler& batt, bool flash = true);
	BattleAnimationBattlers(const RPG::Animation& anim, const std::vector<Game_Battler*>& batts, bool flash = true);
	~BattleAnimationBattlers() override;
	void Draw() override;
protected:
	void SetFlash(Color c) override;
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
	void SetFlash(Color c) override;
	bool ShouldScreenFlash() const override;
};

#endif
