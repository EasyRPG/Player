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

#ifndef EP_SPRITE_BATTLER_H
#define EP_SPRITE_BATTLER_H

// Headers
#include "sprite.h"
#include "async_handler.h"

class BattleAnimation;
class Game_Battler;

/**
 * Sprite_Battler class, used for battle sprites
 */
class Sprite_Battler : public Sprite {
public:
	enum FixedFacing {
		Disabled = 0,
		AlwaysFlipped = 1,
		NeverFlipped = 2,
	};
	/**
	 * Constructor.
	 *
	 * @param battler game battler to display
	 * @param battle_index battle index for Z ordering
	 */
	Sprite_Battler(Game_Battler* battler, int battle_index);

	~Sprite_Battler() override;

	Game_Battler* GetBattler() const;

	void SetBattler(Game_Battler* new_battler);

	void ResetFixedFacingDirection();

	FixedFacing GetFixedFacing() const;

	/**
	 * Sets the sprite flip when fixed facing direction is enabled. Otherwise does nothing..
	 */
	void SetFixedFlipX();

	/**
	 * Recompute the Z value for the sprite from it's Y coordinate.
	 */
	virtual void ResetZ();

protected:
	Game_Battler* battler = nullptr;
	int battle_index = 0;
	FixedFacing fixed_facing = Disabled;
};

inline Game_Battler* Sprite_Battler::GetBattler() const {
	return battler;
}

inline void Sprite_Battler::SetBattler(Game_Battler* new_battler) {
	battler = new_battler;
}

inline void Sprite_Battler::ResetFixedFacingDirection() {
	fixed_facing = Disabled;
}

inline void Sprite_Battler::SetFixedFlipX() {
	switch (fixed_facing) {
		case AlwaysFlipped:
			SetFlipX(true);
			break;
		case NeverFlipped:
			SetFlipX(false);
			break;
		default:
			break;
	}
}

inline Sprite_Battler::FixedFacing Sprite_Battler::GetFixedFacing() const {
	return fixed_facing;
}

#endif
