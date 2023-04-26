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

#ifndef EP_WINDOW_GAMELIST_H
#define EP_WINDOW_GAMELIST_H

// Headers
#include <vector>
#include "window_help.h"
#include "window_selectable.h"
#include "filefinder.h"

/**
 * Window_GameList class.
 */
class Window_GameList : public Window_Selectable {

public:
	/**
	 * Constructor.
	 */
	Window_GameList(int ix, int iy, int iwidth, int iheight);

	/**
	 * Refreshes the game list.
	 */
	bool Refresh(FilesystemView filesystem_base, bool show_dotdot);

	/**
	 * Draws an item together with the quantity.
	 *
	 * @param index index of item to draw.
	 */
	void DrawItem(int index);

	void DrawErrorText(bool show_dotdot);

	/**
	 * @return true if at least one entry is in the directory
	 */
	bool HasValidEntry();

	/**
	 * @return filesystem and entry name of the selected game
	 */
	std::pair<FilesystemView, std::string> GetGameFilesystem() const;

private:
	FilesystemView base_fs;
	std::vector<std::string> game_directories;

	bool show_dotdot = false;
};

#endif
