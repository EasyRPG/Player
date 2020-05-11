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

#ifndef EP_GAME_VEHICLE_H
#define EP_GAME_VEHICLE_H

// Headers
#include <string>
#include <lcf/rpg/music.h>
#include <lcf/rpg/savevehiclelocation.h>
#include "game_character.h"

using Game_VehicleBase = Game_CharacterDataStorage<lcf::rpg::SaveVehicleLocation>;

/**
 * Game_Vehicle class.
 */
class Game_Vehicle : public Game_VehicleBase {
public:
	enum Type {
		None = 0,
		Boat,
		Ship,
		Airship
	};

	static const char TypeNames[4][8];

	explicit Game_Vehicle(Type type);

	/** Load from saved game */
	void SetSaveData(lcf::rpg::SaveVehicleLocation save);

	/** @return save game data */
	lcf::rpg::SaveVehicleLocation GetSaveData() const;

	/**
	 * Implementation of abstract methods
	 */
	/** @{ */
	int GetVehicleType() const override;
	/** @} */

	/** Update this for the current frame */
	void Update();

	const lcf::rpg::Music& GetBGM();
	bool IsInCurrentMap() const;
	bool IsInPosition(int x, int y) const override;
	bool GetVisible() const override;
	bool IsAscending() const;
	bool IsDescending() const;
	bool IsAscendingOrDescending() const;
	int GetAltitude() const;
	void GetOn();
	void GetOff();
	bool IsInUse() const;
	bool IsAboard() const;
	void SyncWithPlayer();
	void AnimateAscentDescent();
	int GetScreenY(bool apply_shift = false, bool apply_jump = true) const override;
	bool CanLand() const;
	void UpdateAnimation();

	/**
	 * Sets default sprite name. Usually the name of the graphic file.
	 *
	 * @param sprite_name new sprite name
	 * @param index the index of the new sprite.
	 */
	void SetOrigSpriteGraphic(std::string sprite_name, int index);

	/** Gets the original sprite graphic name */
	StringView GetOrigSpriteName() const;

	/** Gets the original sprite graphic index */
	int GetOrigSpriteIndex() const;
};

inline void Game_Vehicle::SetOrigSpriteGraphic(std::string sprite_name, int index) {
	data()->orig_sprite_name = std::move(sprite_name);
	data()->orig_sprite_id = index;
}

#endif
