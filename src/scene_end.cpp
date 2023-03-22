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
#include <vector>
#include "audio.h"
#include "baseui.h"
#include "cache.h"
#include "game_system.h"
#include "input.h"
#include "scene_end.h"
#include "scene_menu.h"
#include "scene_title.h"
#include "util_macro.h"
#include "bitmap.h"
#include <player.h>

Scene_End::Scene_End(SceneType target_scene)
	: target_scene(target_scene) {

	Scene::type = Scene::End;
}

void Scene_End::Start() {
	CreateCommandWindow();
	CreateHelpWindow();
}

void Scene_End::vUpdate() {
	command_window->Update();

	if (Input::IsTriggered(Input::CANCEL)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
		Scene::Pop(); // Select End Game
	} else if (Input::IsTriggered(Input::DECISION)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
		switch (command_window->GetIndex()) {
		case 0: // Yes
			Main_Data::game_system->BgmFade(400);
			if (target_scene == Scene::Title) {
				Scene::ReturnToTitleScene();
			} else {
				Scene::PopUntil(target_scene);
			}
			break;
		case 1: // No
			Scene::Pop();
			break;
		}
	}
}

void Scene_End::CreateCommandWindow() {
	// Create Options Window
	std::vector<std::string> options;

	std::string term_yes = ToString(lcf::Data::terms.yes);
	if (term_yes.empty()) {
		term_yes = "Yes";
	}

	std::string term_no = ToString(lcf::Data::terms.no);
	if (term_no.empty()) {
		term_no = "No";
	}

	options.push_back(term_yes);
	options.push_back(term_no);

	command_window.reset(new Window_Command(options));
	command_window->SetX((Player::screen_width / 2) - (command_window->GetWidth() / 2));
	command_window->SetY(Player::menu_offset_y + 72 + 48);
	command_window->SetIndex(1);
}

void Scene_End::CreateHelpWindow() {
	std::string term_exit_message = ToString(lcf::Data::terms.exit_game_message);
	if (term_exit_message.empty()) {
		term_exit_message = "Do you really want to quit?";
	}

	int text_size = Text::GetSize(*Font::Default(), term_exit_message).width;

	int window_width = text_size + 16;

	help_window.reset(new Window_Help(Player::menu_offset_x + (MENU_WIDTH / 2) - (window_width / 2),
									  Player::menu_offset_y + 72, window_width, 32));
	help_window->SetText(term_exit_message, Font::ColorDefault, Text::AlignLeft, false);

	command_window->SetHelpWindow(help_window.get());
}
