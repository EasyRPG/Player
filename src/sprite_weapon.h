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

#ifndef EP_SPRITE_WEAPON_H
#define EP_SPRITE_WEAPON_H

// Headers
#include <cstdint>
#include "sprite_battler.h"
#include "async_handler.h"

class Game_Actor;

/**
 * Sprite_Weapon class, used for weapon battle sprites
 */
class Sprite_Weapon : public Sprite {
public:
	/**
	 * Constructor.
	 *
	 * @param actor game battler to display
	 */
	Sprite_Weapon(Game_Actor* actor);

	~Sprite_Weapon() override;

	/**
	 * Updates sprite state.
	 */
	void Update();

	void SetWeaponAnimation(int nweapon_animation_id);

	void SetRanged(bool nranged);

	void StartAttack(bool secondary_weapon);

	void StopAttack();

	void Draw(Bitmap& dst) override;

protected:
	void CreateSprite();
	void OnBattleWeaponReady(FileRequestResult* result, int32_t weapon_index);

	Game_Actor* battler;

	BitmapRef graphic;
	int weapon_animation_id = 0;
	bool ranged = false;
	bool attacking = false;
	int cycle = 0;
	int sprite_frame = -1;

	FileRequestBinding request_id;
};


#endif
