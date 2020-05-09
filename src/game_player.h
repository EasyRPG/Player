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
#include <lcf/rpg/fwd.h>
#include <lcf/rpg/music.h>
#include <lcf/rpg/savepartylocation.h>
#include "game_character.h"
#include <lcf/flag_set.h>
#include "teleport_target.h"
#include <vector>

class Game_Vehicle;

/**
 * Game Player class
 */
class Game_Player : public Game_Character {
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
	int GetScreenZ(bool apply_shift = false) const override;
	bool GetVisible() const override;
	bool MakeWay(int x, int y) const override;
	void BeginMove() override;
	int GetVehicleType() const override;
	void UpdateSelfMovement() override;
	void OnMoveFailed(int x, int y) override;
	void UpdateMoveRoute(int32_t& current_index, const lcf::rpg::MoveRoute& current_route) override;
	/** @} */

	bool IsPendingTeleport() const;
	TeleportTarget GetTeleportTarget() const;
	void ResetTeleportTarget(TeleportTarget tt = {});

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

	void MoveTo(int x, int y) override;

	/** Update this for the current frame */
	void Update();

	void Refresh();

	bool GetOnOffVehicle();
	bool InVehicle() const;
	bool InAirship() const;
	bool IsAboard() const;
	bool IsBoardingOrUnboarding() const;
	Game_Vehicle* GetVehicle() const;

	/**
	 * Callback function invoked by the Vehicle to notify that the unboarding has finished
	 */
	void UnboardingFinished();

	/**
	 * Set the menu callling flag
	 *
	 * @param value the value of the flag to set
	 */
	void SetMenuCalling(bool value);

	/** @return the menu calling flag */
	bool IsMenuCalling() const;

	/**
	 * Set the encounter callling flag
	 *
	 * @param value the value of the flag to set
	 */
	void SetEncounterCalling(bool value);

	/** @return the encounter calling flag */
	bool IsEncounterCalling() const;

	/** @return number of encounter steps scaled by terrain encounter rate percentage. */
	int GetEncounterSteps() const;

	/**
	 * Sets encounter_steps to steps.
	 *
	 * @param steps the steps value to set.
	 */
	void SetEncounterSteps(int steps);

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

	void LockPan();
	void UnlockPan();
	void StartPan(int direction, int distance, int speed);
	void ResetPan(int speed);

	/** @return how many frames it'll take to finish the current pan */
	int GetPanWait();

	int IsMapCompatibleWithSave(int map_save_count) const;
	int IsDatabaseCompatibleWithSave(int database_save_count) const;

	void UpdateSaveCounts(int db_save_count, int map_save_count);
protected:
	lcf::rpg::SavePartyLocation* data();
	const lcf::rpg::SavePartyLocation* data() const;
private:
	using TriggerSet = lcf::FlagSet<lcf::rpg::EventPage::Trigger>;

	void UpdateScroll(int prev_x, int prev_y);
	void UpdatePan();
	bool UpdateEncounterSteps();
	bool CheckActionEvent();
	bool CheckEventTriggerHere(TriggerSet triggers, bool face_hero, bool triggered_by_decision_key);
	bool CheckEventTriggerThere(TriggerSet triggers, int x, int y, bool face_hero, bool triggered_by_decision_key);
	bool GetOnVehicle();
	bool GetOffVehicle();
	void Unboard();
	void UpdateVehicleActions();

	std::unique_ptr<lcf::rpg::SavePartyLocation> _data_copy;
	TeleportTarget teleport_target;
	int last_encounter_idx = 0;
};

inline lcf::rpg::SavePartyLocation* Game_Player::data() {
	return static_cast<lcf::rpg::SavePartyLocation*>(Game_Character::data());
}

inline const lcf::rpg::SavePartyLocation* Game_Player::data() const {
	return static_cast<const lcf::rpg::SavePartyLocation*>(Game_Character::data());
}

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

inline int Game_Player::GetEncounterSteps() const {
	return data()->encounter_steps;
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

inline int Game_Player::IsMapCompatibleWithSave(int map_save_count) const {
	return data()->map_save_count == map_save_count;
}

inline int Game_Player::IsDatabaseCompatibleWithSave(int database_save_count) const {
	return data()->database_save_count == database_save_count;
}

inline void Game_Player::UpdateSaveCounts(int db_save_count, int map_save_count) {
	data()->database_save_count = db_save_count;
	data()->map_save_count = map_save_count;
}

#endif
