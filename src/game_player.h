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
#include "game_character.h"
#include "teleport_target.h"
#include <vector>
#include <lcf/rpg/fwd.h>
#include <lcf/rpg/music.h>
#include <lcf/rpg/savepartylocation.h>
#include <lcf/flag_set.h>

class Game_Vehicle;
using Game_PlayerBase = Game_CharacterDataStorage<lcf::rpg::SavePartyLocation>;

/**
 * Game Player class
 */
class Game_Player : public Game_PlayerBase {
public:
	Game_Player();

	/** Load from saved game */
	void SetSaveData(lcf::rpg::SavePartyLocation data);

	/** @return save game data */
	lcf::rpg::SavePartyLocation GetSaveData() const;

	/**
	 * Implementation of abstract methods
	 */
	/** @{ */
	Drawable::Z_t GetScreenZ(bool apply_shift = false) const override;
	bool IsVisible() const override;
	bool MakeWay(int from_x, int from_y, int to_x, int to_y) override;
	void UpdateNextMovementAction() override;
	void UpdateMovement(int amount) override;
	void MoveRouteSetSpriteGraphic(std::string sprite_name, int index) override;
	bool Move(int dir) override;
	/** @} */

	bool IsPendingTeleport() const;
	TeleportTarget GetTeleportTarget() const;
	void ResetTeleportTarget(TeleportTarget tt = {});

	bool TriggerEventAt(int x, int y);

	/**
	 * Sets the map, position and direction that the game player must have after the teleport is over
	 *
	 * @param map_id Id of the target map
	 * @param x new x position after teleport
	 * @param y new y position after teleport
	 * @param direction New direction after teleport. If -1, the direction isn't changed.
	 * @param tt teleport type
	 */
	void ReserveTeleport(int map_id, int x, int y, int direction, TeleportTarget::Type tt);
	void ReserveTeleport(const lcf::rpg::SaveTarget& target);
	void PerformTeleport();

	void MoveTo(int map_id, int x, int y) override;

	/** Update this for the current frame */
	void Update();

	/** Resets graphic based on current party */
	void ResetGraphic();

	bool GetOnOffVehicle();
	bool InVehicle() const;
	bool InAirship() const;
	bool IsAboard() const;
	bool IsBoardingOrUnboarding() const;
	void ForceGetOffVehicle();
	int GetVehicleType() const;
	Game_Vehicle* GetVehicle() const;

	/**
	 * Set the menu callling flag
	 *
	 * @param value the value of the flag to set
	 */
	void SetMenuCalling(bool value);

	/** @return the menu calling flag */
	bool IsMenuCalling() const;

	/**
	 * Set the encounter calling flag
	 *
	 * @param value the value of the flag to set
	 */
	void SetEncounterCalling(bool value);

	/** @return the encounter calling flag */
	bool IsEncounterCalling() const;

	/** @return number of encounter steps scaled by terrain encounter rate percentage. */
	int GetTotalEncounterRate() const;

	/**
	 * Sets accumulated encounter rate
	 *
	 * @param rate the rate to set.
	 */
	void SetTotalEncounterRate(int rate);

	enum PanDirection {
		PanUp,
		PanRight,
		PanDown,
		PanLeft
	};

	bool IsPanActive() const;
	bool IsPanLocked() const;
	int GetPanX() const;
	int GetPanY() const;
	int GetTargetPanX() const;
	int GetTargetPanY() const;

	static int GetDefaultPanX();
	static int GetDefaultPanY();

	void LockPan();
	void UnlockPan();
	void StartPan(int direction, int distance, int speed);
	void ResetPan(int speed);

	/** @return how many frames it'll take to finish the current pan */
	int GetPanWait();

	bool IsMapCompatibleWithSave(int map_save_count) const;
	bool IsDatabaseCompatibleWithSave(int database_save_count) const;

	void UpdateSaveCounts(int db_save_count, int map_save_count);
private:
	using TriggerSet = lcf::FlagSet<lcf::rpg::EventPage::Trigger>;

	void UpdateScroll(int amount, bool was_jumping);
	void UpdatePan();
	void UpdateEncounterSteps();
	bool CheckActionEvent();
	bool CheckEventTriggerHere(TriggerSet triggers, bool triggered_by_decision_key);
	bool CheckEventTriggerThere(TriggerSet triggers, int x, int y, bool triggered_by_decision_key);
	bool GetOnVehicle();
	bool GetOffVehicle();
	bool UpdateAirship();
	void UpdateVehicleActions();

	TeleportTarget teleport_target;
	int last_encounter_idx = 0;
};

inline bool Game_Player::IsPendingTeleport() const {
	return teleport_target.IsActive();
}

inline TeleportTarget Game_Player::GetTeleportTarget() const {
	return teleport_target;
}

inline void Game_Player::ResetTeleportTarget(TeleportTarget tt) {
	teleport_target = std::move(tt);
}

inline void Game_Player::SetMenuCalling(bool value) {
	data()->menu_calling = value;
}

inline bool Game_Player::IsMenuCalling() const {
	return data()->menu_calling;
}

inline void Game_Player::SetEncounterCalling(bool value) {
	data()->encounter_calling = value;
}

inline bool Game_Player::IsEncounterCalling() const {
	return data()->encounter_calling;
}

inline int Game_Player::GetTotalEncounterRate() const {
	return data()->total_encounter_rate;
}

inline bool Game_Player::IsPanActive() const {
	return GetPanX() != GetTargetPanX() || GetPanY() != GetTargetPanY();
}

inline bool Game_Player::IsPanLocked() const {
	return data()->pan_state == lcf::rpg::SavePartyLocation::PanState_fixed;
}

inline int Game_Player::GetPanX() const {
	return data()->pan_current_x;
}

inline int Game_Player::GetPanY() const {
	return data()->pan_current_y;
}

inline int Game_Player::GetTargetPanX() const {
	return data()->pan_finish_x;
}

inline int Game_Player::GetTargetPanY() const {
	return data()->pan_finish_y;
}

inline bool Game_Player::IsMapCompatibleWithSave(int map_save_count) const {
	return data()->map_save_count == map_save_count;
}

inline bool Game_Player::IsDatabaseCompatibleWithSave(int database_save_count) const {
	return data()->database_save_count == database_save_count;
}

inline void Game_Player::UpdateSaveCounts(int db_save_count, int map_save_count) {
	data()->database_save_count = db_save_count;
	data()->map_save_count = map_save_count;
}

inline bool Game_Player::IsVisible() const {
	return !IsAboard() && Game_Character::IsVisible();
}

inline int Game_Player::GetVehicleType() const {
	return data()->vehicle;
}


#endif
