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
	int GetX() const;
	void SetX(int new_x);
	int GetY() const;
	void SetY(int new_y);
	int GetMapId() const;
	void SetMapId(int new_map_id);
	int GetDirection() const;
	void SetDirection(int new_direction);
	int GetSpriteDirection() const;
	void SetSpriteDirection(int new_direction);
	bool IsFacingLocked() const;
	void SetFacingLocked(bool locked);
	int GetLayer() const;
	void SetLayer(int new_layer);
	int GetSteppingSpeed() const;
	int GetMoveSpeed() const;
	void SetMoveSpeed(int speed);
	int GetMoveFrequency() const;
	void SetMoveFrequency(int frequency);
	const RPG::MoveRoute& GetMoveRoute() const;
	void SetMoveRoute(const RPG::MoveRoute& move_route);
	int GetOriginalMoveRouteIndex() const;
	void SetOriginalMoveRouteIndex(int new_index);
	int GetMoveRouteIndex() const;
	void SetMoveRouteIndex(int new_index);
	bool IsMoveRouteOverwritten() const;
	void SetMoveRouteOverwritten(bool force);
	bool IsMoveRouteRepeated() const;
	void SetMoveRouteRepeated(bool force);
	const std::string& GetSpriteName() const;
	void SetSpriteName(const std::string& sprite_name);
	int GetSpriteIndex() const;
	void SetSpriteIndex(int index);
	Color GetFlashColor() const;
	void SetFlashColor(const Color& flash_color);
	double GetFlashLevel() const;
	void SetFlashLevel(double flash_level);
	int GetFlashTimeLeft() const;
	void SetFlashTimeLeft(int time_left);
	bool IsPassable(int x, int y, int d) const;
	/** @} */

	void LoadSystemSettings();
	RPG::Music& GetBGM();
	void Refresh();
	void SetPosition(int _map_id, int _x, int _y);
	bool IsInCurrentMap() const;
	virtual bool IsInPosition(int x, int y) const;
	virtual bool GetVisible() const;
	bool IsAscending() const;
	bool IsDescending() const;
	int GetAltitude() const;
	void GetOn();
	void GetOff();
	bool IsInUse() const;
	void SyncWithPlayer();
	virtual int GetScreenY() const;
	bool IsMovable();
	bool CanLand() const;
	virtual void Update();
	virtual bool CheckEventTriggerTouch(int x, int y);

protected:
	RPG::SaveVehicleLocation& data;

	Type type;
	bool driving;
};

#endif
