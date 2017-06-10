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

class Game_Vehicle;
namespace RPG {
	class SaveTarget;
}

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
	int GetX() const override;
	void SetX(int new_x) override;
	int GetY() const override;
	void SetY(int new_y) override;
	int GetScreenZ() const override;
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
	bool GetVisible() const override;
	Color GetFlashColor() const override;
	void SetFlashColor(const Color& flash_color) override;
	double GetFlashLevel() const override;
	void SetFlashLevel(double flash_level) override;
	int GetFlashTimeLeft() const override;
	void SetFlashTimeLeft(int time_left) override;
	bool GetThrough() const override;
	void SetThrough(bool through) override;
	bool MakeWay(int x, int y, int d) const override;
	void BeginMove() override;
	void CancelMoveRoute() override;
	/** @} */

	bool IsTeleporting() const;

	/**
	 * Sets the map, position and direction that the game player must have after the teleport is over
	 *
	 * @param map_id Id of the target map
	 * @param x new x position after teleport
	 * @param y new y position after teleport
	 * @param direction New direction after teleport. If -1, the direction isn't changed.
	 */
	void ReserveTeleport(int map_id, int x, int y, int direction = -1);
	void ReserveTeleport(const RPG::SaveTarget& target);
	void StartTeleport();
	void PerformTeleport();
	void Center();
	void MoveTo(int x, int y) override;
	void Update() override;

	void Refresh();

	bool CheckEventTriggerTouch(int x, int y) override;

	bool GetOnOffVehicle();
	bool IsMovable() const;
	bool InVehicle() const;
	bool InAirship() const;
	bool IsBoardingOrUnboarding() const;
	Game_Vehicle* GetVehicle() const;
	bool CanWalk(int x, int y);

	/* Workaround used to avoid blocking the player with move routes that are completeable in a single frame */
	bool IsBlockedByMoveRoute() const;

private:
	RPG::SavePartyLocation& location;

	bool teleporting = false;
	int new_map_id = 0, new_x = 0, new_y = 0, new_direction = 0;

	int last_pan_x = 0, last_pan_y = 0;
	int last_remaining_move = 0, last_remaining_jump = 0;
	// These track how much of the pan has actually occurred, which
	// may be less than the pan values if the pan went off the map.
	int actual_pan_x = 0, actual_pan_y = 0;

	RPG::Music walking_bgm;

	void UpdateScroll();
	bool CheckTouchEvent();
	bool CheckCollisionEvent();
	bool CheckActionEvent();
	bool CheckEventTriggerHere(const std::vector<int>& triggers, bool triggered_by_decision_key = false);
	bool CheckEventTriggerThere(const std::vector<int>& triggers, bool triggered_by_decision_key = false);
	bool GetOnVehicle();
	bool GetOffVehicle();
	void Unboard();
};

#endif
