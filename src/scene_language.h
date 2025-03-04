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

#ifndef EP_SCENE_LANGUAGE_H
#define EP_SCENE_LANGUAGE_H

 // Headers
#include <vector>
#include "scene.h"
#include "window_command.h"
#include "window_command_horizontal.h"
#include "window_about.h"
#include "window_selectable.h"
#include "window_settings.h"
#include "window_input_settings.h"
#include "async_handler.h"
#include "sprite.h"
#include "game_config.h"

/**
 * Scene allowing configuration of system state.
 */
class Scene_Language : public Scene {

public:
	/**
	 * Constructor.
	 */
	Scene_Language();

	void Start() override;
	void vUpdate() override;

	void OnTranslationChanged() override;

private:

	/**
	 * Creates the Window displaying available translations.
	 */
	void CreateTranslationWindow();

	/**
	 * Creates the Help window and hides it
	 */
	void CreateHelpWindow();

	/**
	 * Picks a new language based and switches to it.
	 * @param lang_str If the empty string, switches the game to 'No Translation'. Otherwise, switch to that translation by name.
	 */
	void ChangeLanguage(const std::string& lang_str);

	void CreateTitleGraphic();
	void OnTitleSpriteReady(FileRequestResult* result);

	void PopOrTitle();

	/** Displays all available translations (languages). */
	std::unique_ptr<Window_Command> translate_window;

	/** Displays help text for a given language **/
	std::unique_ptr<Window_Help> help_window;

	/** Contains directory names for each language; entry 0 is resverd for the default (no) translation */
	std::vector<std::string> lang_dirs;

	/** Contains help strings for each language; entry 0 is resverd for the default (no) translation */
	std::vector<std::string> lang_helps;

	std::unique_ptr<Sprite> title;
	FileRequestBinding request_id;
	int input_reset_counter = 0;
	bool shutdown = false;

	Window_Settings::UiMode mode = Window_Settings::eNone;
};


#endif
