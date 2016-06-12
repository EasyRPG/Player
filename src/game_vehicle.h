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

#ifndef _GAME_VEHICLE_H_
#define _GAME_VEHICLE_H_

// Headers
#include <string>
#include "rpg_music.h"
#include "rpg_savevehiclelocation.h"
#include "game_character.h"

/**
 * Game_Vehicle class.
 */
class Game_Vehicle : public Game_Character {
public:
	enum Type {
		None = 0,
		Boat,
		Ship,
		Airship
	};

	Game_Vehicle(Type _type);

	/**
	 * Implementation of abstract methods
	 */
	/** @{ */
	int GetX() const override;
	void SetX(int new_x) override;
	int GetY() const override;
	void SetY(int new_y) override;
	int GetMapId() const override;
	void SetMapId(int new_map_id) override;
	int GetDirection() const override;
	void SetDirection(int new_direction) override;
	int GetSpriteDirection() const override;
	void SetSpriteDirection(int new_direction) override;
	bool IsFacingLocked() const override;
	void SetFacingLocked(bool locked) override;
	int GetLayer() const override;
	void SetLayer(int new_layer) override;
	int GetSteppingSpeed() const override;
	int GetMoveSpeed() const override;
	void SetMoveSpeed(int speed) override;
	int GetMoveFrequency() const override;
	void SetMoveFrequency(int frequency) override;
	const RPG::MoveRoute& GetMoveRoute() const override;
	void SetMoveRoute(const RPG::MoveRoute& move_route) override;
	int GetOriginalMoveRouteIndex() const override;
	void SetOriginalMoveRouteIndex(int new_index) override;
	int GetMoveRouteIndex() const override;
	void SetMoveRouteIndex(int new_index) override;
	bool IsMoveRouteOverwritten() const override;
	void SetMoveRouteOverwritten(bool force) override;
	bool IsMoveRouteRepeated() const override;
	void SetMoveRouteRepeated(bool force) override;
	const std::string& GetSpriteName() const override;
	void SetSpriteName(const std::string& sprite_name) override;
	int GetSpriteIndex() const override;
	void SetSpriteIndex(int index) override;
	Color GetFlashColor() const override;
	void SetFlashColor(const Color& flash_color) override;
	double GetFlashLevel() const override;
	void SetFlashLevel(double flash_level) override;
	int GetFlashTimeLeft() const override;
	void SetFlashTimeLeft(int time_left) override;
	bool GetThrough() const override;
	void SetThrough(bool through) override;
	bool MakeWay(int x, int y, int d) const override;
	int GetBushDepth() const override;
	/** @} */

	void LoadSystemSettings();
	RPG::Music& GetBGM();
	void Refresh();
	void SetPosition(int _map_id, int _x, int _y);
	bool IsInCurrentMap() const;
	bool IsInPosition(int x, int y) const override;
	bool GetVisible() const override;
	bool IsAscending() const;
	bool IsDescending() const;
	int GetAltitude() const;
	void GetOn();
	void GetOff();
	bool IsInUse() const;
	void SyncWithPlayer();
	int GetScreenY() const override;
	bool IsMovable();
	bool CanLand() const;
	void Update() override;
	bool CheckEventTriggerTouch(int x, int y) override;

protected:
	RPG::SaveVehicleLocation& data;

	Type type;
	bool driving;
};

#endif
