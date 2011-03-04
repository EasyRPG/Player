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
#include <string>
#include <vector>
#include "scene_status.h"
#include "game_party.h"
#include "game_system.h"
#include "input.h"

////////////////////////////////////////////////////////////
Scene_Status::Scene_Status(int actor_index) :
	actor_index(actor_index) {
	type = Scene::Status;
}

////////////////////////////////////////////////////////////
void Scene_Status::Start() {
	int actor = Game_Party::GetActors()[actor_index]->GetId();

	actorinfo_window = new Window_ActorInfo(0, 0, 124, 208, actor);
	actorstatus_window = new Window_ActorStatus(124, 0, 196, 64, actor);
	gold_window = new Window_Gold(0, 208, 124, 32);
	equipstatus_window = new Window_EquipStatus(124, 64, 196, 80, actor, false);
	equip_window = new Window_Equip(124, 144, 196, 96, actor);

	equip_window->SetActive(false);
	equipstatus_window->SetActive(false);

	equip_window->SetIndex(-1);
}

////////////////////////////////////////////////////////////
void Scene_Status::Terminate() {
	delete actorinfo_window;
	delete actorstatus_window;
	delete gold_window;
	delete equipstatus_window;
	delete equip_window;
}

////////////////////////////////////////////////////////////
void Scene_Status::Update() {
	gold_window->Update();
	equipstatus_window->Update();
	equip_window->Update();

	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Data::system.cancel_se);
		Scene::Pop();
	} else if (Game_Party::GetActors().size() > 1 && Input::IsTriggered(Input::RIGHT)) {
		Game_System::SePlay(Data::system.cursor_se);
		actor_index = (actor_index + 1) % Game_Party::GetActors().size();
		Scene::Push(new Scene_Status(actor_index), true);
	} else if (Game_Party::GetActors().size() > 1 && Input::IsTriggered(Input::LEFT)) {
		Game_System::SePlay(Data::system.cursor_se);
		actor_index = (actor_index + Game_Party::GetActors().size() - 1) % Game_Party::GetActors().size();
		Scene::Push(new Scene_Status(actor_index), true);
	}
}
