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

#ifndef EP_SCENE_DEBUG_PICTURE_H
#define EP_SCENE_DEBUG_PICTURE_H

#include "scene.h"
#include "window_debug_picture.h"
#include <memory>

class Scene_DebugPicture : public Scene {
public:
	Scene_DebugPicture();
	void Start() override;
	void vUpdate() override;

private:
	std::unique_ptr<Window_DebugPictureList> list_window;
	std::unique_ptr<Window_DebugPictureInfo> info_window;
};

#endif
