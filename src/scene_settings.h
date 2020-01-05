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
	void Update() override;

	/**
	 * Updates the range list window.
	 */
	void UpdateRangeListWindow();

	/**
	 * Updates the var list window.
	 */
	void UpdateVarListWindow();

	/**
	 * Updates the item window.
	 */
	void UpdateItemSelection();

	/**
	 * Gets an int with the current switch/variable selected.
	 */
	int GetIndex();

	/**
	 * Resets the remembered indices
	 */
	static void ResetPrevIndices();

private:
	enum Mode {
		eMain,
		eInput,
		eVideo,
		eAudio,
	};
	Mode mode = eMain;

	void CreateMainWindow();

	void UpdateMain();
	void UpdateInput();
	void UpdateVideo();
	void UpdateAudio();

	/** Displays a range selection for mode. */
	std::unique_ptr<Window_Command> main_window;
};

#endif
