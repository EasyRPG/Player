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

#ifndef EP_SCENE_TITLE_H
#define EP_SCENE_TITLE_H

// Headers
#include "scene.h"
#include "sprite.h"
#include "window_command.h"
#include "async_handler.h"

/**
 * Scene Title class.
 */
class Scene_Title : public Scene {
public:
	/**
	 * Constructor.
	 */
	Scene_Title();

	void Start() override;
	void Continue() override;
	void TransitionIn() override;
	void Resume() override;
	void Update() override;

	/**
	 * Loads all databases.
	 */
	void LoadDatabase();

	/**
	 * Creates the background graphic of the scene.
	 */
	void CreateTitleGraphic();

	/**
	 * Creates the Window displaying the options.
	 */
	void CreateCommandWindow();

	/**
	 * Plays the title music.
	 */
	void PlayTitleMusic();

	/**
	 * Checks if there is a player start location.
	 *
	 * @return true if there is one, false otherwise.
	 */
	bool CheckValidPlayerLocation();

	/**
	 * Initializes a battle test session.
	 */
	void PrepareBattleTest();

	/**
	 * Option New Game.
	 * Starts a new game.
	 */
	void CommandNewGame();

	/**
	 * Option Continue.
	 * Shows the Load-Screen (Scene_Load).
	 */
	void CommandContinue();

	/**
	 * Option Shutdown.
	 * Does a player shutdown.
	 */
	void CommandShutdown();

private:
	void OnTitleSpriteReady(FileRequestResult* result);

	/** Displays the options of the title scene. */
	std::unique_ptr<Window_Command> command_window;

	/** Background graphic. */
	std::unique_ptr<Sprite> title;

	/** Contains the state of continue button. */
	bool continue_enabled;

	FileRequestBinding request_id;
};

#endif
