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
#include "data.h"
#include "filefinder.h"
#include "game_system.h"
#include "game_party.h"
#include "input.h"
#include "lsd_reader.h"
#include "rpg_save.h"
#include "scene_file.h"

Scene_File::Scene_File(std::string message) :
	help_window(NULL), message(message) {
	top_index = 0;
	index = 0;
}

void Scene_File::Start() {
	// Create the windows
	help_window.reset(new Window_Help(0, 0, 320, 32));
	help_window->SetText(message);

	for (int i = 0; i < 15; i++) {
		EASYRPG_SHARED_PTR<Window_SaveFile>
			w(new Window_SaveFile(0, 40 + i * 64, 320, 64));
		w->SetIndex(i);

		// Try to access file
		std::stringstream ss;
		ss << "Save" << (i <= 8 ? "0" : "") << (i+1) << ".lsd";
		std::string file = FileFinder::FindDefault(ss.str());
		if (!file.empty()) {
			// File found
			std::auto_ptr<RPG::Save> savegame =
				LSD_Reader::Load(file);

			if (savegame.get())	{
				std::vector<std::pair<int, std::string> > party;

				// When a face_name is empty the party list ends
				int party_size =
					savegame->title.face1_name.empty() ? 0 :
					savegame->title.face2_name.empty() ? 1 :
					savegame->title.face3_name.empty() ? 2 :
					savegame->title.face4_name.empty() ? 3 : 4;

				party.resize(party_size);

				switch (party_size) {
					case 4:
						party[3].first = savegame->title.face4_id;
						party[3].second = savegame->title.face4_name;
					case 3:
						party[2].first = savegame->title.face3_id;
						party[2].second = savegame->title.face3_name;
					case 2:
						party[1].first = savegame->title.face2_id;
						party[1].second = savegame->title.face2_name;
					case 1:
						party[0].first = savegame->title.face1_id;
						party[0].second = savegame->title.face1_name;
						break;
					default:;
				}

				w->SetParty(party, savegame->title.hero_name, savegame->title.hero_hp,
					savegame->title.hero_level);
			} else {
				w->SetCorrupted(true);
			}
		}

		w->Refresh();

		file_windows.push_back(w);
	}

	Refresh();
}

void Scene_File::Refresh() {
	for (int i = 0; (size_t) i < file_windows.size(); i++) {
		Window_SaveFile *w = file_windows[i].get();
		w->SetY(40 + (i - top_index) * 64);
		w->SetActive(i == index);
		w->SetVisible(i >= top_index && i < top_index + 3);
	}
}

void Scene_File::Update() {
	for (int i = 0; (size_t) i < file_windows.size(); i++)
		file_windows[i]->Update();

	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Data::system.cancel_se);
		Scene::Pop();
	} else if (Input::IsTriggered(Input::DECISION)) {
		Game_System::SePlay(Data::system.decision_se);
		Action(index);
		Scene::Pop();
	}

	int old_top_index = top_index;
	int old_index = index;

	if (Input::IsRepeated(Input::DOWN)) {
		Game_System::SePlay(Data::system.cursor_se);
		index++;
		if ((size_t) index >= file_windows.size())
			index--;
		top_index = std::max(top_index, index - 3 + 1);
	}
	if (Input::IsRepeated(Input::UP)) {
		Game_System::SePlay(Data::system.cursor_se);
		index--;
		if (index < 0)
			index++;
		top_index = std::min(top_index, index);
	}

	if (top_index != old_top_index || index != old_index)
		Refresh();
}
