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
#include "output.h"

#ifdef EMSCRIPTEN
#  include <emscripten.h>
#  include "platform/emscripten/interface.h"
#endif

constexpr int arrow_animation_frames = 20;

Scene_File::Scene_File(std::string message) :
	message(message) {
}

std::unique_ptr<Sprite> Scene_File::MakeBorderSprite(int y) {
	int border_height = 8;
	auto bitmap = Bitmap::Create(MENU_WIDTH, border_height, Cache::System()->GetBackgroundColor());
	auto sprite = std::unique_ptr<Sprite>(new Sprite());
	sprite->SetVisible(true);
	sprite->SetZ(Priority_Window + 1);
	sprite->SetBitmap(bitmap);
	sprite->SetX(Player::menu_offset_x);
	sprite->SetY(y);
	return sprite;
}

std::unique_ptr<Sprite> Scene_File::MakeArrowSprite(bool down) {
	int sprite_width = 8;
	int sprite_height = 8;

	Rect rect = Rect(40, (down ? 16 : sprite_height), 16, sprite_height);
	auto bitmap = Bitmap::Create(*(Cache::System()), rect);
	auto sprite = std::unique_ptr<Sprite>(new Sprite());
	sprite->SetVisible(false);
	sprite->SetZ(Priority_Window + 2);
	sprite->SetBitmap(bitmap);
	sprite->SetX((MENU_WIDTH / 2) - sprite_width + Player::menu_offset_x);
	sprite->SetY(down ? Player::screen_height - sprite_height : 32);
	return sprite;
}

void Scene_File::CreateHelpWindow() {
	help_window = std::make_unique<Window_Help>(this, Player::menu_offset_x, 0, MENU_WIDTH, 32);
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

	std::string file = fs.FindFile(ss.str());

	if (!file.empty()) {
		// File found
		auto save_stream = FileFinder::Save().OpenInputStream(file);
		if (!save_stream) {
			Output::Debug("Save {} read error", file);
			win.SetCorrupted(true);
			return;
		}

		std::unique_ptr<lcf::rpg::Save> savegame = lcf::LSD_Reader::Load(save_stream, Player::encoding);

		if (savegame) {
			PopulatePartyFaces(win, id, *savegame);
			UpdateLatestTimestamp(id, *savegame);
		} else {
			Output::Debug("Save {} corrupted", file);
			win.SetCorrupted(true);
		}
	}
}

void Scene_File::Start() {
	CreateHelpWindow();
	border_top = Scene_File::MakeBorderSprite(32);

	// Refresh File Finder Save Folder
	fs = FileFinder::Save();

	for (int i = 0; i < Utils::Clamp<int32_t>(lcf::Data::system.easyrpg_max_savefiles, 3, 99); i++) {
		std::shared_ptr<Window_SaveFile>
			w = std::make_unique<Window_SaveFile>(this, Player::menu_offset_x, 40 + i * 64, MENU_WIDTH, 64);
		w->SetIndex(i);
		w->SetZ(Priority_Window);
		PopulateSaveWindow(*w, i);
		w->Refresh();

		file_windows.push_back(w);
	}

	border_bottom = Scene_File::MakeBorderSprite(Player::screen_height - 8);

	up_arrow = Scene_File::MakeArrowSprite(false);
	down_arrow = Scene_File::MakeArrowSprite(true);

	index = latest_slot;
	oldIndex = index;
	top_index = std::max(0, index - 2);

	RefreshWindows();

	for (auto& fw: file_windows) {
		fw->Update();
	}

	std::vector<std::string> commands;
#ifdef EMSCRIPTEN
	commands.emplace_back("Download Savegame");
	commands.emplace_back("Upload Savegame");
#endif
	extra_commands_window = std::make_unique<Window_Command>(this, commands);
	extra_commands_window->SetZ(Priority_Window + 100);
	extra_commands_window->SetVisible(false);
}

void Scene_File::RefreshWindows() {
	for (int i = 0; i < (int)file_windows.size(); i++) {
		Window_SaveFile *w = file_windows[i].get();
		w->SetY(40 + (i - top_index) * 64);
		if (disabledByMouse)
			w->SetActive(false);
		else
			w->SetActive(i == index);
		w->Refresh();
	}
}

void Scene_File::Refresh() {
	for (int i = 0; i < Utils::Clamp<int32_t>(lcf::Data::system.easyrpg_max_savefiles, 3, 99); i++) {
		Window_SaveFile *w = file_windows[i].get();
		PopulateSaveWindow(*w, i);
		w->Refresh();
	}
}

