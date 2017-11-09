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
#include "game_temp.h"
#include "input.h"
#include "player.h"

Scene_Name::Scene_Name() {
	Scene::type = Scene::Name;
}

void Scene_Name::Start() {
	// Create the windows

	name_window.reset(new Window_Name(96, 40, 192, 32));
	name_window->Set(Game_Temp::hero_name);
	name_window->Refresh();

	face_window.reset(new Window_Face(32, 8, 64, 64));
	face_window->Set(Game_Temp::hero_name_id);
	face_window->Refresh();

	layout_index = Game_Temp::hero_name_charset;

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
	kbd_window.reset(new Window_Keyboard(32, 72, 256, SCREEN_TARGET_WIDTH / 2, done));

	size_t next_index = layout_index + 1;
	if (next_index >= layouts.size()) {
		next_index = 0;
	}
	kbd_window->SetMode(layouts[layout_index], layouts[next_index]);

	kbd_window->Refresh();
	kbd_window->UpdateCursorRect();
}

void Scene_Name::Update() {
	kbd_window->Update();
	name_window->Update();

	if (Input::IsTriggered(Input::CANCEL)) {
		if (name_window->Get().size() > 0) {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cancel));
			name_window->Erase();
		} else {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
		}
	} else if (Input::IsTriggered(Input::DECISION)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
		std::string const& s = kbd_window->GetSelected();

		assert(!s.empty());

		if (s == Window_Keyboard::DONE) {
			Game_Temp::hero_name = name_window->Get();
			Game_Actor* actor = Game_Actors::GetActor(Game_Temp::hero_name_id);
			if (actor != NULL) {
				if (name_window->Get().empty()) {
					name_window->Set(actor->GetName());
					name_window->Refresh();
				} else {
					actor->SetName(name_window->Get());
					Scene::Pop();
				}
			}
		} else if (s == Window_Keyboard::NEXT_PAGE) {
			++layout_index;
			if (layout_index >= layouts.size()) {
				layout_index = 0;
			}

			size_t next_index = layout_index + 1;
			if (next_index >= layouts.size()) {
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
