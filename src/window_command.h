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

#ifndef EP_WINDOW_COMMAND_H
#define EP_WINDOW_COMMAND_H

// Headers
#include <vector>
#include "window_selectable.h"
#include "font.h"

/**
 * Window Command class.
 */
class Window_Command: public Window_Selectable {
public:
	/**
	 * Constructor.
	 *
	 * @param commands commands to display.
	 * @param width window width, if no width is passed
	 *              the width is autocalculated.
	 * @param max_item forces a window height for max_item
	 *                 items, if no height is passed
	 *                 the height is autocalculated.
	 */
	Window_Command(Scene* parent, std::vector<std::string> commands, int width = -1, int max_item = -1);

	/**
	 * Refreshes the window contents.
	 */
	void Refresh();

	/**
	 * Disables a command.
	 *
	 * @param index command index.
	 */
	void DisableItem(int index);

	/**
	 * Enables a command.
	 *
	 * @param index command index.
	 */
	void EnableItem(int index);

	/**
	 * Enables or disables a command.
	 *
	 * @param index command index.
	 * @param enabled true: enable, false: disable
	 */
	void SetItemEnabled(int index, bool enabled);

	/**
	 * Tests if an item is enabled.
	 *
	 * @param index command index.
	 * @return if enabled.
	 */
	bool IsItemEnabled(int index);

	/**
	 * Replaces the text of an item.
	 *
	 * @param index command index.
	 * @param text new item text.
	 */
	void SetItemText(unsigned index, StringView text);

	/**
	 * Replace all commands with a new command set.
	 *
	 * @param commands the commands to replace with
	 * @note auto-generating width and height is not supported.
	 */
	void ReplaceCommands(std::vector<std::string> commands);

protected:
	std::vector<std::string> commands;
	std::vector<bool> commands_enabled;

	virtual void DrawItem(int index, Font::SystemColor color);
};

#endif
