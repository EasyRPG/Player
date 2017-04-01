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

#ifndef _PLAYER_H_
#define _PLAYER_H_

// Headers
#include "baseui.h"
#include <vector>

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
		/** RPG Maker 2003 v1.10 or newer (Official English translation) */
		EngineRpg2k3E = 8
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
	 * Returns executed game frames since player start.
	 * Should be 60 fps when game ran fast enough.
	 *
	 * @return Update frames since player start
	 */
	int GetFrames();

	/**
	 * Resets the fps count (both updates and frames per second).
	 * Should be called after an expensive operation.
	 */
	void FrameReset();

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
	 * Moves the player to the start map.
	 */
	void SetupPlayerSpawn();

	/**
	 * Gets current codepage.
	 *
	 * @return current codepage
	 */
	std::string GetEncoding();

	/**
	 * @return Whether engine is RPG2k
	 */
	bool IsRPG2k();

	/**
	 * @return If engine is RPG2k3 v1.09a or older
	 */
	bool IsRPG2k3Legacy();

	/**
	 * @return If engine is RPG2k3
	 */
	bool IsRPG2k3();

	/**
	 * @return If engine is RPG2k v1.50 or newer, or RPG2k3 v1.05 or newer
	 */
	bool IsMajorUpdatedVersion();

	/**
	 * @return If engine is the official English release (v1.10) or newer.
	 */
	bool IsRPG2k3E();

	/**
	 * @return if encoding is CP932 or not
	 */
	bool IsCP932();

	/**
	 * @return Returns how fast EasyRPG currently runs (1: Normal speed, 2: double speed, 5: 5x speed, ...)
	 */
	int GetSpeedModifier();

	/** Output program version on stdout */
	void PrintVersion();

	/** Output program usage information on stdout */
	void PrintUsage();

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

	/** Mouse flag, if true enables mouse click and scroll wheel */
	extern bool mouse_flag;

	/** Touch flag, if true enables finger taps */
	extern bool touch_flag;

	/** Battle Test flag, if true will run battle test. */
	extern bool battle_test_flag;

	/** Battle Test Troop ID to fight with if battle test is run. */
	extern int battle_test_troop_id;

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

	/** Backslash recoded */
	extern std::string escape_symbol;

	/** Currently interpreted engine. */
	extern int engine;

	/** Path to replay input log from */
	extern std::string replay_input_path;

	/** Path to record input log to */
	extern std::string record_input_path;

	/** Game title. */
	extern std::string game_title;

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
}

#endif
