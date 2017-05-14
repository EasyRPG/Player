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

#ifndef _SCENE_NAME_H_
#define _SCENE_NAME_H_

#include "scene.h"
#include "window_name.h"
#include "window_face.h"
#include "window_keyboard.h"

/**
 * Scene_Item class.
 */
class Scene_Name : public Scene {

public:
	/**
	 * Constructor.
	 */
	Scene_Name();

	void Start() override;
	void Update() override;

private:
	std::unique_ptr<Window_Keyboard> kbd_window;
	std::unique_ptr<Window_Name> name_window;
	std::unique_ptr<Window_Face> face_window;
};

#endif
