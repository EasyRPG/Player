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
#include "scene_equip.h"
#include "game_party.h"
#include "game_system.h"
#include "graphics.h"
#include "input.h"
#include "player.h"
#include "scene_menu.h"

////////////////////////////////////////////////////////////
Scene_Equip::Scene_Equip(int actor_index, int equip_index) :
	actor_index(actor_index),
	equip_index(equip_index) {
	type = Scene::Equip;
}

////////////////////////////////////////////////////////////
Scene_Equip::~Scene_Equip() {
	delete help_window;
	delete left_window;
	delete right_window;
	for (int i = 0; i < 5; ++i) {
		delete item_window[i];
	}
}

////////////////////////////////////////////////////////////
void Scene_Equip::MainFunction() {
	Game_Actor* actor = Game_Party::GetActors()[actor_index];

	help_window = new Window_Help();
	left_window = new Window_EquipLeft(actor->GetActorId());
	right_window = new Window_EquipRight(actor->GetActorId());

	item_window.push_back(new Window_EquipItem(actor->GetActorId(),
		Window_EquipItem::weapon));
	// ToDo: Must be weapon again if two-handed
	item_window.push_back(new Window_EquipItem(actor->GetActorId(),
		Window_EquipItem::shield));
	item_window.push_back(new Window_EquipItem(actor->GetActorId(),
		Window_EquipItem::armor));
	item_window.push_back(new Window_EquipItem(actor->GetActorId(),
		Window_EquipItem::helmet));
	item_window.push_back(new Window_EquipItem(actor->GetActorId(),
		Window_EquipItem::other));

	Refresh();

	Graphics::Transition(Graphics::FadeIn, 20, true);

	// Scene loop
	while (instance == this) {
		Player::Update();
		Graphics::Update();
		Input::Update();
		Update();
	}

	Graphics::Transition(Graphics::FadeOut, 20, false);

	Scene::old_instance = this;
}

////////////////////////////////////////////////////////////
void Scene_Equip::Refresh() {
	active_item_window = item_window[0];
}

////////////////////////////////////////////////////////////
void Scene_Equip::Update() {
	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Data::system.cancel_se);
		Scene::instance = new Scene_Menu(2); // Select Equipment
	}

	left_window->Update();
	right_window->Update();
	active_item_window->Update();

	Refresh();
}
