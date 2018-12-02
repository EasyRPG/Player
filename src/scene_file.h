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

#ifndef EP_SCENE_FILE_H
#define EP_SCENE_FILE_H

// Headers
#include <vector>
#include "scene.h"
#include "filefinder.h"
#include "window_help.h"
#include "window_savefile.h"

/**
 * Base class used by the save and load scenes.
 */
class Scene_File : public Scene {

public:
	/**
	 * Constructor.
	 *
	 * @param message title message.
	 */
	Scene_File(std::string message);

	void Start() override;
	void Update() override;

	virtual void Action(int index) = 0;

	virtual bool IsSlotValid(int index) = 0;

	bool IsWindowMoving() const;

protected:
	void Refresh();
	void MoveFileWindows(int dy, int dt);

	int index;
	int top_index;
	std::unique_ptr<Window_Help> help_window;
	std::vector<std::shared_ptr<Window_SaveFile> > file_windows;
	std::unique_ptr<Sprite> border_top;
	std::unique_ptr<Sprite> border_bottom;
	std::string message;

	std::shared_ptr<FileFinder::DirectoryTree> tree;

	double latest_time;
	int latest_slot;
};

#endif
