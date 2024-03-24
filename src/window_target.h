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

#ifndef EP_WINDOW_ACTORINFO_H
#define EP_WINDOW_ACTORINFO_H

// Headers
#include "window_command.h"

/**
 * Window ActorInfo Class.
 * Displays the left hand information window in the status
 * scene.
 */
class Window_Target : public Window_Command {
public:
	/**
	 * Constructor.
	 */
	Window_Target(Scene* parent, std::vector<std::string> commands, int width = -1, int max_item = -1);

	virtual bool ExcludeForMouse() const {
		return true;
	}

	/**
	 * Updates the window state.
	 */
	void Update() override;
};

#endif
