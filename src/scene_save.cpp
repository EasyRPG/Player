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

#include <lcf/data.h>
#include "dynrpg.h"
#include "filefinder.h"
#include "game_actor.h"
#include "game_map.h"
#include "game_party.h"
#include "game_switches.h"
#include "game_variables.h"
#include "game_strings.h"
#include "game_party.h"
#include "game_actors.h"
#include "game_system.h"
#include "game_targets.h"
#include "game_screen.h"
#include "game_pictures.h"
#include "game_windows.h"
#include <lcf/lsd/reader.h>
#include "output.h"
#include "player.h"
#include "scene_save.h"
#include "translation.h"
#include "version.h"

Scene_Save::Scene_Save() :
	Scene_File(ToString(lcf::Data::terms.save_game_message)) {
	Scene::type = Scene::Save;
}

void Scene_Save::Start() {
	Scene_File::Start();

	for (int i = 0; i < Utils::Clamp<int32_t>(lcf::Data::system.easyrpg_max_savefiles, 3, 99); i++) {
		file_windows[i]->SetHasSave(true);
		file_windows[i]->Refresh();
	}
}

void Scene_Save::Action(int index) {
	Save(fs, index + 1);

	Scene::Pop();
}

std::string Scene_Save::GetSaveFilename(const FilesystemView& fs, int slot_id) {
	const auto save_file = fmt::format("Save{:02d}.lsd", slot_id);

	std::string filename = fs.FindFile(save_file);

	if (filename.empty()) {
		filename = save_file;
	}
	return filename;
}

bool Scene_Save::Save(const FilesystemView& fs, int slot_id, bool prepare_save) {
	const auto filename = GetSaveFilename(fs, slot_id);
	Output::Debug("Saving to {}", filename);

	auto save_stream = FileFinder::Save().OpenOutputStream(filename);

	if (!save_stream) {
		Output::Warning("Failed saving to {}", filename);
		return false;
	}

	return Save(save_stream, slot_id, prepare_save);
}

bool Scene_Save::Save(std::ostream& os, int slot_id, bool prepare_save) {
	lcf::rpg::Save save;
	auto& title = save.title;
	// TODO: Maybe find a better place to setup the save file?

	int size = (int)Main_Data::game_party->GetActors().size();
	Game_Actor* actor;

	if (size > 3) {
		actor = Main_Data::game_party->GetActors()[3];
		title.face4_id = actor->GetFaceIndex();
		title.face4_name = ToString(actor->GetFaceName());
	}
	if (size > 2) {
		actor = Main_Data::game_party->GetActors()[2];
		title.face3_id = actor->GetFaceIndex();
		title.face3_name = ToString(actor->GetFaceName());
	}
	if (size > 1) {
		actor = Main_Data::game_party->GetActors()[1];
		title.face2_id = actor->GetFaceIndex();
		title.face2_name = ToString(actor->GetFaceName());
	}
	if (size > 0) {
		actor = Main_Data::game_party->GetActors()[0];
		title.face1_id = actor->GetFaceIndex();
		title.face1_name = ToString(actor->GetFaceName());
		title.hero_hp = actor->GetHp();
		title.hero_level = actor->GetLevel();
		title.hero_name = ToString(actor->GetName());
	}

	Main_Data::game_system->SetSaveSlot(slot_id);
	save.party_location = Main_Data::game_player->GetSaveData();
	Game_Map::PrepareSave(save);

	if (prepare_save) {
		// When a translation is loaded always store in Unicode to prevent data loss
		int codepage = Tr::HasActiveTranslation() ? 65001 : 0;

		lcf::LSD_Reader::PrepareSave(save, PLAYER_SAVEGAME_VERSION, codepage);
		Main_Data::game_system->IncSaveCount();
	}

	save.targets = Main_Data::game_targets->GetSaveData();
	save.system = Main_Data::game_system->GetSaveData();
	save.system.switches = Main_Data::game_switches->GetData();
	save.system.variables = Main_Data::game_variables->GetData();
	save.system.maniac_strings = Main_Data::game_strings->GetData();
	save.inventory = Main_Data::game_party->GetSaveData();
	save.actors = Main_Data::game_actors->GetSaveData();
	save.screen = Main_Data::game_screen->GetSaveData();
	save.pictures = Main_Data::game_pictures->GetSaveData();
	save.easyrpg_data.windows = Main_Data::game_windows->GetSaveData();

	save.system.scene = Scene::instance ? Scene::rpgRtSceneFromSceneType(Scene::instance->type) : -1;

	// 2k RPG_RT always stores SaveMapEvent with map_id == 0.
	if (Player::IsRPG2k()) {
		for (auto& sme: save.map_info.events) {
			sme.map_id = 0;
		}
	}
	auto lcf_engine = Player::IsRPG2k3() ? lcf::EngineVersion::e2k3 : lcf::EngineVersion::e2k;
	bool res = lcf::LSD_Reader::Save(os, save, lcf_engine, Player::encoding);

	DynRpg::Save(slot_id);

#ifdef EMSCRIPTEN
	// Save changed file system
	EM_ASM({
		FS.syncfs(function(err) {
		});
	});
#endif

	return res;
}

bool Scene_Save::IsSlotValid(int) {
	return true;
}
