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

#ifndef _GAME_PLAYER_H_
#define _GAME_PLAYER_H_

// Headers
#include "rpg_music.h"
#include "rpg_savepartylocation.h"
#include "game_character.h"
#include <vector>

/**
 * Game Player class
 */
class Game_Player : public Game_Character {
public:
	Game_Player();

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

	bool IsPassable(int x, int y, int d) const;
	bool IsTeleporting() const;
	void ReserveTeleport(int map_id, int x, int y);
	void StartTeleport();
	void PerformTeleport();
	void Center(int x, int y);
	void MoveTo(int x, int y);
	void Update();

	void Refresh();

	bool CheckEventTriggerTouch(int x, int y);
	bool GetOnOffVehicle();
	bool IsMovable() const;
	bool InVehicle() const;
	bool InAirship() const;
	bool AirshipLandOk(int x, int y) const;
	bool CanWalk(int x, int y);
	void BeginMove();

private:
	RPG::SavePartyLocation& location;

	bool teleporting;
	int vehicle_type;
	bool vehicle_getting_on;
	bool vehicle_getting_off;
	int new_map_id, new_x, new_y;
	RPG::Music walking_bgm;

	void UpdateScroll(int last_real_x, int last_real_y);
	void UpdateNonMoving(bool last_moving);
	bool CheckTouchEvent();
	bool CheckActionEvent();
	bool CheckEventTriggerHere(const std::vector<int>& triggers);
	bool CheckEventTriggerThere(const std::vector<int>& triggers);
	bool GetOnVehicle();
	bool GetOffVehicle();
};

#endif
