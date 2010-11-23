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
#include "game_map.h"
#include "lmu_reader.h"
#include "main_data.h"
#include "output.h"
#include "util_macro.h"

////////////////////////////////////////////////////////////
namespace {
	std::string chipset_name;
	std::string battleback_name;
	std::string panorama_name;
	int panorama_type;
	int panorama_speed;
	int display_x;
	int display_y;
	bool need_refresh;

	std::vector<unsigned char> passages_down;
	std::vector<unsigned char> passages_up;
	std::vector<short> terrain_tags;
	std::vector<Game_Event*> events;

	RPG::Map* map;
	int map_id;
	int scroll_direction;
	int scroll_rest;
	int scroll_speed;
}

////////////////////////////////////////////////////////////
void Game_Map::Init() {
	panorama_type = 0;
	panorama_speed = 0;
	display_x = 0;
	display_y = 0;
	need_refresh = false;
	
	map = NULL;
	map_id = 0;
	scroll_direction = 0;
	scroll_rest = 0;
	scroll_speed = 0;
}

////////////////////////////////////////////////////////////
void Game_Map::Dispose() {
	for (size_t i = 0; i < events.size(); i++) {
		delete events[i];
	}
	events.clear();

	delete map;
	map = NULL;
}

////////////////////////////////////////////////////////////
void Game_Map::Setup(int _id) {
	map_id = _id;
	char file[12];
	sprintf(file, "Map%04d.lmu", map_id);

	map = LMU_Reader::LoadMap(file);
	if (map == NULL) {
		Output::ErrorStr(Reader::GetError());
	}

	RPG::Chipset chipset = Data::chipsets[map->chipset_id - 1];
	chipset_name = chipset.chipset_name;
	passages_down = chipset.passable_data_lower;
	passages_up = chipset.passable_data_lower;
	terrain_tags = chipset.terrain_data;
	panorama_speed = chipset.animation_speed;
	panorama_type = chipset.animation_type;
	display_x = 0;
	display_y = 0;
	need_refresh = false;
	/*events.clear();
	for i in @map.events.keys
		@events[i] = Game_Event.new(@map_id, @map.events[i])
	end
	@common_events.clear();
	for i in 1...$data_common_events.size
		@common_events[i] = Game_CommonEvent.new(i)
	end*/
	scroll_direction = 2;
	scroll_rest = 0;
	scroll_speed = 4;
}

////////////////////////////////////////////////////////////
void Game_Map::Autoplay() {
}

////////////////////////////////////////////////////////////
void Game_Map::Refresh() {
	/*if (map_id > 0) {
		for (int i = 0; i < events.size(); i++) {
			events[i]->Refresh();
		}
		for (int i = 0; i < common_events.size(); i++) {
			common_events[i]->Refresh();
		}
	}*/
	need_refresh = false;
}

////////////////////////////////////////////////////////////
void Game_Map::ScrollDown(int distance) {
	display_y = min(display_y + distance, (map->height - 15) * 128);
}

////////////////////////////////////////////////////////////
void Game_Map::ScrollLeft(int distance) {
	display_x = max(display_x - distance, 0);
}

////////////////////////////////////////////////////////////
void Game_Map::ScrollRight(int distance) {
	display_x = min(display_x + distance, (map->width - 20) * 128);
}

////////////////////////////////////////////////////////////
void Game_Map::ScrollUp(int distance) {
	display_y = max(display_y - distance, 0);
}

////////////////////////////////////////////////////////////
bool Game_Map::IsValid(int x, int y) {
	return (x >= 0 && x < GetWidth() && y >= 0 && y < GetHeight());
}

////////////////////////////////////////////////////////////
bool Game_Map::IsPassable(int x, int y, int d, Game_Event* self_event) {
	return true;
}

////////////////////////////////////////////////////////////
bool Game_Map::IsBush(int x, int y) {
	return false;
}

////////////////////////////////////////////////////////////
bool Game_Map::IsCounter(int x, int y) {
	return false;
}

////////////////////////////////////////////////////////////
int Game_Map::GetTerrainTag(int x, int y) {
	return 0;
}

