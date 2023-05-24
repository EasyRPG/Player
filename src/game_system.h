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
#include <map>
#include <lcf/rpg/animation.h>
#include <lcf/rpg/music.h>
#include <lcf/rpg/sound.h>
#include <lcf/rpg/system.h>
#include <lcf/rpg/savesystem.h>
#include "color.h"
#include "transition.h"
#include "string_view.h"
#include "async_handler.h"
#include "filesystem_stream.h"

struct FileRequestResult;

/**
 * Game System namespace.
 */
class Game_System {
public:
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

	using AtbMode = lcf::rpg::SaveSystem::AtbMode;

	/**
	 * Initializes Game System.
	 */
	Game_System();

	/** Initialize from save game */
	void SetupFromSave(lcf::rpg::SaveSystem save);

	/** @return save game data */
	const lcf::rpg::SaveSystem& GetSaveData() const;

	/**
	 * Plays a Music.
	 *
	 * @param bgm music data.
	 */
	void BgmPlay(lcf::rpg::Music const& bgm);

	/**
	 * Stops playing music.
	 */
	void BgmStop();

	/**
	 * Fades out the current BGM
	 *
	 * @param duration Duration in ms
	 * @param clear_current_music If true then current_music is set to (OFF). Only needed on starting a new game.
	 */
	void BgmFade(int duration, bool clear_current_music = false);

	/**
	 * Returns whether a Bgm track played once.
	 * For RPG_RT compatibility always returns false for WAV files.
	 * Use Audio().BGM_PlayedOnce() if this is not desired.
	 *
	 * @return Whether Bgm played once
	 */
	bool BgmPlayedOnce();

	/**
	 * Plays a Sound.
	 *
	 * @param se sound data.
	 * @param stop_sounds If true stops all SEs when playing (OFF)/(...). Only used by the interpreter.
	 */
	void SePlay(const lcf::rpg::Sound& se, bool stop_sounds = false);

	/**
	 * Plays the first valid sound in the animation.
	 *
	 * @param animation animation data.
	 */
	void SePlay(const lcf::rpg::Animation& animation);

	/** @return system graphic filename.  */
	StringView GetSystemName();

	/** @return message stretch style */
	lcf::rpg::System::Stretch GetMessageStretch();

	/** @return system font */
	lcf::rpg::System::Font GetFontId();

	/**
	 * Sets the system graphic.
	 *
	 * @param system_name new system name.
	 * @param message_stretch message stretch style
	 * @param font_id The system font to use.
	 */
	void SetSystemGraphic(const std::string& system_name,
			lcf::rpg::System::Stretch stretch,
			lcf::rpg::System::Font font);

	/** Resets the system graphic to the default value. */
	void ResetSystemGraphic();

	/** @return the system2 graphic name */
	StringView GetSystem2Name();

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
	const lcf::rpg::Music& GetSystemBGM(int which);

	/**
	 * Sets the system music.
	 *
	 * @param which which "context" to set the music for.
	 * @param bgm the music.
	 */
	void SetSystemBGM(int which, lcf::rpg::Music bgm);

	/**
	 * Gets the system sound effects.
	 *
	 * @param which which "context" to set the music for.
	 * @return the sound.
	 */
	const lcf::rpg::Sound& GetSystemSE(int which);

	/**
	 * Sets a system sound effect.
	 *
	 * @param which which "context" to set the effect for.
	 * @param sfx the sound effect.
	 */
	void SetSystemSE(int which, lcf::rpg::Sound sfx);

	/**
	 * Gets the system transitions.
	 *
	 * @param which which "context" to get the transition for.
	 * @return the transition.
	 */
	Transition::Type GetTransition(int which);

	/**
	 * Sets the system transitions.
	 *
	 * @param which which "context" to set the transition for.
	 * @param transition the transition.
	 */
	void SetTransition(int which, int transition);

	bool GetAllowTeleport();
	void SetAllowTeleport(bool allow);
	bool GetAllowEscape();
	void SetAllowEscape(bool allow);
	bool GetAllowSave();
	void SetAllowSave(bool allow);
	bool GetAllowMenu();
	void SetAllowMenu(bool allow);

	int GetSaveCount();
	void IncSaveCount();

	const lcf::rpg::Music& GetCurrentBGM();
	void MemorizeBGM();
	void PlayMemorizedBGM();

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
	 * @param find_func Find function to use (OpenSound or OpenMusic)
	 * @param found_stream Handle to the file to play
	 * @return true when the file is supposed to Stop playback.
	 *         false otherwise and a handle to the file is returned in found_stream
	 */
	static bool IsStopFilename(StringView name, Filesystem_Stream::InputStream (*find_func) (StringView), Filesystem_Stream::InputStream& found_stream);

	static bool IsStopMusicFilename(StringView name, Filesystem_Stream::InputStream& found_stream);
	static bool IsStopMusicFilename(StringView name);
	static bool IsStopSoundFilename(StringView name, Filesystem_Stream::InputStream& found_stream);
	static bool IsStopSoundFilename(StringView name);

	/** @return current atb mode */
	AtbMode GetAtbMode();

