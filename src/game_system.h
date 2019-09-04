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

#ifndef EP_GAME_SYSTEM_H
#define EP_GAME_SYSTEM_H

// Headers
#include <string>
#include "rpg_animation.h"
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
	 * Fades out the current BGM
	 *
	 * @param duration Duration in ms
	 */
	void BgmFade(int duration);

	/**
	 * Plays a Sound.
	 *
	 * @param se sound data.
	 * @param stop_sounds If true stops all SEs when playing (OFF)/(...). Only used by the interpreter.
	 */
	void SePlay(const RPG::Sound& se, bool stop_sounds = false);

	/**
	 * Plays the first valid sound in the animation.
	 *
	 * @param animation animation data.
	 */
	void SePlay(const RPG::Animation& animation);

	/** @return system graphic filename.  */
	const std::string& GetSystemName();

	/** @return message stretch style */
	RPG::System::Stretch GetMessageStretch();

	/** @return system font */
	RPG::System::Font GetFontId();

	/**
	 * Sets the system graphic.
	 *
	 * @param system_name new system name.
	 * @param message_stretch message stretch style
	 * @param font_id The system font to use.
	 */
	void SetSystemGraphic(const std::string& system_name,
			RPG::System::Stretch stretch,
			RPG::System::Font font);

	/** Resets the system graphic to the default value. */
	void ResetSystemGraphic();

	/** @return the system2 graphic name */
	const std::string& GetSystem2Name();

	/** @return true if the game has a configured system graphic */
	bool HasSystemGraphic();

	/** @return true if the game has a configured system2 graphic */
	bool HasSystem2Graphic();

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

	int GetSaveCount();

	RPG::Music& GetCurrentBGM();
	void MemorizeBGM();
	void PlayMemorizedBGM();

	void OnBgmReady(FileRequestResult* result);
	void OnSeReady(FileRequestResult* result, int volume, int tempo, bool stop_sounds);
	void ReloadSystemGraphic();

	/**
	 * Determines if the requested file is supposed to Stop BGM/SE play.
	 * For empty string and (OFF) this is always the case.
	 * Many RPG Maker translation overtranslated the (OFF) reserved string,
	 * e.g. (Brak) and (Kein Sound).
	 * A file is detected as "Stop BGM/SE" when the file is missing in the
	 * filesystem and the name is wrapped in (), otherwise it is a regular
	 * file.
	 *
	 * @param name File to find
	 * @param find_func Find function to use (FindSound or FindMusic)
	 * @param found_name Name of the found file to play
	 * @return true when the file is supposed to Stop playback.
	 *         false otherwise and file to play is returned as found_name
	 */
	bool IsStopFilename(const std::string& name, std::string (*find_func) (const std::string&), std::string& found_name);

	bool IsStopMusicFilename(const std::string& name, std::string& found_name);
	bool IsStopMusicFilename(const std::string& name);
	bool IsStopSoundFilename(const std::string& name, std::string& found_name);
	bool IsStopSoundFilename(const std::string& name);
}

inline bool Game_System::HasSystemGraphic() {
	return !GetSystemName().empty();
}

inline bool Game_System::HasSystem2Graphic() {
	return !GetSystem2Name().empty();
}

inline bool Game_System::IsStopMusicFilename(const std::string& name) {
	std::string s;
	return IsStopMusicFilename(name, s);
}
inline bool Game_System::IsStopSoundFilename(const std::string& name) {
	std::string s;
	return IsStopSoundFilename(name, s);
}

#endif
