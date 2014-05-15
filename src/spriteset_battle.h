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

#ifndef _SPRITESET_BATTLE_H_
#define _SPRITESET_BATTLE_H_

// Headers
#include "tilemap.h"
#include "background.h"
#include "sprite_battler.h"
#include "sprite_character.h"
#include <boost/scoped_ptr.hpp>

class Game_Battler;
/**
 * Spriteset_Battle class.
 */
class Spriteset_Battle {
public:
	Spriteset_Battle();

	void Update();
	Sprite_Battler* FindBattler(const Game_Battler* battler);

protected:
	boost::scoped_ptr<Background> background;
	std::vector<EASYRPG_SHARED_PTR<Sprite_Battler> > sprites;
	/*std::vector<Sprite_Picture*> picture_sprites;
	Sprite_Timer* timer_sprite;*/
};

#endif
