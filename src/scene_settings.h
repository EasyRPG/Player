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

#ifndef EP_SCENE_SYSTEM_H
#define EP_SCENE_SYSTEM_H

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
class Scene_Settings : public Scene {

public:
	/**
	 * Constructor.
	 */
	Scene_Settings();

	void Start() override;
	void vUpdate() override;

	/**
	 * Saves the configuration to the global config file.
	 * @param silent When true only log messages instead of displaying them
	 * @return whether saving was successful
	 */
	static bool SaveConfig(bool silent = false);

private:
	void CreateMainWindow();
	void CreateOptionsWindow();

	void CreateTitleGraphic();
	void OnTitleSpriteReady(FileRequestResult* result);

	void SetMode(Window_Settings::UiMode new_mode);

	void UpdateMain();
	void UpdateOptions();
	void UpdateButtonOption();
	void UpdateButtonAdd();
	void UpdateButtonRemove();

	bool RefreshInputEmergencyReset();
	void RefreshInputActionAllowed();

	std::unique_ptr<Window_Command> main_window;
	std::unique_ptr<Window_Help> help_window;
	std::unique_ptr<Window_About> about_window;
	std::unique_ptr<Window_Settings> options_window;
	std::unique_ptr<Window_InputSettings> input_window;
	std::unique_ptr<Window_Help> input_help_window;
	std::unique_ptr<Window_Command_Horizontal> input_mode_window;
	std::unique_ptr<Window_Command> picker_window;
	std::unique_ptr<Window_NumberInput> number_window;

	std::unique_ptr<Sprite> title;
	FileRequestBinding request_id;
	int input_reset_counter = 0;

	Window_Settings::UiMode mode = Window_Settings::eNone;
};


#endif