////////////////////////////////////////////////////////////
int Game_Map::CheckEvent(int x, int y) {
	/*for (int i = 0; i < events.size(); i++) {
		if (events[i]->x == x && events[i]->y == y) {
			return events[i]->id;
		}
	}*/
	return 0;
}

////////////////////////////////////////////////////////////
void Game_Map::StartScroll(int direction, int distance, int speed) {
	scroll_direction = direction;
	scroll_rest = distance * 128;
	scroll_speed = speed;
}

////////////////////////////////////////////////////////////
bool Game_Map::IsScrolling() {
	return scroll_rest > 0;
}

////////////////////////////////////////////////////////////
void Game_Map::Update() {
	if (need_refresh) Refresh();
	if (scroll_rest > 0) {
		int distance = (1 << scroll_speed);
		switch (scroll_direction) {
			case 2:
				ScrollDown(distance);
				break;
			case 4:
				ScrollLeft(distance);
				break;
			case 6:
				ScrollRight(distance);
				break;
			case 8:
				ScrollUp(distance);
				break;
		}
		scroll_rest -= distance;
	}
	
	/*for (int i = 0; i < events.size(); i++) {
		Game_Map::events[i].Update();
	}

	for (int i = 0; i < common_events.size(); i++) {
		common_events[i].Update();
	}*/
}

////////////////////////////////////////////////////////////
int Game_Map::GetMapId() {
	return map_id;
}

int Game_Map::GetWidth() {
	return map->width;
}

int Game_Map::GetHeight() {
	return map->height;
}

std::vector<RPG::Encounter>& Game_Map::GetEncounterList() {
	return Data::treemap.maps[map_id].encounters;
}

int Game_Map::GetEncounterStep() {
	return Data::treemap.maps[map_id].encounter_steps;
}

std::vector<short>& Game_Map::GetMapDataDown() {
	return map->lower_layer;
}

std::vector<short>& Game_Map::GetMapDataUp() {
	return map->upper_layer;
}

////////////////////////////////////////////////////////////
std::string& Game_Map::GetChipsetName() {
	return chipset_name;
}
void Game_Map::SetChipsetName(std::string new_chipset_name) {
	chipset_name = new_chipset_name;
}

std::string& Game_Map::GetBattlebackName() {
	return battleback_name;
}
void Game_Map::SetBattlebackName(std::string new_battleback_name) {
	battleback_name = new_battleback_name;
}

std::string& Game_Map::GetPanoramaName() {
	return panorama_name;
}
void Game_Map::SetPanoramaName(std::string new_panorama_name) {
	panorama_name = new_panorama_name;
}

int Game_Map::GetPanoramaType() {
	return panorama_type;
}
void Game_Map::SetPanoramaType(int new_panorama_type) {
	panorama_type = new_panorama_type;
}

int Game_Map::GetPanoramaSpeed() {
	return panorama_speed;
}
void Game_Map::SetPanoramaSpeed(int new_panorama_speed) {
	panorama_speed = new_panorama_speed;
}

int Game_Map::GetDisplayX() {
	return display_x;
}
void Game_Map::SetDisplayX(int new_display_x) {
	display_x = new_display_x;
}

int Game_Map::GetDisplayY() {
	return display_y;
}
void Game_Map::SetDisplayY(int new_display_y) {
	display_y = new_display_y;
}

bool Game_Map::GetNeedRefresh() {
	return need_refresh;
}
void Game_Map::SetNeedRefresh(bool new_need_refresh) {
	need_refresh = new_need_refresh;
}

////////////////////////////////////////////////////////////
std::vector<unsigned char>& Game_Map::GetPassagesDown() {
	return Data::chipsets[map->chipset_id - 1].passable_data_lower;
}

std::vector<unsigned char>& Game_Map::GetPassagesUp() {
	return Data::chipsets[map->chipset_id - 1].passable_data_upper;
}

std::vector<short>& Game_Map::GetTerrainTags() {
	return Data::chipsets[map->chipset_id - 1].terrain_data;
}

std::vector<Game_Event*>& Game_Map::GetEvents() {
	return events;
}
