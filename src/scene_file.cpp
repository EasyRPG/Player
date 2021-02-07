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
#include <algorithm>
#include <sstream>
#include <vector>
#include "baseui.h"
#include "cache.h"
#include <lcf/data.h>
#include "game_system.h"
#include "game_party.h"
#include "input.h"
#include <lcf/lsd/reader.h>
#include "player.h"
#include "scene_file.h"
#include "bitmap.h"
#include <lcf/reader_util.h>

Scene_File::Scene_File(std::string message) :
	message(message) {
}

std::unique_ptr<Sprite> Scene_File::MakeBorderSprite(int y) {
	auto bitmap = Bitmap::Create(SCREEN_TARGET_WIDTH, 8, Cache::System()->GetBackgroundColor());
	auto sprite = std::unique_ptr<Sprite>(new Sprite());
	sprite->SetVisible(true);
	sprite->SetZ(Priority_Window + 1);
	sprite->SetBitmap(bitmap);
	sprite->SetX(0);
	sprite->SetY(y);
	return sprite;
}

void Scene_File::CreateHelpWindow() {
	help_window.reset(new Window_Help(0, 0, SCREEN_TARGET_WIDTH, 32));
	help_window->SetText(message);
	help_window->SetZ(Priority_Window + 1);
}

void Scene_File::PopulatePartyFaces(Window_SaveFile& win, int /* id */, lcf::rpg::Save& savegame) {
	win.SetParty(savegame.title);
	win.SetHasSave(true);
}

void Scene_File::UpdateLatestTimestamp(int id, lcf::rpg::Save& savegame) {
	if (savegame.title.timestamp > latest_time) {
		latest_time = savegame.title.timestamp;
		latest_slot = id;
	}
}

void Scene_File::PopulateSaveWindow(Window_SaveFile& win, int id) {
	// Try to access file
	std::stringstream ss;
	ss << "Save" << (id <= 8 ? "0" : "") << (id + 1) << ".lsd";

	std::string file = tree->FindFile(ss.str());

	if (!file.empty()) {
		// File found
		auto save_stream = FileFinder::OpenInputStream(file);
		std::unique_ptr<lcf::rpg::Save> savegame = lcf::LSD_Reader::Load(save_stream, Player::encoding);

		if (savegame) {
			PopulatePartyFaces(win, id, *savegame);
			UpdateLatestTimestamp(id, *savegame);
		} else {
			win.SetCorrupted(true);
		}
	}
}

void Scene_File::Start() {
	CreateHelpWindow();
	border_top = Scene_File::MakeBorderSprite(32);

	// Refresh File Finder Save Folder
	tree = FileFinder::CreateSaveDirectoryTree();

	for (int i = 0; i < 15; i++) {
		std::shared_ptr<Window_SaveFile>
			w(new Window_SaveFile(0, 40 + i * 64, SCREEN_TARGET_WIDTH, 64));
		w->SetIndex(i);
		w->SetZ(Priority_Window);
		PopulateSaveWindow(*w, i);
		w->Refresh();

		file_windows.push_back(w);
	}

	border_bottom = Scene_File::MakeBorderSprite(232);

	index = latest_slot;
	top_index = std::max(0, index - 2);

	Refresh();

	for (auto& fw: file_windows) {
		fw->Update();
	}
}

void Scene_File::Refresh() {
	for (int i = 0; i < (int)file_windows.size(); i++) {
		Window_SaveFile *w = file_windows[i].get();
		w->SetY(40 + (i - top_index) * 64);
		w->SetActive(i == index);
		w->Refresh();
	}
}

void Scene_File::Update() {
	if (IsWindowMoving()) {
		for (auto& fw: file_windows) {
			fw->Update();
		}
		return;
	}

	if (Input::IsTriggered(Input::CANCEL)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
		Scene::Pop();
	} else if (Input::IsTriggered(Input::DECISION)) {
		if (IsSlotValid(index)) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
			Action(index);
		}
		else {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
		}
	}

	int old_top_index = top_index;
	int old_index = index;
	int max_index = static_cast<int>(file_windows.size()) - 1;

	if (Input::IsRepeated(Input::DOWN) || Input::IsTriggered(Input::SCROLL_DOWN)) {
		if (Input::IsTriggered(Input::DOWN) || Input::IsTriggered(Input::SCROLL_DOWN)
			|| index < max_index) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
			index = (index + 1) % file_windows.size();
		}

		//top_index = std::max(top_index, index - 3 + 1);
	}
	if (Input::IsRepeated(Input::UP) || Input::IsTriggered(Input::SCROLL_UP)) {
		if (Input::IsTriggered(Input::UP) || Input::IsTriggered(Input::SCROLL_UP)
			|| index >= 1) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
			index = (index + max_index) % file_windows.size();
		}

		//top_index = std::min(top_index, index);
	}

	if (Input::IsRepeated(Input::PAGE_DOWN) && index < max_index) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
		index = (index + 3 <= max_index) ? index + 3 : max_index;
	}
	if (Input::IsRepeated(Input::PAGE_UP) && index >= 1) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
		index = (index > 3) ? index - 3 : 0;
	}

	if (index > top_index + 2) {
		MoveFileWindows((top_index + 2 - index) * 64, 8);
		top_index = std::max(top_index, index - 3 + 1);
	}
	else if (index < top_index) {
		MoveFileWindows((top_index - index) * 64, 8);
		top_index = std::min(top_index, index);
	}

	//top_index = std::min(top_index, std::max(top_index, index - 3 + 1));

	if (top_index != old_top_index || index != old_index)
		Refresh();

	for (auto& fw: file_windows) {
		fw->Update();
	}
}


bool Scene_File::IsWindowMoving() const {
	for (auto& fw: file_windows) {
		if (fw->IsMovementActive()) {
			return true;
		}
	}
	return false;
}

void Scene_File::MoveFileWindows(int dy, int dt) {
	for (auto& fw: file_windows) {
		fw->InitMovement(fw->GetX(), fw->GetY(), fw->GetX(), fw->GetY() + dy, dt);
	}
}

