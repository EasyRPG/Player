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

#ifndef EP_WINDOW_COMMAND_HORIZONTAL_H
#define EP_WINDOW_COMMAND_HORIZONTAL_H

// Headers
#include <vector>
#include "window_command.h"
#include "font.h"

/**
 * Like Window Command but the menu items are layed out horizontal, not vertical.
 */
class Window_Command_Horizontal: public Window_Command {
public:
	/**
	 * Constructor.
	 *
	 * @param commands commands to display.
	 * @param width window width, if no width is passed
	 *              the width is autocalculated.
	 */
	Window_Command_Horizontal(Scene* parent, std::vector<std::string> commands, int width = -1);

	/**
	 * Replace all commands with a new command set.
	 *
	 * @param commands the commands to replace with
	 * @note auto-generating width and height is not supported.
	 */
	void ReplaceCommands(std::vector<std::string> commands);

protected:
	void DrawItem(int index, Font::SystemColor color) override;
};

#endif
