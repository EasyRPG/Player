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

#ifndef EP_SCENE_END_H
#define EP_SCENE_END_H

// Headers
#include "scene.h"
#include "scene_title.h"
#include "window_command.h"
#include "window_help.h"

/**
 * Scene End class.
 * Displays the "Do you really want to exit?" text.
 */
class Scene_End : public Scene {

public:
	/**
	 * Constructor.
	 *
	 * @param target_scene Scene entered after selecting "Yes"
	 */
	Scene_End(SceneType target_scene = Scene::Title);

	void Start() override;
	void vUpdate() override;

	/**
	 * Creates the Window displaying the yes and no option.
	 */
	void CreateCommandWindow();

	/**
	 * Creates the Window displaying the confirmation
	 * text.
	 */
	void CreateHelpWindow();

private:
	/** Help window showing the confirmation text. */
	std::unique_ptr<Window_Help> help_window;
	/** Command window containing the yes and no option. */
	std::unique_ptr<Window_Command> command_window;

	SceneType target_scene;
};

#endif
