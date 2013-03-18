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

#ifndef _SCENE_GAMEOVER_H_
#define _SCENE_GAMEOVER_H_

// Headers
#include "scene.h"
#include "sprite.h"
#include <boost/scoped_ptr.hpp>

/**
 * Scene Game Over class.
 * Displays the Game Over screen.
 */
class Scene_Gameover : public Scene {

public:
	/**
	 * Constructor.
	 */
	Scene_Gameover();

	void Start();
	void Update();

private:
	/** Background graphic. */
	boost::scoped_ptr<Sprite> background;
};

#endif
