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

// Headers
#include <sstream>
#include "filefinder.h"
#include "game_system.h"
#include "input.h"
#include <lcf/lsd/reader.h>
#include "output.h"
#include "player.h"
#include "scene_file.h"
#include "scene_import.h"

Scene_Import::Scene_Import() :
	Scene_File(Player::meta->GetExVocabImportSaveHelpText()) {
	Scene::type = Scene::Load;  // For all intents and purposes, treat Import as an extension of Load
}

void Scene_Import::PopulateSaveWindow(Window_SaveFile& win, int id) {
	// File access is already determined
	if (id < static_cast<int>(files.size())) {
		win.SetDisplayOverride(files[id].short_path, files[id].file_id);

		std::unique_ptr<lcf::rpg::Save> savegame =
			lcf::LSD_Reader::Load(files[id].full_path, Player::encoding);

		if (savegame.get()) {
			PopulatePartyFaces(win, id, *savegame);
			UpdateLatestTimestamp(id, *savegame);
		} else {
			win.SetCorrupted(true);
		}
	} else {
		win.SetDisplayOverride("No lcf::Data", 0);
	}
}

void Scene_Import::Start() {
	CreateHelpWindow();
	border_top = Scene_File::MakeBorderSprite(32);

	// For consistency, we only show 15 windows
	// We don't populate them until later (once we've loaded all potential importable files).
	for (int i = 0; i < 15; i++) {
		std::shared_ptr<Window_SaveFile>
			w = std::make_unique<Window_SaveFile>(this, 0, 40 + i * 64, Player::screen_width, 64);
		w->SetIndex(i);
		w->SetVisible(false);
		w->SetZ(Priority_Window);

		file_windows.push_back(w);
	}

	// Create a window to show scanning progress, since this can take a while.
	progress_window = std::make_unique<Window_ImportProgress>(this, Player::screen_width/4, 40 + 64, Player::screen_width/2, 64);
	progress_window->SetZ(Priority_Window + 1);

	border_bottom = Scene_File::MakeBorderSprite(232);

	index = latest_slot;
	top_index = std::max(0, index - 2);

	Scene_File::Start();

	Refresh();
	Update();
}

void Scene_Import::vUpdate() {
	if (progress_window->IsVisible()) {
		UpdateScanAndProgress();
		return;
	}

	Scene_File::vUpdate();
}

void Scene_Import::UpdateScanAndProgress() {
	// Every tick, we still check for user input specifically for canceling...
	if (Input::IsTriggered(Input::CANCEL)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
		Scene::Pop();
		return;
	}

	// Leads to better fading.
	if (!first_frame_skipped) {
		first_frame_skipped = true;
		return;
	}

	// Gather the list of children, if it does not exist.
	if (children.empty()) {
		std::string parentPath = FileFinder::Save().MakePath("../");
		if (FileFinder::Save().Exists("../")) {
			parent_fs = FileFinder::Root().Create(parentPath);
			if (parent_fs) {
				children = Player::meta->GetImportChildPaths(parent_fs);
			}
		}
		if (children.empty()) {
			FinishScan();
		}
	} else if (curr_child_id < children.size()) {
		auto candidates = Player::meta->SearchImportPaths(parent_fs, children[curr_child_id]);
		files.insert(files.end(), candidates.begin(), candidates.end());

		progress_window->SetProgress((curr_child_id*100)/children.size(), children[curr_child_id]);
		curr_child_id += 1;
	} else {
		FinishScan();
	}
}

void Scene_Import::FinishScan() {
	for (int i = 0; i < 15; i++) {
		auto w = file_windows[i];
		PopulateSaveWindow(*w, i);
		w->Refresh();
		w->SetVisible(true);
	}
	progress_window->SetVisible(false);
}

void Scene_Import::Action(int index) {
	Player::LoadSavegame(files[index].full_path);
}

bool Scene_Import::IsSlotValid(int index) {
	return index < static_cast<int>(files.size());
}