void Scene_File::vUpdate() {
	UpdateArrows();

	if (IsWindowMoving()) {
		for (auto& fw: file_windows) {
			fw->Update();
		}
		return;
	}

	if (HandleExtraCommandsWindow()) {
		return;
	}
	
	int old_top_index = top_index;
	int old_index = index;
	int max_index = static_cast<int>(file_windows.size()) - 1;

	if (Input::GetUseMouseButton()) {
		int i = 0;
		bool mouseOutside = true;
		Point mouseP = Input::GetMousePosition();

		if (mouseP.y >= 40 - file_windows[0]->GetBorderY() && mouseP.y < 40 && up_arrow) {

			// Change cursor (Hand)
			DisplayUi->ChangeCursor(1);

			if (Input::IsRepeated(Input::MOUSE_LEFT)) {
				top_index = std::max(0, top_index - 1);
				index = top_index;
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
			}

		}
		else if (mouseP.y >= Player::screen_height - file_windows[0]->GetBorderY() && mouseP.y < Player::screen_height && down_arrow) {

			// Change cursor (Hand)
			DisplayUi->ChangeCursor(1);

			if (Input::IsRepeated(Input::MOUSE_LEFT)) {
				top_index = std::min(max_index - 2, top_index + 1);
				index = top_index;
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
			}

		}
		else if (mouseP.y >= 40) {
			for (auto& fw : file_windows) {
				if (fw->IsVisible()) {
					

					if (mouseP.x >= fw->GetX() + fw->GetBorderX() && mouseP.x <= fw->GetX() + fw->GetWidth() - fw->GetBorderX() &&
						mouseP.y >= fw->GetY() + fw->GetBorderY() && mouseP.y < fw->GetY() + fw->GetHeight() - fw->GetBorderY()) {

						// Change cursor (Hand)
						DisplayUi->ChangeCursor(1);

						//if (Input::IsPressed(Input::MOUSE_LEFT)) {
						if (Input::MouseMoved()) {
							mouseOutside = false;
							if (oldIndex != i)
								Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
							oldIndex = index;
							index = i;
							disabledByMouse = false;
							Refresh();
							break;
						}
					}
				}
				i++;
			}
		}

		//if (Input::IsPressed(Input::MOUSE_LEFT) && mouseOutside) {
		if (Input::MouseMoved() && mouseOutside) {
			disabledByMouse = true;
			Refresh();
		}
	}

	if (Input::IsTriggered(Input::CANCEL)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
		Scene::Pop();
	} else if (Input::IsTriggered(Input::DECISION) && !disabledByMouse) {
		if (IsSlotValid(index)) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
			Action(index);
		}
		else {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
		}
	} else if (Input::IsTriggered(Input::SHIFT)) {
#ifdef EMSCRIPTEN
		extra_commands_window->SetX(SCREEN_TARGET_WIDTH - extra_commands_window->GetWidth() - 8);
		extra_commands_window->SetY(file_windows[index]->GetY() + 8);
		extra_commands_window->SetItemEnabled(0, file_windows[index]->IsValid() && file_windows[index]->HasParty());
		extra_commands_window->SetVisible(true);
		return;
#endif
	}

	if (disabledByMouse) {
		if (Input::IsRepeated(Input::DOWN) || (Input::IsRepeated(Input::SCROLL_DOWN) && !Input::GetUseMouseButton())) {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
				disabledByMouse = false;
				index = oldIndex;
				Refresh();
		}
		if (Input::IsRepeated(Input::UP) || (Input::IsRepeated(Input::SCROLL_UP) && !Input::GetUseMouseButton())) {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
				disabledByMouse = false;
				index = oldIndex;
				Refresh();
		}
	}
	else {
		if (Input::IsRepeated(Input::DOWN) || (Input::IsTriggered(Input::SCROLL_DOWN) && !Input::GetUseMouseButton())) {
			if (Input::IsTriggered(Input::DOWN) || (Input::IsTriggered(Input::SCROLL_DOWN) && !Input::GetUseMouseButton())
				|| index < max_index) {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
				index = (index + 1) % file_windows.size();
			}

			//top_index = std::max(top_index, index - 3 + 1);
		}
		if (Input::IsRepeated(Input::UP) || (Input::IsTriggered(Input::SCROLL_UP) && !Input::GetUseMouseButton())) {
			if (Input::IsTriggered(Input::UP) || (Input::IsTriggered(Input::SCROLL_UP) && !Input::GetUseMouseButton())
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
	}

	if (Input::GetUseMouseButton()) {

		bool show_up_arrow = (top_index > 0);
		bool show_down_arrow = (top_index < max_index - 2);

		if (Input::IsTriggered(Input::SCROLL_DOWN) && show_down_arrow) {

			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
			index = (index + 1) % file_windows.size();
			top_index += 1;

			//top_index = std::max(top_index, index - 3 + 1);
		}
		if (Input::IsTriggered(Input::SCROLL_UP) && show_up_arrow) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
			index = (index + max_index) % file_windows.size();
			top_index -= 1;

			//top_index = std::min(top_index, index);
		}
	}

	if (index > top_index + 2) {
		MoveFileWindows((top_index + 2 - index) * 64, 7);
		top_index = std::max(top_index, index - 3 + 1);
	}
	else if (index < top_index) {
		MoveFileWindows((top_index - index) * 64, 7);
		top_index = std::min(top_index, index);
	}

	//top_index = std::min(top_index, std::max(top_index, index - 3 + 1));

	if (top_index != old_top_index || index != old_index)
		RefreshWindows();

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

void Scene_File::UpdateArrows() {
	int max_index = static_cast<int>(file_windows.size()) - 1;

	bool show_up_arrow = (top_index > 0);
	bool show_down_arrow = (top_index < max_index - 2);

	if (show_up_arrow || show_down_arrow) {
		arrow_frame = (arrow_frame + 1) % (arrow_animation_frames * 2);
	}
	bool arrow_visible = (arrow_frame < arrow_animation_frames);
	up_arrow->SetVisible(show_up_arrow && arrow_visible);
	down_arrow->SetVisible(show_down_arrow && arrow_visible);
}

bool Scene_File::HandleExtraCommandsWindow() {
	if (!extra_commands_window->IsVisible()) {
		return false;
	}

	extra_commands_window->Update();

#ifdef EMSCRIPTEN
	if (Input::IsTriggered(Input::DECISION)) {
		if (extra_commands_window->GetIndex() == 0) {
			// Download
			if (!extra_commands_window->IsItemEnabled(0)) {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
				return true;
			}

			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
			Emscripten_Interface::DownloadSavegame(index + 1);
			extra_commands_window->SetVisible(false);
		} else {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
			Emscripten_Interface::UploadSavegame(index + 1);
			extra_commands_window->SetVisible(false);
		}
	} else if (Input::IsTriggered(Input::CANCEL)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
		extra_commands_window->SetVisible(false);
	}
#endif

	return true;
}
