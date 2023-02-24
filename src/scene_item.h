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

#ifndef EP_SCENE_ITEM_H
#define EP_SCENE_ITEM_H

// Headers
#include "scene.h"
#include "window_help.h"
#include "window_item.h"

/**
 * Scene_Item class.
 */
class Scene_Item : public Scene {

public:
	/**
	 * Constructor.
	 *
	 * @param item_index index to select.
	 */
	Scene_Item(int item_index = 0);

	void Start() override;
	void Continue(SceneType prev_scene) override;
	void vUpdate() override;
	void TransitionOut(Scene::SceneType next_scene) override;

private:
	/** Displays description about the selected item. */
	std::unique_ptr<Window_Help> help_window;
	/** Displays available items. */
	std::unique_ptr<Window_Item> item_window;
	/** Index of item selected on startup. */
	int item_index;
};

#endif
