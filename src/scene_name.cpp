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

#include <cassert>

#include "scene_name.h"
#include "game_actors.h"
#include "game_system.h"
#include "input.h"
#include "player.h"
#include "output.h"

Scene_Name::Scene_Name(int actor_id, int charset, bool use_default_name)
	: actor_id(actor_id), layout_index(charset), use_default_name(use_default_name)
{
	Scene::type = Scene::Name;

	auto *actor = Main_Data::game_actors->GetActor(actor_id);
	if (!actor) {
		Output::Error("EnterHeroName: Invalid actor ID {}", actor_id);
	}
}

void Scene_Name::Start() {
	// Create the windows

	auto *actor = Main_Data::game_actors->GetActor(actor_id);
	assert(actor);

	int margin_x = 32;
	int margin_y = 8;
	int window_face_width = 64;
	int window_face_height = 64;
	int window_name_width = 192;
	int window_name_height = 32;
	int window_keyboard_width = 256;
	int window_keyboard_height = 160;

	face_window.reset(new Window_Face(Player::menu_offset_x + margin_x, Player::menu_offset_y + margin_y, window_face_width, window_face_height));
	face_window->Set(actor_id);
	face_window->Refresh();

	name_window.reset(new Window_Name(Player::menu_offset_x + window_face_width + margin_x, Player::menu_offset_y + margin_y + 32, window_name_width, window_name_height));
	name_window->Set(use_default_name ? ToString(actor->GetName()) : "");
	name_window->Refresh();

	const char* done = Window_Keyboard::DONE;
	// Japanese pages
	if (Player::IsCP932()) {
		layouts.push_back(Window_Keyboard::Hiragana);
		layouts.push_back(Window_Keyboard::Katakana);
		done = Window_Keyboard::DONE_JP;
	// Korean pages
	} else if (Player::IsCP949()) {
		layouts.push_back(Window_Keyboard::Hangul1);
		layouts.push_back(Window_Keyboard::Hangul2);
		done = Window_Keyboard::DONE_KO;
	// Simp. Chinese pages
	} else if (Player::IsCP936()) {
		layouts.push_back(Window_Keyboard::ZhCn1);
		layouts.push_back(Window_Keyboard::ZhCn2);
		done = Window_Keyboard::DONE_ZH_CN;
	// Trad. Chinese pages
	} else if (Player::IsBig5()) {
		layouts.push_back(Window_Keyboard::ZhTw1);
		layouts.push_back(Window_Keyboard::ZhTw2);
		done = Window_Keyboard::DONE_ZH_TW;
	// Cyrillic page (we assume it’s Russian since we have no way to detect Serbian etc.)
	} else if (Player::IsCP1251()) {
		layouts.push_back(Window_Keyboard::RuCyrl);
		done = Window_Keyboard::DONE_RU;
	}

	// Letter and symbol pages are used everywhere
	layouts.push_back(Window_Keyboard::Letter);
	layouts.push_back(Window_Keyboard::Symbol);
	kbd_window.reset(new Window_Keyboard(Player::menu_offset_x + margin_x, Player::menu_offset_y + window_face_height + margin_y, window_keyboard_width, window_keyboard_height, done));

	auto next_index = layout_index + 1;
	if (next_index >= static_cast<int>(layouts.size())) {
		next_index = 0;
	}
	kbd_window->SetMode(layouts[layout_index], layouts[next_index]);

	kbd_window->Refresh();
	kbd_window->UpdateCursorRect();
}

void Scene_Name::vUpdate() {
	kbd_window->Update();
	name_window->Update();

	if (Input::IsTriggered(Input::CANCEL)) {
		if (name_window->Get().size() > 0) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
			name_window->Erase();
		} else {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
		}
	} else if (Input::IsTriggered(Input::DECISION) && !kbd_window->mouseOutside) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
		std::string const& s = kbd_window->GetSelected();

		assert(!s.empty());

		if (s == Window_Keyboard::DONE) {
			auto* actor = Main_Data::game_actors->GetActor(actor_id);
			if (actor != nullptr) {
				if (name_window->Get().empty()) {
					name_window->Set(ToString(actor->GetName()));
					name_window->Refresh();
				} else {
					actor->SetName(name_window->Get());
					Scene::Pop();
				}
			}
		} else if (s == Window_Keyboard::NEXT_PAGE) {
			++layout_index;
			if (layout_index >= static_cast<int>(layouts.size())) {
				layout_index = 0;
			}

			auto next_index = layout_index + 1;
			if (next_index >= static_cast<int>(layouts.size())) {
				next_index = 0;
			}
			kbd_window->SetMode(layouts[layout_index], layouts[next_index]);
		} else if (s == Window_Keyboard::SPACE) {
			name_window->Append(" ");
		} else {
			name_window->Append(s);
		}
	}
}
