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

#ifndef EP_WINDOW_IMPORT_PROGRESS_H
#define EP_WINDOW_IMPORT_PROGRESS_H

// Headers
#include "window_base.h"
#include "text.h"

/**
 * Window_ImportProgress class.
 * Used to inform the user of progress as save files are scanned for Scene_Import
 */
class Window_ImportProgress : public Window_Base {

public:
	/**
	 * Constructor.
	 * @param ix x position of the window
	 * @param iy y position of the window
	 * @param iwidth width of the window
	 * @param iheight height of the window
	 */
	Window_ImportProgress(Scene* parent, int ix, int iy, int iwidth, int iheight);

	/**
	 * Update to display scanning progress and the directory currently being scanned
	 * @param pct percent of the scan that is complete
	 * @param path current directory being scanned
	 */
	void SetProgress(int pct, const std::string& path);

private:
	void Refresh();

	/** Current progress as a percentage */
	int percent = 0;

	/** Current file/path being scanned */
	std::string curr_path;
};

#endif
