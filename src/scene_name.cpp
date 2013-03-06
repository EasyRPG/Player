/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "scene_name.h"
#include "game_actors.h"
#include "game_system.h"
#include "game_temp.h"
#include "input.h"

#include <ciso646>
#include <cassert>

////////////////////////////////////////////////////////////
Scene_Name::Scene_Name() :
	kbd_window(NULL), name_window(NULL), face_window(NULL) {
	Scene::type = Scene::Name;
}

////////////////////////////////////////////////////////////
void Scene_Name::Start() {
	// Create the windows

	name_window.reset(new Window_Name(96, 40, 192, 32));
	name_window->Set(Game_Temp::hero_name);
	name_window->Refresh();

	face_window.reset(new Window_Face(32, 8, 64, 64));
	face_window->Set(Game_Temp::hero_name_id);
	face_window->Refresh();

	kbd_window.reset(new Window_Keyboard(32, 72, 256, 160));
	kbd_window->SetMode(Window_Keyboard::Mode(Game_Temp::hero_name_charset));
	kbd_window->Refresh();
	kbd_window->UpdateCursorRect();
}

////////////////////////////////////////////////////////////
void Scene_Name::Update() {
	kbd_window->Update();

	if (Input::IsTriggered(Input::CANCEL)) {
		if (name_window->Get().size() > 0) {
			Game_System::SePlay(Data::system.cancel_se);
			name_window->Erase();
		}
		else
			Game_System::SePlay(Data::system.buzzer_se);
	} else if (Input::IsTriggered(Input::DECISION)) {
		Game_System::SePlay(Data::system.decision_se);
		std::string const& s = kbd_window->GetSelected();

		assert(not s.empty());

		if(s == Window_Keyboard::DONE || s == Window_Keyboard::DONE_JP) {
			Game_Temp::hero_name = name_window->Get();
			Game_Actor* actor = Game_Actors::GetActor(Game_Temp::hero_name_id);
			if (actor != NULL) {
				actor->SetName(name_window->Get());
			}
			Scene::Pop();
		} else if(s == Window_Keyboard::TO_SYMBOL) {
			kbd_window->SetMode(Window_Keyboard::Symbol);
		} else if(s == Window_Keyboard::TO_LETTER) {
			kbd_window->SetMode(Window_Keyboard::Letter);
		} else if(s == Window_Keyboard::TO_HIRAGANA) {
			kbd_window->SetMode(Window_Keyboard::Hiragana);
		} else if(s == Window_Keyboard::TO_KATAKANA) {
			kbd_window->SetMode(Window_Keyboard::Katakana);
		} else { name_window->Append(s); }
	}
}
