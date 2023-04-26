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

#ifndef EP_SCENE_LOGO_H
#define EP_SCENE_LOGO_H

// Headers
#include "system.h"
#include "scene.h"
#include "sprite.h"
#include "async_handler.h"

/**
 * Scene Logo class.
 * Displays the shiny EasyRPG logo on startup and inititalizes the game.
 * When the startup directory does not contain a game it loads the Game Browser
 * instead.
 */
class Scene_Logo : public Scene {

public:
	/**
	 * Constructor.
	 */
	Scene_Logo();

	void Start() override;
	void vUpdate() override;
	void DrawBackground(Bitmap& dst) override;
	void DrawText(bool verbose);

private:
	std::unique_ptr<Sprite> logo;
	BitmapRef logo_img;
	int frame_counter;

	void OnIndexReady(FileRequestResult* result);
	FileRequestBinding request_id;
	bool async_ready = false;
};

#endif
