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
#include "data.h"
#include "game_system.h"
#include "game_party.h"
#include "input.h"
#include "lsd_reader.h"
#include "player.h"
#include "rpg_save.h"
#include "scene_file.h"
#include "bitmap.h"
#include "reader_util.h"

Scene_File::Scene_File(std::string message) :
	message(message), latest_time(0), latest_slot(0) {
	top_index = 0;
	index = 0;
}

static std::unique_ptr<Sprite> makeBorderSprite(int y) {
	auto bitmap = Bitmap::Create(SCREEN_TARGET_WIDTH, 8, Cache::System()->GetBackgroundColor());
	auto sprite = std::unique_ptr<Sprite>(new Sprite());
	sprite->SetVisible(true);
	sprite->SetZ(Priority_Window + 1);
	sprite->SetBitmap(bitmap);
	sprite->SetX(0);
	sprite->SetY(y);
	return sprite;
}

void Scene_File::Start() {
	// Create the windows
	help_window.reset(new Window_Help(0, 0, SCREEN_TARGET_WIDTH, 32));
	help_window->SetText(message);
	help_window->SetZ(Priority_Window + 1);

	border_top = makeBorderSprite(32);

	// Refresh File Finder Save Folder
	tree = FileFinder::CreateSaveDirectoryTree();

	for (int i = 0; i < 15; i++) {
		std::shared_ptr<Window_SaveFile>
			w(new Window_SaveFile(0, 40 + i * 64, SCREEN_TARGET_WIDTH, 64));
		w->SetIndex(i);
		w->SetZ(Priority_Window);

		// Try to access file
		std::stringstream ss;
		ss << "Save" << (i <= 8 ? "0" : "") << (i+1) << ".lsd";

		std::string file = FileFinder::FindDefault(*tree, ss.str());

		if (!file.empty()) {
			// File found
			std::unique_ptr<RPG::Save> savegame =
				LSD_Reader::Load(file, Player::encoding);

			if (savegame.get())	{
				std::vector<std::pair<int, std::string> > party;

				// When a face_name is empty the party list ends
				int party_size =
					savegame->title.face1_name.empty() ? 0 :
					savegame->title.face2_name.empty() ? 1 :
					savegame->title.face3_name.empty() ? 2 :
					savegame->title.face4_name.empty() ? 3 : 4;

				party.resize(party_size);

				if (party_size > 3) {
					party[3].first = savegame->title.face4_id;
					party[3].second = savegame->title.face4_name;
				}
				if (party_size > 2) {
					party[2].first = savegame->title.face3_id;
					party[2].second = savegame->title.face3_name;
				}
				if (party_size > 1) {
					party[1].first = savegame->title.face2_id;
					party[1].second = savegame->title.face2_name;
				}
				if (party_size > 0) {
					party[0].first = savegame->title.face1_id;
					party[0].second = savegame->title.face1_name;
				}

				w->SetParty(party, savegame->title.hero_name, savegame->title.hero_hp,
					savegame->title.hero_level);
				w->SetHasSave(true);

				if (savegame->title.timestamp > latest_time) {
					latest_time = savegame->title.timestamp;
					latest_slot = i;
				}
			} else {
				w->SetCorrupted(true);
			}
		}

		w->Refresh();

		file_windows.push_back(w);
	}

	border_bottom = makeBorderSprite(232);

	index = latest_slot;
	top_index = std::max(0, index - 2);

	Refresh();
	Update();
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
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cancel));
		Scene::Pop();
	} else if (Input::IsTriggered(Input::DECISION)) {
		if (IsSlotValid(index)) {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
			Action(index);
		}
		else {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
		}
	}

	unsigned int old_top_index = top_index;
	unsigned int old_index = index;
	unsigned int max_index = file_windows.size() - 1;

	if (Input::IsRepeated(Input::DOWN) || Input::IsTriggered(Input::SCROLL_DOWN)) {
		if (Input::IsTriggered(Input::DOWN) || Input::IsTriggered(Input::SCROLL_DOWN)
			|| index < max_index) {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cursor));
			index = (index + 1) % file_windows.size();
		}

		//top_index = std::max(top_index, index - 3 + 1);
	}
	if (Input::IsRepeated(Input::UP) || Input::IsTriggered(Input::SCROLL_UP)) {
		if (Input::IsTriggered(Input::UP) || Input::IsTriggered(Input::SCROLL_UP)
			|| index >= 1) {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cursor));
			index = (index + max_index) % file_windows.size();
		}

		//top_index = std::min(top_index, index);
	}

	if (Input::IsRepeated(Input::PAGE_DOWN) && index < max_index) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cursor));
		index = (index + 3 <= max_index) ? index + 3 : max_index;
	}
	if (Input::IsRepeated(Input::PAGE_UP) && index >= 1) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cursor));
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

