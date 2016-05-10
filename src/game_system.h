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

#ifndef _GAME_SYSTEM_H_
#define _GAME_SYSTEM_H_

// Headers
#include <string>
#include "rpg_music.h"
#include "rpg_sound.h"
#include "rpg_system.h"

struct FileRequestResult;

/**
 * Game System namespace.
 */
namespace Game_System {
	enum sys_bgm {
		BGM_Battle,
		BGM_Victory,
		BGM_Inn,
		BGM_Boat,
		BGM_Ship,
		BGM_Airship,
		BGM_GameOver,

		BGM_Count
	};

	enum sys_sfx {
		SFX_Cursor,
		SFX_Decision,
		SFX_Cancel,
		SFX_Buzzer,
		SFX_BeginBattle,
		SFX_Escape,
		SFX_EnemyAttacks,
		SFX_EnemyDamage,
		SFX_AllyDamage,
		SFX_Evasion,
		SFX_EnemyKill,
		SFX_UseItem,

		SFX_Count
	};

	enum sys_transition {
		Transition_TeleportErase,
		Transition_TeleportShow,
		Transition_BeginBattleErase,
		Transition_BeginBattleShow,
		Transition_EndBattleErase,
		Transition_EndBattleShow,
		Transition_Count
	};

	class Target {
	public:
		int map_id;
		int x;
		int y;
		int switch_id;
		Target()
			: map_id(0),
			  x(0),
			  y(0),
			  switch_id(0)
		{}
		Target(int map_id, int x, int y, int switch_id)
			: map_id(map_id),
			  x(x),
			  y(y),
			  switch_id(switch_id)
		{}
	};

	/**
	 * Initializes Game System.
	 */
	void Init();

	/**
	 * Plays a Music.
	 *
	 * @param bgm music data.
	 */
	void BgmPlay(RPG::Music const& bgm);

	/**
	 * Stops playing music.
	 */
	void BgmStop();

	/**
	 * Plays a Sound.
	 *
	 * @param se sound data.
	 */
	void SePlay(RPG::Sound const& se);

	/**
	 * Gets system graphic name.
	 *
	 * @return system graphic filename.
	 */
	std::string GetSystemName();

	/**
	 * Sets the system graphic.
	 *
	 * @param system_name new system name.
	 */
	void SetSystemName(std::string const& system_name);

	/**
	 * Gets the system music.
	 *
	 * @param which which "context" to set the music for.
	 * @return the music.
	 */
	RPG::Music& GetSystemBGM(int which);

	/**
	 * Sets the system music.
	 *
	 * @param which which "context" to set the music for.
	 * @param bgm the music.
	 */
	void SetSystemBGM(int which, RPG::Music const& bgm);

	/**
	 * Gets the system sound effects.
	 *
	 * @param which which "context" to set the music for.
	 * @return the sound.
	 */
	RPG::Sound& GetSystemSE(int which);

	/**
	 * Sets a system sound effect.
	 *
	 * @param which which "context" to set the effect for.
	 * @param sfx the sound effect.
	 */
	void SetSystemSE(int which, RPG::Sound const& sfx);

	/**
	 * Gets the system transitions.
	 *
	 * @param which which "context" to get the transition for.
	 * @return the transition.
	 */
	int GetTransition(int which);

	/**
	 * Sets the system transitions.
	 *
	 * @param which which "context" to set the transition for.
	 * @param transition the transition.
	 */
	void SetTransition(int which, int transition);

	/**
	 * Sets a teleport target.
	 *
	 * @param map_id the destination map.
	 * @param x the destination X coordinate.
	 * @param y the destination Y coordinate.
	 * @param switch_id the switch ID.
	 */
	void AddTeleportTarget(int map_id, int x, int y, int switch_id);

	/**
	 * Removes a teleport target.
	 *
	 * @param map_id the map for which the target was used.
	 */
	void RemoveTeleportTarget(int map_id);

	/**
	 * Finds a teleport target.
	 *
	 * @param map_id the map for which to obtain the target.
	 * @return: pointer to a Target structure, or NULL.
	 */
	Target* GetTeleportTarget(int map_id);

	/**
	 * Sets an escape  target.
	 *
	 * @param map_id the destination map.
	 * @param x the destination X coordinate.
	 * @param y the destination Y coordinate.
	 * @param switch_id the switch ID.
	 */
	void SetEscapeTarget(int map_id, int x, int y, int switch_id);

	/**
	 * Finds an escape target.
	 *
	 * @return pointer to a Target structure, or NULL.
	 */
	Target* GetEscapeTarget();

	bool GetAllowTeleport();
	void SetAllowTeleport(bool allow);
	bool GetAllowEscape();
	void SetAllowEscape(bool allow);
	bool GetAllowSave();
	void SetAllowSave(bool allow);
	bool GetAllowMenu();
	void SetAllowMenu(bool allow);
	RPG::System::Stretch GetMessageStretch();
	void SetMessageStretch(RPG::System::Stretch stretch);

	int GetSaveCount();

	RPG::Music& GetCurrentBGM();
	void MemorizeBGM();
	void PlayMemorizedBGM();

	void OnBgmReady(FileRequestResult* result);
	void OnSeReady(FileRequestResult* result, int volume, int tempo);
}

#endif
