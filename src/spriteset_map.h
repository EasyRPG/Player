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

#ifndef _SPRITESET_MAP_H_
#define _SPRITESET_MAP_H_

// Headers
#include <boost/scoped_ptr.hpp>
#include "sprite_airshipshadow.h"
#include "sprite_timer.h"
#include "system.h"
#include "tilemap.h"
#include "plane.h"
#include <string>

class Sprite_Character;
class Game_Character;

/**
 * Spriteset_Map class.
 */
class Spriteset_Map {
public:
	Spriteset_Map();

	void Update();

	/**
	 * Finds the sprite for a specific character.
	 */
	Sprite_Character* FindCharacter(Game_Character* character) const;

	/**
	 * Notifies that the map's chipset has changed.
	 */
	void ChipsetUpdated();

	/**
	 * Notifies that the System graphic has changed.
	 */
	void SystemGraphicUpdated();

	/**
	 * Substitutes tile in lower layer.
	 */
	void SubstituteDown(int old_id, int new_id);
	/**
	 * Substitutes tile in upper layer.
	 */
	void SubstituteUp(int old_id, int new_id);

protected:
	Tilemap tilemap;
	Plane panorama;
	std::string panorama_name;
	std::vector<EASYRPG_SHARED_PTR<Sprite_Character> > character_sprites;
	boost::scoped_ptr<Sprite_AirshipShadow> airship_shadow;
	boost::scoped_ptr<Sprite_Timer> timer1;
	boost::scoped_ptr<Sprite_Timer> timer2;
	bool chipset_ready;
};

#endif
