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

#ifndef EP_PLAYER_H
#define EP_PLAYER_H

// Headers
#include "meta.h"
#include "game_clock.h"
#include <vector>
#include <memory>

/**
 * Player namespace.
 */
namespace Player {
	/** Bitmask for type of emulated engine */
	enum EngineType {
		EngineNone = 0,
		/** All versions of RPG Maker 2000 */
		EngineRpg2k = 1,
		/** All versions of RPG Maker 2003 */
		EngineRpg2k3 = 2,
		/** RPG Maker 2000 v1.50 or newer, 2003 v1.05 or newer */
		EngineMajorUpdated = 4,
		/** Official English translation (RPG Maker 2003 v1.10 or newer,
		 * or RPG Maker 2000 v.1.61 or newer) */
		EngineEnglish = 8
	};

	/**
	 * Initializes EasyRPG Player.
	 */
	void Init(int argc, char *argv[]);

	/**
	 * Runs the game engine.
	 */
	void Run();

	/**
	 * Runs the game loop.
	 */
	void MainLoop();

	/**
	 * Pauses the game engine.
	 */
	void Pause();

	/**
	 * Resumes the game engine.
	 */
	void Resume();

	/**
	 * Updates EasyRPG Player.
	 *
	 * @param update_scene Whether to update the current scene.
	 */
	void Update(bool update_scene = true);

	/**
	 * Process input events
	 */
	void UpdateInput();

	/**
	 * Renders EasyRPG Player state to the screen
	 */
	void Draw();

	/**
	 * Returns executed game frames since player start.
	 * Should be 60 fps when game ran fast enough.
	 *
	 * @return Update frames since player start
	 */
	int GetFrames();

	/**
	 * Increment the frame counters.
	 */
	void IncFrame();

	/**
	 * Exits EasyRPG Player.
	 */
	void Exit();

	/**
	 * Parses the command line arguments.
	 */
	void ParseCommandLine(int argc, char *argv[]);

	/**
	 * Initializes all game objects
	 */
	void CreateGameObjects();

	/**
	 * Resets all game objects. Faster then CreateGameObjects because
	 * the database is not reparsed.
	 */
	void ResetGameObjects();

	/**
	 * Loads all databases.
	 */
	void LoadDatabase();

	/**
	 * Loads savegame data.
	 *
	 * @param save_file Savegame file to load
	 */
	void LoadSavegame(const std::string& save_file);

	/**
	 * Starts a new game
	 */
	void SetupNewGame();

	/**
	 * Moves the player to the start map.
	 */
	void SetupPlayerSpawn();

	/**
	 * Gets current codepage.
	 *
	 * @return current codepage
	 */
	std::string GetEncoding();

	/** @return If engine is any version of RPG2k */
	bool IsRPG2k();

	/** @return If engine is any version of RPG2k3 */
	bool IsRPG2k3();

	/** @return If engine is RPG2k <= 1.10 */
	bool IsRPG2kLegacy();

	/** @return If engine is RPG2k3 <= v1.04 */
	bool IsRPG2k3Legacy();

	/** @return If engine is RPG2k >= 1.50 */
	bool IsRPG2kUpdated();

	/** @return If engine is RPG2k3 >= 1.05 */
	bool IsRPG2k3Updated();

	/**
	 * @return If engine is the official English RM2k release v.1.61 or newer.
	 * False if engine is RM2k3, Japanese, unofficial or v.1.60.
	 */
	bool IsRPG2kE();

	/** @return If engine is the official English RM2k3 release (v1.10) or newer. */
	bool IsRPG2k3E();

	/** @return If engine is RPG2kLegacy() or RPG2k3Legacy() */
	bool IsLegacy();

	/** @return If engine is RPG2kUpdated() or RPG2k3Updated() */
	bool IsMajorUpdatedVersion();

	/**
	 * @return If engine is the official English release (and not RM2k v.1.60,
	 * which is hard to detect).
	 */
	bool IsEnglish();

	/**
	 * @return true if encoding is CP932 (Shift-JIS, used for Japanese),
	 * false if not
	 */
	bool IsCP932();

	/**
	 * @return true if encoding is CP949 (used for Korean), false if
	 * not
	 */
	bool IsCP949();

	/**
	 * @return true if encoding is Big5 (CP950, used for Traditional
	 * Chinese), false if not
	 */
	bool IsBig5();

	/**
	 * @return true if encoding is CP936 (used for Simplified Chinese)
	 * or false if not
	 */
	bool IsCP936();