	/** Set the atb mode */
	void SetAtbMode(AtbMode m);

	/** Flip the atb mode to the opposite */
	void ToggleAtbMode();

	/** @return Music playing before battle started */
	const lcf::rpg::Music& GetBeforeBattleMusic();

	/**
	 * Set before battle music
	 *
	 * @param music music to set
	 */
	void SetBeforeBattleMusic(lcf::rpg::Music music);

	/** @return Music playing before boarded vehicle */
	const lcf::rpg::Music& GetBeforeVehicleMusic();

	/**
	 * Set before vehicle music
	 *
	 * @param name name of music to set
	 */
	void SetBeforeVehicleMusic(lcf::rpg::Music music);

	/** @return save slot used for last save game */
	int GetSaveSlot();

	/**
	 * Set the save slot used when saving the game
	 *
	 * @param slot the slot number
	 */
	void SetSaveSlot(int slot);

	/** @return RPG_RT compatible frame counter */
	int GetFrameCounter();

	/** Reset the RPG_RT compatible frame counter to 0 */
	void ResetFrameCounter();

	/** Increment the RPG_RT compatible frame counter */
	void IncFrameCounter();

	/** Get the system background color */
	Color GetBackgroundColor();

	/** @return true if battle animations are flipped if attacked from behind */
	bool GetInvertAnimations();

	/** Reset the face graphic. */
	void ClearMessageFace();

	/** @return name of file that contains the face. */
	StringView GetMessageFaceName();

	/**
	 * Set FaceSet graphic file containing the face.
	 *
	 * @param face FaceSet file
	 */
	void SetMessageFaceName(const std::string& face);

	/**
	 * Gets index of the face to display.
	 *
	 * @return face index
	 */
	int GetMessageFaceIndex();

	/**
	 * Sets index of the face to display
	 *
	 * @param index face index
	 */
	void SetMessageFaceIndex(int index);

	/**
	 * Whether to mirror the face.
	 *
	 * @return true: flipped, false: normal
	 */
	bool IsMessageFaceFlipped();

	/**
	 * Sets whether to mirror the face.
	 *
	 * @param flipped Enable/Disable mirroring
	 */
	void SetMessageFaceFlipped(bool flipped);

	/**
	 * If the face shall be placed right.
	 *
	 * @return true: right side, false: left side
	 */
	bool IsMessageFaceRightPosition();

	/**
	 * Sets the face position.
	 *
	 * @param right true: right side, false: left side
	 */
	void SetMessageFaceRightPosition(bool right);

	/**
	 * Gets if the message background is transparent.
	 *
	 * @return message transparent
	 */
	bool IsMessageTransparent();

	/**
	 * Sets message box background state
	 *
	 * @param transparent true: transparent, false: opaque
	 */
	void SetMessageTransparent(bool transparent);

	/**
	 * Gets the message box position.
	 *
	 * @return 0: top, 1: middle, 2: bottom
	 */
	int GetMessagePosition();

	/**
	 * Sets the message box position.
	 * Depending on the player position this value is ignored to prevent overlap.
	 * (see SetPositionFixed)
	 *
	 * @param new_position 0: top, 1: middle, 2: bottom
	 */
	void SetMessagePosition(int new_position);

	/**
	 * Gets whether message box position is fixed.
	 * In that case the hero can be obstructed.
	 *
	 * @return fixed
	 */
	bool IsMessagePositionFixed();

	/**
	 * Sets if message box is moved to avoid obscuring the player.
	 *
	 * @param fixed position fixed
	 */
	void SetMessagePositionFixed(bool fixed);

	/**
	 * Gets if parallel events continue while message box is displayed.
	 *
	 * @return whether events continue
	 */
	bool GetMessageContinueEvents();

	/**
	 * Sets if parallel events continue while message box is displayed.
	 *
	 * @param continue_events continue events
	 */
	void SetMessageContinueEvents(bool continue_events);

	/**
	 * Sets the RpgRt event message active flag.
	 *
	 * @param value what to set the flag to
	 */
	void SetMessageEventMessageActive(bool value);

	/** @return the RpgRt event message active flag */
	bool GetMessageEventMessageActive();

	/** @return Whether the game was loaded from a savegame in the current frame */
	bool IsLoadedThisFrame() const;

private:
	void OnBgmReady(FileRequestResult* result);
	void OnBgmInelukiReady(FileRequestResult* result);
	void OnSeReady(FileRequestResult* result, lcf::rpg::Sound se, bool stop_sounds);
	void OnChangeSystemGraphicReady(FileRequestResult* result);
private:
	lcf::rpg::SaveSystem data;
	const lcf::rpg::System* dbsys;
	FileRequestBinding music_request_id;
	FileRequestBinding system_request_id;
	std::map<std::string, FileRequestBinding> se_request_ids;
	Color bg_color = Color{ 0, 0, 0, 255 };
	bool bgm_pending = false;
	uint32_t loaded_frame_count = 0;
};

inline bool Game_System::HasSystemGraphic() {
	return !GetSystemName().empty();
}

