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
#include "game_actors.h"
#include "game_map.h"
#include "game_message.h"
#include "game_party.h"
#include "game_player.h"
#include "game_system.h"
#include "game_temp.h"
#include "lsd_reader.h"
#include "scene_load.h"
#include "scene_file.h"
#include "scene_map.h"
#include "reader_util.h"

Scene_Load::Scene_Load() :
	Scene_File(Data::terms.load_game_message) {
	Scene::type = Scene::Load;
}

void Scene_Load::Action(int index) {
	std::stringstream ss;
	ss << "Save" << (index <= 8 ? "0" : "") << (index + 1) << ".lsd";

	std::auto_ptr<RPG::Save> save = LSD_Reader::Load(FileFinder::FindDefault(*tree, ss.str()),
		ReaderUtil::GetEncoding(FileFinder::FindDefault(INI_NAME)));

	CreateGameObjects();

	SetupSavegameData(save);

	Scene::Push(EASYRPG_MAKE_SHARED<Scene_Map>(true), true);
}

void Scene_Load::CreateGameObjects() {
	Game_Temp::Init();
	Main_Data::game_screen.reset(new Game_Screen());
	Game_Actors::Init();
	Game_Message::Init();
	Game_Map::Init();
	Main_Data::game_player.reset(new Game_Player());
}

void Scene_Load::SetupSavegameData(std::auto_ptr<RPG::Save> save) {
	RPG::SaveSystem system = Main_Data::game_data.system;

	Main_Data::game_data = *save.get();

	Main_Data::game_data.party_location.Fixup();
	Main_Data::game_data.system.Fixup();
	Main_Data::game_data.screen.Fixup();
	Game_Actors::Fixup();

	Game_Map::SetupFromSave();

	Main_Data::game_player->MoveTo(
		save->party_location.position_x, save->party_location.position_y);
	Main_Data::game_player->Refresh();

	RPG::Music current_music = Main_Data::game_data.system.current_music;
	Main_Data::game_data.system.current_music = RPG::Music();
	Game_System::BgmPlay(current_music);
}

bool Scene_Load::IsSlotValid(int index) {
	return file_windows[index]->IsValid();
}
