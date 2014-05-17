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

/**
 * Player namespace.
 */
namespace Player {
	enum EngineType {
		EngineRpg2k,
		EngineRpg2k3
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
	 * Pauses the game engine.
	 */
	void Pause();

	/**
	 * Resumes the game engine.
	 */
	void Resume();

	/**
	 * Updates EasyRPG Player.
	 */
	void Update();

	/**
	 * Exits EasyRPG Player.
	 */
	void Exit();

	/**
	 * Parses the command line arguments.
	 */
	void ParseCommandLine(int argc, char *argv[]);

	/**
	 * (Re)Initializes all game objects
	 */
	void CreateGameObjects();

	/**
	 * Loads all databases.
	 */
	void LoadDatabase();

	/**
	 * Moves the player to the start map.
	 */
	void SetupPlayerSpawn();

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

	/** Battle Test flag, if true will run battle test. */
	extern bool battle_test_flag;

	/** Battle Test Troop ID to fight with if battle test is run. */
	extern int battle_test_troop_id;

	/** Overwrite party x position */
	extern int party_x_position;

	/** Overwrite porty y position */
	extern int party_y_position;

	/** Overwrite start map */
	extern int start_map_id;

	/** New game flag, if true a new game starts directly. */
	extern bool new_game_flag;

	/** Mutes audio playback */
	extern bool no_audio_flag;

	/** Currently interpreted engine. */
	extern EngineType engine;

	/** Game title. */
	extern std::string game_title;
}

#endif
