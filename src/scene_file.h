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
#include "filefinder.h"
#include <lcf/rpg/save.h>
#include "scene.h"
#include "window_help.h"
#include "window_savefile.h"
#include "sprite.h"


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
	void vUpdate() override;

	virtual void Action(int index) = 0;

	virtual bool IsSlotValid(int index) = 0;

	bool IsWindowMoving() const;

protected:
	virtual void CreateHelpWindow();
	virtual void PopulateSaveWindow(Window_SaveFile& win, int id);
	virtual void PopulatePartyFaces(Window_SaveFile& win, int id, lcf::rpg::Save& savegame);
	virtual void UpdateLatestTimestamp(int id, lcf::rpg::Save& savegame);
	static std::unique_ptr<Sprite> MakeBorderSprite(int y);
	static std::unique_ptr<Sprite> MakeArrowSprite(bool down);

	void Refresh();
	void MoveFileWindows(int dy, int dt);
	void UpdateArrows();

	int index = 0;
	int top_index = 0;
	std::unique_ptr<Window_Help> help_window;
	std::vector<std::shared_ptr<Window_SaveFile> > file_windows;
	std::unique_ptr<Sprite> border_top;
	std::unique_ptr<Sprite> border_bottom;
	std::unique_ptr<Sprite> up_arrow;
	std::unique_ptr<Sprite> down_arrow;
	std::string message;

	FilesystemView fs;

	double latest_time = 0;
	int latest_slot = 0;

	int arrow_frame = 0;
};

#endif
