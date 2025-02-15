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
	void Continue(SceneType prev_scene) override;
	void TransitionIn(SceneType prev_scene) override;
	void Suspend(SceneType next_scene) override;
	void vUpdate() override;
	void Refresh() override;

	void OnTranslationChanged() override;

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
	 * Checks if game or engine configuration requires usage of title
	 * graphic and music or not.
	 *
	 * @return true when graphic and music are shown
	 */
	bool CheckEnableTitleGraphicAndMusic();

	/**
	 * @return Whether the 2k3E chunk for showing the title is set
	 */
	bool Check2k3ShowTitle();

	/**
	 * Checks if there is a player start location.
	 *
	 * @return true if there is one, false otherwise.
	 */
	bool CheckValidPlayerLocation();

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
	 * Option Import.
	 * Shows the Import screen, for use with multi-game save files.
	 */
	void CommandImport();

	/**
	 * Options the settings to configure the Player.
	 */
	void CommandSettings();

	/**
	 * Option Translation.
	 * Shows the Translation menu, for picking between multiple languages or localizations
	 */
	void CommandTranslation();

	/**
	 * Option Shutdown.
	 * Does a player shutdown.
	 */
	void CommandShutdown();

	/**
	 * Invoked when a new game is started or a save game is loaded.
	 */
	void OnGameStart();

	/**
	 * Moves a window (typically the New/Continue/Quit menu) to the middle or bottom-center of the screen.
	 * @param window The window to resposition.
	 * @param center_vertical If true, the menu will be centered vertically. Otherwise, it will be at the bottom of the screen.
	 */
	static void RepositionWindow(Window_Command& window, bool center_vertical);

private:
	void OnTitleSpriteReady(FileRequestResult* result);

	/** Displays the options of the title scene. */
	std::unique_ptr<Window_Command> command_window;

	/** Background graphic. */
	std::unique_ptr<Sprite> title;

	/**
	 * Offsets for each selection, in case "Import" or "Translate" is enabled.
	 *   Listed in the order they may appear; exit_index will always be last,
	 *   and import appears before translate, if it exists.
	 * Stored in a struct for easy resetting, as Scene_Title can be reused.
	 */
	struct CommandIndices {
		int new_game =  0;
		int continue_game =  1;
		int import = -1;
		int settings = -1;
		int translate = -1;
		int exit =  2;
	};
	CommandIndices indices;

	/** Contains the state of continue button. */
	bool continue_enabled = false;

	bool restart_title_cache = false;

	FileRequestBinding request_id;
};

#endif
