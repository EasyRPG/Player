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

#ifndef EP_SCENE_IMPORT_H
#define EP_SCENE_IMPORT_H

// Headers
#include <memory>
#include <vector>
#include <string>
#include "meta.h"
#include "scene_file.h"
#include "window_import_progress.h"

/**
 * Scene_Item class.
 */
class Scene_Import : public Scene_File {

public:
	Scene_Import();

	void Start() override;
	void vUpdate() override;

	void Action(int index) override;
	bool IsSlotValid(int index) override;

protected:
	virtual void PopulateSaveWindow(Window_SaveFile& win, int id) override;

private:
	void UpdateScanAndProgress();
	void FinishScan();

	/** Visually track scanning of other game folders */
	std::unique_ptr<Window_ImportProgress> progress_window;

	/** Collection of all folders in ../ */
	FilesystemView parent_fs;

	/** Tracking status: vector of child folders to check and current index in that list */
	std::vector<std::string> children;
	size_t curr_child_id = 0;

	bool first_frame_skipped = false;

	/** Final file list; used for the Import windows */
	std::vector<Meta::FileItem> files;
};

#endif
