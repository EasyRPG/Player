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

#ifndef EP_GAME_PLAYER_H
#define EP_GAME_PLAYER_H

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
	int GetScreenZ() const override;
	int GetOriginalMoveRouteIndex() const override;
	void SetOriginalMoveRouteIndex(int new_index) override;
	bool GetVisible() const override;
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

	/*
	 * Overridden to convince Game_Character we aren't stopped if boarding/unboarding.
	 * Consider calling this 'IsReadyToMove' or something, and 'IsMovable' -> 'IsPlayerMovable'
	 */
	bool IsStopping() const override;

	bool GetOnOffVehicle();
	bool IsMovable() const;
	bool InVehicle() const;
	bool InAirship() const;
	bool IsBoardingOrUnboarding() const;
	Game_Vehicle* GetVehicle() const;
	bool CanWalk(int x, int y);

	/** Workaround used to avoid blocking the player with move routes that are completeable in a single frame */
	bool IsBlockedByMoveRoute() const;

	/**
	 * Callback function invoked by the Vehicle to notify that the unboarding has finished
	 */
	void UnboardingFinished();

protected:
	RPG::SavePartyLocation* data();
	const RPG::SavePartyLocation* data() const;
private:

	bool teleporting = false;
	int new_map_id = 0, new_x = 0, new_y = 0, new_direction = 0;

	RPG::Music walking_bgm;

	void UpdateScroll(int prev_x, int prev_y);
	void UpdatePan();
	bool CheckTouchEvent();
	bool CheckCollisionEvent();
	bool CheckActionEvent();
	bool CheckEventTriggerHere(const std::vector<int>& triggers, bool triggered_by_decision_key = false);
	bool CheckEventTriggerThere(const std::vector<int>& triggers, bool triggered_by_decision_key = false);
	bool GetOnVehicle();
	bool GetOffVehicle();
	void Unboard();
};

inline RPG::SavePartyLocation* Game_Player::data() {
	return static_cast<RPG::SavePartyLocation*>(Game_Character::data());
}

inline const RPG::SavePartyLocation* Game_Player::data() const {
	return static_cast<const RPG::SavePartyLocation*>(Game_Character::data());
}

#endif
