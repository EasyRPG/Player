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

#ifndef _SCENE_ORDER_H_
#define _SCENE_ORDER_H_

// Headers
#include "scene.h"
#include "window_command.h"
#include <boost/scoped_ptr.hpp>

/**
 * Scene Order class.
 * Allows changing the order of the party actors.
 */
class Scene_Order : public Scene {
public:
	/**
	 * Constructor.
	 */
	Scene_Order();

	void Start();
	void Terminate();
	void Update();

private:
	/** Contains new actor order or 0 if actor wasnt rearranged yet. */
	std::vector<int> actors;
	/** Contains number of rearranged actors. */
	int actor_counter;

	boost::scoped_ptr<Window_Command>
		window_left, window_right, window_confirm;

	void CreateCommandWindow();

	void UpdateOrder();
	void UpdateConfirm();

	void Redo();
	void Confirm();
};

#endif