	/**
	 * @return true if game is in Chinese, Japanese, or Korean
	 * (based on the encoding), false otherwise
	 */
	bool IsCJK();

	/**
	 * @return true if encoding is CP1251 (used for languages written in
	 * Cyrillic script) or false if not
	 */
	bool IsCP1251();

	/**
	 * @return Running engine version. 2000 for RPG2k and 2003 for RPG2k3
	 */
	int EngineVersion();
	std::string GetEngineVersion();

	/** Output program version on stdout */
	void PrintVersion();

	/** Output program usage information on stdout */
	void PrintUsage();

	/** Set the desired rendering frames per second */
	void SetTargetFps(int fps);

	/** Exit flag, if true will exit application on next Player::Update. */
	extern bool exit_flag;

	/** Reset flag, if true will restart game on next Player::Update. */
	extern bool reset_flag;

	/** Debug flag, if true will run game in debug mode. */
	extern bool debug_flag;

	/** Hide Title flag, if true title scene will run without image and music. */
	extern bool hide_title_flag;

	/** Window flag, if true will run in window mode instead of full screen. */
	extern bool window_flag;

	/** FPS flag, if true will display frames per second counter. */
	extern bool fps_flag;

	/** FPS render window flag, if true will draw the FPS on the screen even in windowed mode. */
	extern bool fps_render_window;

	/** Mouse flag, if true enables mouse click and scroll wheel */
	extern bool mouse_flag;

	/** Touch flag, if true enables finger taps */
	extern bool touch_flag;

	/** Overwrite party x position */
	extern int party_x_position;

	/** Overwrite party y position */
	extern int party_y_position;

	/** Overwrite starting party members */
	extern std::vector<int> party_members;

	/** Overwrite start map */
	extern int start_map_id;

	/** New game flag, if true a new game starts directly. */
	extern bool new_game_flag;

	/** If set, savegame is loaded directly */
	extern int load_game_id;

	/** Prevent adding of RTP paths to the file finder */
	extern bool no_rtp_flag;

	/** Mutes audio playback */
	extern bool no_audio_flag;

	/** Encoding used */
	extern std::string encoding;

	/** Backslash recoded to utf8 string */
	extern std::string escape_symbol;

	/** Backslash recoded to character */
	extern uint32_t escape_char;

	/** Currently interpreted engine. */
	extern int engine;

	/** Path to replay input log from */
	extern std::string replay_input_path;

	/** Path to record input log to */
	extern std::string record_input_path;

	/** Game title. */
	extern std::string game_title;

	/** Meta class containing additional external data for this game. */
	extern std::shared_ptr<Meta> meta;

	/**
	 * The default speed modifier applied when the speed up button is pressed
	 *  Only used for configuring the speedup, don't read this var directly use
	 *  GetSpeedModifier() instead.
	 */
	extern int speed_modifier;

#ifdef EMSCRIPTEN
	/** Name of game emscripten uses */
	extern std::string emscripten_game_name;
#endif

#ifdef _3DS
	/** Is executed from a .3dsx (otherwise .cia) */
	extern bool is_3dsx;
#endif

	/**
	 * Vsync mode was requested. This does not mean it's actually being used.
	 * One must ask the DisplayUi if it's configured for vsync.
	 */
	extern bool vsync;
}

inline bool Player::IsRPG2k() {
	return (engine & EngineRpg2k) == EngineRpg2k;
}

inline bool Player::IsRPG2k3() {
	return (engine & EngineRpg2k3) == EngineRpg2k3;
}

inline bool Player::IsRPG2kLegacy() {
	return engine == EngineRpg2k;
}

inline bool Player::IsRPG2k3Legacy() {
	return engine == EngineRpg2k3;
}

inline bool Player::IsLegacy() {
	return IsRPG2kLegacy() || IsRPG2k3Legacy();
}

inline bool Player::IsMajorUpdatedVersion() {
	return (engine & EngineMajorUpdated) == EngineMajorUpdated;
}

inline bool Player::IsEnglish() {
	return (engine & EngineEnglish) == EngineEnglish;
}

inline bool Player::IsRPG2kUpdated() {
	return (IsRPG2k() && IsMajorUpdatedVersion());
}

inline bool Player::IsRPG2k3Updated() {
	return (IsRPG2k3() && IsMajorUpdatedVersion());
}

inline bool Player::IsRPG2kE() {
	return (IsRPG2k() && IsEnglish());
}

inline bool Player::IsRPG2k3E() {
	return (IsRPG2k3() && IsEnglish());
}

#endif
