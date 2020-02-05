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

#ifdef EMSCRIPTEN
#  include <emscripten.h>
#endif

#include "data.h"
#include "filefinder.h"
#include "game_actor.h"
#include "game_map.h"
#include "game_party.h"
#include "game_switches.h"
#include "game_variables.h"
#include "game_party.h"
#include "game_system.h"
#include "game_targets.h"
#include "game_screen.h"
#include "game_pictures.h"
#include "lsd_reader.h"
#include "output.h"
#include "player.h"
#include "scene_save.h"
#include "scene_file.h"
#include "reader_util.h"
#include "version.h"

Scene_Save::Scene_Save() :
	Scene_File(Data::terms.save_game_message) {
	Scene::type = Scene::Save;
}

void Scene_Save::Start() {
	Scene_File::Start();

	for (int i = 0; i < 15; i++) {
		file_windows[i]->SetHasSave(true);
		file_windows[i]->Refresh();
	}
}

void Scene_Save::Action(int index) {
	std::stringstream ss;
	ss << "Save" << (index <= 8 ? "0" : "") << (index + 1) << ".lsd";

	Output::Debug("Saving to %s", ss.str().c_str());

	// TODO: Maybe find a better place to setup the save file?
	RPG::SaveTitle title;

	int size = (int)Main_Data::game_party->GetActors().size();
	Game_Actor* actor;

	if (size > 3) {
		actor = Main_Data::game_party->GetActors()[3];
		title.face4_id = actor->GetFaceIndex();
		title.face4_name = actor->GetFaceName();
	}
	if (size > 2) {
		actor = Main_Data::game_party->GetActors()[2];
		title.face3_id = actor->GetFaceIndex();
		title.face3_name = actor->GetFaceName();
	}
	if (size > 1) {
		actor = Main_Data::game_party->GetActors()[1];
		title.face2_id = actor->GetFaceIndex();
		title.face2_name = actor->GetFaceName();
	}
	if (size > 0) {
		actor = Main_Data::game_party->GetActors()[0];
		title.face1_id = actor->GetFaceIndex();
		title.face1_name = actor->GetFaceName();
		title.hero_hp = actor->GetHp();
		title.hero_level = actor->GetLevel();
		title.hero_name = actor->GetName();
	}

	Main_Data::game_data.title = title;

	Game_System::SetSaveSlot(index + 1);

	Game_Map::PrepareSave();

	std::string save_file = ss.str();
	std::string filename = FileFinder::FindDefault(*tree, ss.str());

	if (filename.empty()) {
		filename = FileFinder::MakePath((*tree).directory_path, save_file);
	}

	LSD_Reader::PrepareSave(Main_Data::game_data, PLAYER_SAVEGAME_VERSION);
	auto data_copy = LSD_Reader::ClearDefaults(Main_Data::game_data, Game_Map::GetMapInfo(), Game_Map::GetMap());
	// RPG_RT doesn't save these chunks in rm2k as they are meaningless
	if (Player::IsRPG2k()) {
		for (auto& actor: data_copy.actors) {
			actor.two_weapon = 0;
			actor.lock_equipment = 0;
			actor.auto_battle = 0;
			actor.super_guard = 0;
		}
	}

	data_copy.targets = Main_Data::game_targets->GetSaveData();
	data_copy.system.switches = Main_Data::game_switches->GetData();
	data_copy.system.variables = Main_Data::game_variables->GetData();
	data_copy.inventory = Main_Data::game_party->GetSaveData();

	data_copy.screen = Main_Data::game_screen->GetSaveData();
	data_copy.pictures = Main_Data::game_pictures->GetSaveData();

	// RPG_RT saves always have the scene set to this.
	data_copy.system.scene = RPG::SaveSystem::Scene_file;
	// 2k RPG_RT always stores SaveMapEvent with map_id == 0.
	if (Player::IsRPG2k()) {
		for (auto& sme: data_copy.map_info.events) {
			sme.map_id = 0;
		}
	}
	LSD_Reader::Save(filename, data_copy, Player::encoding);

#ifdef EMSCRIPTEN
	// Save changed file system
	EM_ASM({
		FS.syncfs(function(err) {
		});
	});
#endif

	Scene::Pop();
}

bool Scene_Save::IsSlotValid(int) {
	return true;
}
