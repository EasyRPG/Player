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
#include <sstream>
#include "data.h"
#include "filefinder.h"
#include "game_actor.h"
#include "game_party.h"
#include "lsd_reader.h"
#include "scene_save.h"
#include "scene_file.h"

Scene_Save::Scene_Save() :
	Scene_File(Data::terms.save_game_message) {
	Scene::type = Scene::Save;
}

void Scene_Save::Action(int index) {
	std::stringstream ss;
	ss << "Save" << (index <= 8 ? "0" : "") << (index + 1) << ".lsd";

	// TODO: Maybe find a better place to setup the save file?
	RPG::SaveTitle title;

	int size = (int)Game_Party::GetActors().size();
	Game_Actor* actor;

	switch (size) {
		case 4:
			actor = Game_Party::GetActors()[3];
			title.face4_id = actor->GetFaceIndex();
			title.face4_name = actor->GetFaceName();
		case 3:
			actor = Game_Party::GetActors()[2];
			title.face3_id = actor->GetFaceIndex();
			title.face3_name = actor->GetFaceName();
		case 2:
			actor = Game_Party::GetActors()[1];
			title.face2_id = actor->GetFaceIndex();
			title.face2_name = actor->GetFaceName();
		case 1:
			actor = Game_Party::GetActors()[0];
			title.face1_id = actor->GetFaceIndex();
			title.face1_name = actor->GetFaceName();
			title.hero_hp = actor->GetHp();
			title.hero_level = actor->GetLevel();
			title.hero_name = actor->GetName();
			break;
		default:;
	}

	Main_Data::game_data.title = title;

	Main_Data::game_data.system.save_slot = index + 1;
	Main_Data::game_data.system.save_count += 1;

	LSD_Reader::Save(FileFinder::FindDefault(ss.str()), Main_Data::game_data);
}
