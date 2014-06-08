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
	static const int MAX_ALTITUDE = 32;	// the airship flies at

	enum Type {
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
	int GetPrelockDirection() const;
	void SetPrelockDirection(int new_direction);
	bool IsFacingLocked() const;
	void SetFacingLocked(bool locked);
	int GetLayer() const;
	void SetLayer(int new_layer);
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
	int GetFlashLevel() const;
	void SetFlashLevel(int flash_level);
	int GetFlashTimeLeft() const;
	void SetFlashTimeLeft(int time_left);
	/** @} */

	void LoadSystemSettings();
	void Refresh();
	void SetPosition(int _map_id, int _x, int _y);
	virtual bool IsInPosition(int x, int y) const;
	virtual bool GetVisible() const;
	void GetOn();
	void GetOff();
	void SyncWithPlayer();
	virtual int GetScreenY() const;
	bool IsMovable();
	virtual void Update();
	virtual bool CheckEventTriggerTouch(int x, int y);

protected:
	RPG::SaveVehicleLocation& data;

	int map_id;
	Type type;
	int altitude;
	bool driving;
	RPG::Music bgm;
};

#endif
