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
	void UpdateNextMovementAction() override;
	void UpdateAnimation() override;
	/** @} */

	/** Update this for the current frame */
	void Update();

	const lcf::rpg::Music& GetBGM();
	int GetVehicleType() const;
	bool IsInCurrentMap() const;
	bool IsInPosition(int x, int y) const override;
	bool IsVisible() const override;
	bool IsAscending() const;
	bool IsDescending() const;
	bool IsAscendingOrDescending() const;
	int GetAltitude() const;
	bool IsInUse() const;
	bool IsAboard() const;
	void SyncWithRider(const Game_Character* rider);
	bool AnimateAscentDescent();
	int GetScreenY(bool apply_shift = false, bool apply_jump = true) const override;
	bool CanLand() const;
	void StartAscent();
	void StartDescent();
	void SetDefaultDirection();
	void ForceLand();

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

inline lcf::rpg::SaveVehicleLocation Game_Vehicle::GetSaveData() const {
	return *data();
}

inline bool Game_Vehicle::IsInPosition(int x, int y) const {
	return IsInCurrentMap() && Game_Character::IsInPosition(x, y);
}

inline bool Game_Vehicle::IsAscending() const {
	return data()->remaining_ascent > 0;
}

inline bool Game_Vehicle::IsDescending() const {
	return data()->remaining_descent > 0;
}

inline bool Game_Vehicle::IsAscendingOrDescending() const {
	return IsAscending() || IsDescending();
}

inline bool Game_Vehicle::IsVisible() const {
	return IsInCurrentMap() && Game_Character::IsVisible();
}

inline void Game_Vehicle::SetDefaultDirection() {
	SetDirection(Left);
	SetFacing(Left);
}

inline int Game_Vehicle::GetVehicleType() const {
	return data()->vehicle;
}


#endif