inline bool Game_System::HasSystem2Graphic() {
	return !GetSystem2Name().empty();
}

inline bool Game_System::IsStopMusicFilename(StringView name) {
	Filesystem_Stream::InputStream s;
	return IsStopMusicFilename(name, s);
}

inline bool Game_System::IsStopSoundFilename(StringView name) {
	Filesystem_Stream::InputStream s;
	return IsStopSoundFilename(name, s);
}

inline void Game_System::ClearMessageFace() {
	SetMessageFaceName("");
	SetMessageFaceIndex(0);
}

inline StringView Game_System::GetMessageFaceName() {
	return data.face_name;
}

inline void Game_System::SetMessageFaceName(const std::string& face) {
	data.face_name = face;
}

inline int Game_System::GetMessageFaceIndex() {
	return data.face_id;
}

inline void Game_System::SetMessageFaceIndex(int index) {
	data.face_id = index;
}

inline bool Game_System::IsMessageFaceFlipped() {
	return data.face_flip;
}

inline void Game_System::SetMessageFaceFlipped(bool flipped) {
	data.face_flip = flipped;
}

inline bool Game_System::IsMessageFaceRightPosition() {
	return data.face_right;
}

inline void Game_System::SetMessageFaceRightPosition(bool right) {
	data.face_right = right;
}

inline void Game_System::SetMessageTransparent(bool transparent) {
	data.message_transparent = transparent;
}

inline int Game_System::GetMessagePosition() {
	return data.message_position;
}

inline void Game_System::SetMessagePosition(int new_position) {
	data.message_position = new_position;
}

inline bool Game_System::IsMessagePositionFixed() {
	return !data.message_prevent_overlap;
}

inline void Game_System::SetMessagePositionFixed(bool fixed) {
	data.message_prevent_overlap = !fixed;
}

inline bool Game_System::GetMessageContinueEvents() {
	return data.message_continue_events;
}

inline void Game_System::SetMessageContinueEvents(bool continue_events) {
	data.message_continue_events = continue_events;
}

inline void Game_System::SetMessageEventMessageActive(bool value) {
	data.event_message_active = value;
}

inline bool Game_System::GetMessageEventMessageActive() {
	return data.event_message_active;
}

inline bool Game_System::IsLoadedThisFrame() const {
	return loaded_frame_count + 1 == data.frame_count;
}

inline Game_System::AtbMode Game_System::GetAtbMode() {
	return static_cast<Game_System::AtbMode>(data.atb_mode);
}

inline void Game_System::SetAtbMode(AtbMode m) {
	data.atb_mode = m;
}

inline void Game_System::ToggleAtbMode() {
	data.atb_mode = !data.atb_mode;
}

inline const lcf::rpg::Music& Game_System::GetBeforeBattleMusic() {
	return data.before_battle_music;
}

inline void Game_System::SetBeforeBattleMusic(lcf::rpg::Music music) {
	data.before_battle_music = std::move(music);
}

inline const lcf::rpg::Music& Game_System::GetBeforeVehicleMusic() {
	return data.before_vehicle_music;
}

inline void Game_System::SetBeforeVehicleMusic(lcf::rpg::Music music) {
	data.before_vehicle_music = std::move(music);
}

inline int Game_System::GetSaveSlot() {
	return data.save_slot;
}

inline void Game_System::SetSaveSlot(int slot) {
	data.save_slot = slot;
}

inline int Game_System::GetFrameCounter() {
	return data.frame_count;
}

inline void Game_System::ResetFrameCounter() {
	data.frame_count = 0;
}

inline void Game_System::IncFrameCounter() {
	++data.frame_count;
}

inline Color Game_System::GetBackgroundColor() {
	return bg_color;
}

inline bool Game_System::GetInvertAnimations() {
	return dbsys->invert_animations;
}

inline int Game_System::GetSaveCount() {
	return data.save_count;
}

inline void Game_System::IncSaveCount() {
	++data.save_count;
}

inline StringView Game_System::GetSystem2Name() {
	return dbsys->system2_name;
}

inline const lcf::rpg::Music& Game_System::GetCurrentBGM() {
	return data.current_music;
}

inline void Game_System::MemorizeBGM() {
	data.stored_music = data.current_music;
}

inline void Game_System::PlayMemorizedBGM() {
	BgmPlay(data.stored_music);
}

inline void Game_System::SetAllowTeleport(bool allow) {
	data.teleport_allowed = allow;
}

inline bool Game_System::GetAllowTeleport() {
	return data.teleport_allowed;
}

inline void Game_System::SetAllowEscape(bool allow) {
	data.escape_allowed = allow;
}

inline bool Game_System::GetAllowEscape() {
	return data.escape_allowed;
}

inline void Game_System::SetAllowSave(bool allow) {
	data.save_allowed = allow;
}

inline bool Game_System::GetAllowSave() {
	return data.save_allowed;
}

inline void Game_System::SetAllowMenu(bool allow) {
	data.menu_allowed = allow;
}

inline bool Game_System::GetAllowMenu() {
	return data.menu_allowed;
}


#endif
