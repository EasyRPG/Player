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
#include <cassert>
#include <iomanip>
#include <sstream>

#include "system.h"
#include "game_map.h"
#include "game_interpreter_map.h"
#include "game_temp.h"
#include "game_player.h"
#include "lmu_reader.h"
#include "reader_lcf.h"
#include "map_data.h"
#include "main_data.h"
#include "output.h"
#include "util_macro.h"
#include "game_system.h"
#include "filefinder.h"
#include "player.h"
#include "input.h"
#include <boost/scoped_ptr.hpp>

namespace map_anon {
	RPG::SaveMapInfo& map_info = Main_Data::game_data.map_info;
	RPG::SavePartyLocation& location = Main_Data::game_data.party_location;

	std::string chipset_name;
	std::string battleback_name;
	bool need_refresh;

	int parallax_auto_x;
	int parallax_auto_y;
	int parallax_x;
	int parallax_y;

	std::vector<unsigned char> passages_down;
	std::vector<unsigned char> passages_up;
	tEventHash events;
	tCommonEventHash common_events;

	std::auto_ptr<RPG::Map> map;
	int scroll_direction;
	int scroll_rest;
	int scroll_speed;

	boost::scoped_ptr<Game_Interpreter> interpreter;
	Game_Vehicle* vehicles[3];

	bool pan_locked;
	bool pan_wait;
	int pan_speed;
}

using namespace map_anon;

void Game_Map::Init() {
	Dispose();

	map_info.position_x = 0;
	map_info.position_y = 0;
	need_refresh = true;

	map.reset();
	location.map_id = 0;
	scroll_direction = 0;
	scroll_rest = 0;
	scroll_speed = 0;
	interpreter.reset(new Game_Interpreter_Map(0, true));
	map_info.encounter_rate = 0;

	for (int i = 0; i < 3; i++)
		vehicles[i] = new Game_Vehicle((Game_Vehicle::Type) i);

	pan_locked = false;
	pan_wait = false;
	pan_speed = 0;
	location.pan_finish_x = 0;
	location.pan_finish_y = 0;
	location.pan_current_x = 0;
	location.pan_current_y = 0;
}

void Game_Map::Dispose() {
	events.clear();
	common_events.clear();

	if (Main_Data::game_screen) {
		Main_Data::game_screen->Reset();
	}

	map.reset();
}

void Game_Map::Quit() {
	Dispose();
	interpreter.reset();
}

void Game_Map::Setup(int _id) {
	SetupCommon(_id);

	for (size_t i = 0; i < map->events.size(); ++i) {
		events.insert(std::make_pair(map->events[i].ID, EASYRPG_MAKE_SHARED<Game_Event>(location.map_id, map->events[i])));
	}

	for (size_t i = 0; i < Data::commonevents.size(); ++i) {
		common_events.insert(std::make_pair(Data::commonevents[i].ID, EASYRPG_MAKE_SHARED<Game_CommonEvent>(Data::commonevents[i].ID)));
	}

	location.pan_finish_x = 0;
	location.pan_finish_y = 0;
	location.pan_current_x = 0;
	location.pan_current_y = 0;
}

void Game_Map::SetupFromSave() {
	SetupCommon(location.map_id);

	for (size_t i = 0; i < map->events.size(); ++i) {
		EASYRPG_SHARED_PTR<Game_Event> evnt;
		if (i < map_info.events.size()) {
			evnt = EASYRPG_MAKE_SHARED<Game_Event>(location.map_id, map->events[i], map_info.events[i]);
		}
		else {
			evnt = EASYRPG_MAKE_SHARED<Game_Event>(location.map_id, map->events[i]);
		}

		events.insert(std::make_pair(map->events[i].ID, evnt));
	}

	for (size_t i = 0; i < Data::commonevents.size(); ++i) {
		EASYRPG_SHARED_PTR<Game_CommonEvent> evnt;
		if (i < Main_Data::game_data.common_events.size()) {
			evnt = EASYRPG_MAKE_SHARED<Game_CommonEvent>(Data::commonevents[i].ID, false, Main_Data::game_data.common_events[i]);
		}
		else {
			evnt = EASYRPG_MAKE_SHARED<Game_CommonEvent>(Data::commonevents[i].ID, false);
		}

		common_events.insert(std::make_pair(Data::commonevents[i].ID, evnt));
	}

	static_cast<Game_Interpreter_Map*>(interpreter.get())->SetupFromSave(Main_Data::game_data.events.events, 0);

	map_info.Fixup(*map.get());

	// FIXME: Handle Pan correctly
	location.pan_current_x = 0;
	location.pan_current_y = 0;
	location.pan_finish_x = 0;
	location.pan_finish_y = 0;
}

void Game_Map::SetupCommon(int _id) {
	// Execute remaining events (e.g. ones listed after a teleport)
	Update();
	Dispose();

	location.map_id = _id;

	// Try loading EasyRPG map files first, then fallback to normal RPG Maker
	std::stringstream ss;
	ss << "Map" << std::setfill('0') << std::setw(4) << location.map_id << ".emu";

	std::string map_file = FileFinder::FindDefault(ss.str());
	if (map_file.empty()) {
		ss.str("");
		ss << "Map" << std::setfill('0') << std::setw(4) << location.map_id << ".lmu";

		map = LMU_Reader::Load(FileFinder::FindDefault(ss.str()), Player::GetEncoding());
		if (map.get() == NULL) {
			Output::ErrorStr(LcfReader::GetError());
		}
	}
	else {
		map = LMU_Reader::LoadXml(map_file);
		if (map.get() == NULL) {
			Output::ErrorStr(LcfReader::GetError());
		}
	}

	if (map->parallax_flag) {
		SetParallaxName(map->parallax_name);
		SetParallaxScroll(map->parallax_loop_x, map->parallax_loop_y,
			map->parallax_auto_loop_x, map->parallax_auto_loop_y,
			map->parallax_sx, map->parallax_sy);
	}
	else {
		SetParallaxName("");
	}

	SetChipset(map->chipset_id);
	need_refresh = true;

	scroll_direction = 2;
	scroll_rest = 0;
	scroll_speed = 4;
	map_info.encounter_rate = Data::treemap.maps[GetMapIndex(location.map_id)].encounter_steps;

	for (int i = 0; i < 3; i++)
		vehicles[i]->Refresh();

	pan_locked = false;
	pan_wait = false;
	pan_speed = 0;

	// Make RPG_RT happy
	// Otherwise current event not resumed after loading
	location.map_save_count = map->save_count;

	ResetEncounterSteps();
}

void Game_Map::PrepareSave() {
	Main_Data::game_data.events.events =
		static_cast<Game_Interpreter_Map*>(interpreter.get())
			->GetSaveData();
	Main_Data::game_data.events.events_size = Main_Data::game_data.events.events.size();

	map_info.events.clear();

	for (tEventHash::iterator i = events.begin(); i != events.end(); ++i) {
		map_info.events.push_back(i->second->GetSaveData());
	}

	std::vector<RPG::SaveCommonEvent>& save_common_events = Main_Data::game_data.common_events;
	save_common_events.clear();
	save_common_events.resize(Data::commonevents.size());

	for (tCommonEventHash::iterator i = common_events.begin(); i != common_events.end(); ++i) {
		save_common_events.push_back(RPG::SaveCommonEvent());
		save_common_events.back().ID = i->first;
		save_common_events.back().event_data = i->second->GetSaveData();
	}
}

void Game_Map::PlayBgm() {
	int parent_index = 0;
	int current_index = GetMapIndex(location.map_id);

	if ((current_index > -1) && !Data::treemap.maps[current_index].music.name.empty()) {
		switch(Data::treemap.maps[current_index].music_type) {
			case 0: // inherits music from parent
				parent_index = GetMapIndex(Data::treemap.maps[current_index].parent_map);
				if (Data::treemap.maps[parent_index].music.name != "(OFF)" && &Data::treemap.maps[parent_index].music != Game_Temp::map_bgm) {
					Game_Temp::map_bgm = &Data::treemap.maps[parent_index].music;
					Game_System::BgmPlay(*Game_Temp::map_bgm);
				}
				break;
			case 1:  // No Change
				break;
			case 2:  // specific map music
				if (&Data::treemap.maps[current_index].music != Game_Temp::map_bgm) {
					if (Game_Temp::map_bgm != NULL) {
						if (Data::treemap.maps[current_index].music.name == Game_Temp::map_bgm->name) {
							// TODO: Here the volume and pitch must be updated if the song is the same
							return;
						}
					}
					Game_Temp::map_bgm = &Data::treemap.maps[current_index].music;
					Game_System::BgmPlay(*Game_Temp::map_bgm);
				}
		}
	}
}

void Game_Map::Refresh() {
	if (location.map_id > 0) {
		for (tEventHash::iterator i = events.begin(); i != events.end(); ++i) {
			i->second->Refresh();
		}

		for (tCommonEventHash::iterator i = common_events.begin(); i != common_events.end(); ++i) {
			i->second->Refresh();
		}
	}
	need_refresh = false;
}

Game_Interpreter& Game_Map::GetInterpreter() {
	assert(interpreter);
	return *interpreter;
}

void Game_Map::ScrollDown(int distance) {
	map_info.position_y = min(map_info.position_y + distance, (GetHeight() - 15) * SCREEN_TILE_WIDTH);
}

void Game_Map::ScrollLeft(int distance) {
	map_info.position_x = max(map_info.position_x - distance, 0);
}

void Game_Map::ScrollRight(int distance) {
	map_info.position_x = min(map_info.position_x + distance, (GetWidth() - 20) * SCREEN_TILE_WIDTH);
}

void Game_Map::ScrollUp(int distance) {
	map_info.position_y = max(map_info.position_y - distance, 0);
}

bool Game_Map::IsValid(int x, int y) {
	return (x >= 0 && x < GetWidth() && y >= 0 && y < GetHeight());
}

bool Game_Map::IsPassable(int x, int y, int d, const Game_Character* self_event) {
	if (!Game_Map::IsValid(x, y)) return false;

	uint8_t bit = 0;
	switch (d)
	{
		case RPG::EventPage::Direction_down:
			bit = Passable::Down;
			break;

		case RPG::EventPage::Direction_up:
			bit = Passable::Up;
			break;

		case RPG::EventPage::Direction_left:
			bit = Passable::Left;
			break;

		case RPG::EventPage::Direction_right:
			bit = Passable::Right;
			break;

		default:
			assert(false);
	}

	int tile_id;

	if (self_event) {
		bool pass = false;
		for (tEventHash::iterator i = events.begin(); i != events.end(); i++) {
			Game_Event* evnt = i->second.get();
			if (evnt != self_event && evnt->GetX() == x && evnt->GetY() == y) {
				if (!evnt->GetThrough()) {
					if (evnt->GetLayer() == RPG::EventPage::Layers_same) {
						if (self_event->GetX() == x && self_event->GetY() == y)
							pass = true;
						else 
							return false;
					}
					else if (evnt->GetTileId() > 0 && evnt->GetLayer() == RPG::EventPage::Layers_below) {
						// Event layer Chipset Tile
						tile_id = evnt->GetTileId();
						if ((passages_up[tile_id] & Passable::Above) != 0)
							if ((passages_down[tile_id] & bit) == 0)
								return false;
						if ((passages_up[tile_id] & bit) != 0)
							pass = true;
						else
							return false;
					}
				}
			}
		}
		if (pass) // All events here are passable
			return true;
	}

	int const tile_index = x + y * GetWidth();

	tile_id = map->upper_layer[tile_index] - BLOCK_F;
	tile_id = map_info.upper_tiles[tile_id];

	if ((passages_up[tile_id] & bit) == 0) {
		return false;
	}

	if ((passages_up[tile_id] & Passable::Above) == 0)
		return true;

	if (map->lower_layer[tile_index] >= BLOCK_E) {
		tile_id = map->lower_layer[tile_index] - BLOCK_E;
		tile_id = map_info.lower_tiles[tile_id];
		tile_id += 18;

		if ((passages_down[tile_id] & bit) == 0)
			return false;

	} else if (map->lower_layer[tile_index] >= BLOCK_D) {
		tile_id = (map->lower_layer[tile_index] - BLOCK_D) / 50;
		int16_t autotile_id = map->lower_layer[tile_index] - BLOCK_D - tile_id * 50;

		tile_id += 6;

		if (((passages_down[tile_id] & Passable::Wall) != 0) && (
				(autotile_id >= 20 && autotile_id <= 23) ||
				(autotile_id >= 33 && autotile_id <= 37) ||
				autotile_id == 42 ||
				autotile_id == 43 ||
				autotile_id == 45
			))
			return true;

		if ((passages_down[tile_id] & bit) == 0)
			return false;

	} else if (map->lower_layer[tile_index] >= BLOCK_C) {
		tile_id = (map->lower_layer[tile_index] - BLOCK_C) / 50 + 3;

		if ((passages_down[tile_id] & bit) == 0)
			return false;

	} else if (map->lower_layer[tile_index] < BLOCK_C) {
		tile_id = map->lower_layer[tile_index] / 1000;

		if ((passages_down[tile_id] & bit) == 0)
			return false;
	}

	return true;
}

bool Game_Map::IsLandable(int x, int y, const Game_Character *self_event)
{
    int tile_id;

    if (self_event) {
        for (tEventHash::iterator i = events.begin(); i != events.end(); i++) {
            Game_Event* evnt = i->second.get();
            if (evnt != self_event && evnt->GetX() == x && evnt->GetY() == y) {
                if (!evnt->GetThrough()) {
                    if (evnt->GetLayer() == RPG::EventPage::Layers_same) {
                        return false;
                    }
                    else if (evnt->GetTileId() >= 0 && evnt->GetLayer() == RPG::EventPage::Layers_below) {
                        // Event layer Chipset Tile
                        tile_id = i->second->GetTileId();
                        return (passages_up[tile_id] & Passable::Down ||
                                passages_up[tile_id] & Passable::Right ||
                                passages_up[tile_id] & Passable::Left ||
                                passages_up[tile_id] & Passable::Up);
                    }
                }
            }
        }
    }

    int const tile_index = x + y * GetWidth();

    tile_id = map->upper_layer[tile_index] - BLOCK_F;
    tile_id = map_info.upper_tiles[tile_id];

    if ((passages_up[tile_id] & Passable::Down) == 0 &&
        (passages_up[tile_id] & Passable::Right) == 0 &&
        (passages_up[tile_id] & Passable::Left) == 0 &&
        (passages_up[tile_id] & Passable::Up) == 0) {
        return false;
    }

    if ((passages_up[tile_id] & Passable::Above) == 0)
        return true;

    if (map->lower_layer[tile_index] >= BLOCK_E) {
        tile_id = map->lower_layer[tile_index] - BLOCK_E;
        tile_id = map_info.lower_tiles[tile_id];
        tile_id += 18;

        if ((passages_down[tile_id] & Passable::Down) == 0 &&
            (passages_down[tile_id] & Passable::Right) == 0 &&
            (passages_down[tile_id] & Passable::Left) == 0 &&
            (passages_down[tile_id] & Passable::Up) == 0)
            return false;

    } else if (map->lower_layer[tile_index] >= BLOCK_D) {
        tile_id = (map->lower_layer[tile_index] - BLOCK_D) / 50;
        int16_t autotile_id = map->lower_layer[tile_index] - BLOCK_D - tile_id * 50;

        tile_id += 6;

        if (((passages_down[tile_id] & Passable::Wall) != 0) && (
                (autotile_id >= 20 && autotile_id <= 23) ||
                (autotile_id >= 33 && autotile_id <= 37) ||
                autotile_id == 42 ||
                autotile_id == 43 ||
                autotile_id == 45
            ))
            return true;

        if ((passages_down[tile_id] & Passable::Down) == 0 &&
            (passages_down[tile_id] & Passable::Right) == 0 &&
            (passages_down[tile_id] & Passable::Left) == 0 &&
            (passages_down[tile_id] & Passable::Up) == 0)
            return false;

    } else if (map->lower_layer[tile_index] >= BLOCK_C) {
        tile_id = (map->lower_layer[tile_index] - BLOCK_C) / 50 + 3;

        if ((passages_down[tile_id] & Passable::Down) == 0 &&
            (passages_down[tile_id] & Passable::Right) == 0 &&
            (passages_down[tile_id] & Passable::Left) == 0 &&
            (passages_down[tile_id] & Passable::Up) == 0)
            return false;

    } else if (map->lower_layer[tile_index] < BLOCK_C) {
        tile_id = map->lower_layer[tile_index] / 1000;

        if ((passages_down[tile_id] & Passable::Down) == 0 &&
            (passages_down[tile_id] & Passable::Right) == 0 &&
            (passages_down[tile_id] & Passable::Left) == 0 &&
            (passages_down[tile_id] & Passable::Up) == 0)
            return false;
    }

    return true;
}

bool Game_Map::IsBush(int /* x */, int /* y */) {
	// TODO
	return false;
}

bool Game_Map::IsCounter(int x, int y) {
	if (!Game_Map::IsValid(x, y)) return false;
	int const tile_id = map->upper_layer[x + y * GetWidth()];
	if (tile_id < BLOCK_F) return false;
	int const index = map_info.lower_tiles[passages_up[tile_id - BLOCK_F]];
	return !!(Data::chipsets[map_info.chipset_id - 1].passable_data_upper[index] & Passable::Counter);
}

int Game_Map::GetTerrainTag(int const x, int const y) {
	unsigned const chipID = map->lower_layer[x + y * GetWidth()];
	unsigned const chip_index =
		(chipID <  3050)?  0 + chipID/1000 :
		(chipID <  4000)?  4 + (chipID-3050)/50 :
		(chipID <  5000)?  6 + (chipID-4000)/50 :
		(chipID <  5144)? 18 + (chipID-5000) :
		0;
	unsigned const chipset_index = map_info.chipset_id - 1;

	assert(chipset_index < Data::data.chipsets.size());
	assert(chip_index < Data::data.chipsets[chipset_index].terrain_data.size());

	return Data::data.chipsets[chipset_index].terrain_data[chip_index];
}

bool Game_Map::AirshipLandOk(int const x, int const y) {
	return Data::data.terrains[GetTerrainTag(x, y) -1].airship_land;
}

void Game_Map::GetEventsXY(std::vector<Game_Event*>& events, int x, int y) {
	std::vector<Game_Event*> result;

	tEventHash::const_iterator i;
	for (i = Game_Map::GetEvents().begin(); i != Game_Map::GetEvents().end(); i++) {
		if (i->second->GetRealX() == x * SCREEN_TILE_WIDTH && i->second->GetRealY() == y * SCREEN_TILE_WIDTH) {
			result.push_back(i->second.get());
		}
	}

	events.swap(result);
}

bool Game_Map::LoopHorizontal() {
	return map->scroll_type == 2 || map->scroll_type == 3;
}

bool Game_Map::LoopVertical() {
	return map->scroll_type == 1 || map->scroll_type == 3;
}

int Game_Map::RoundX(int x) {
	if ( LoopHorizontal() ) {
		return (x + GetWidth()) % GetWidth();
	} else {
		return x;
	}
}

int Game_Map::RoundY(int y) {
	if ( LoopVertical() ) {
		return (y + GetHeight()) % GetHeight();
	} else {
		return y;
	}
}

int Game_Map::XwithDirection(int x, int direction) {
	return RoundX(x + (direction == RPG::EventPage::Direction_right ? 1 : direction == RPG::EventPage::Direction_left ? -1 : 0));
}

int Game_Map::YwithDirection(int y, int direction) {
	return RoundY(y + (direction == RPG::EventPage::Direction_down ? 1 : direction == RPG::EventPage::Direction_up ? -1 : 0));
}

int Game_Map::CheckEvent(int x, int y) {
	tEventHash::iterator i;
	for (i = events.begin(); i != events.end(); i++) {
		if (i->second->GetX() == x && i->second->GetY() == y) {
			return i->second->GetId();
		}
	}

	return 0;
}

void Game_Map::StartScroll(int direction, int distance, int speed) {
	scroll_direction = direction;
	scroll_rest = distance * SCREEN_TILE_WIDTH;
	scroll_speed = speed;
}

bool Game_Map::IsScrolling() {
	return scroll_rest > 0;
}

void Game_Map::UpdateScroll() {
	if (scroll_rest > 0) {
		int distance = (1 << scroll_speed) / 2;
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
}

void Game_Map::Update() {
	if (need_refresh) Refresh();
	UpdateScroll();
	UpdatePan();
	UpdateParallax();

	for (tEventHash::iterator i = events.begin();
		i != events.end(); i++) {
		i->second->Update();
	}

	for (tCommonEventHash::iterator i = common_events.begin();
		i != common_events.end(); i++) {
		i->second->Update();
	}

	for (int i = 0; i < 3; i++)
		vehicles[i]->Update();
}

RPG::Map const& Game_Map::GetMap() {
	return *map;
}


int Game_Map::GetMapId() {
	return location.map_id;
}

int Game_Map::GetWidth() {
	return map->width;
}

int Game_Map::GetHeight() {
	return map->height;
}

std::vector<RPG::Encounter>& Game_Map::GetEncounterList() {
	return Data::treemap.maps[GetMapIndex(location.map_id)].encounters;
}

int Game_Map::GetEncounterRate() {
	return map_info.encounter_rate;
}

void Game_Map::SetEncounterRate(int step) {
	map_info.encounter_rate = step;
}

int Game_Map::GetEncounterSteps() {
	return location.encounter_steps;
}

void Game_Map::UpdateEncounterSteps() {
	if (Player::debug_flag &&
		Input::IsPressed(Input::DEBUG_THROUGH)) {
			return;
	}

	int x = Main_Data::game_player->GetX();
	int y = Main_Data::game_player->GetY();
	int terrain_id = GetTerrainTag(x, y);
	const RPG::Terrain& terrain = Data::terrains[terrain_id - 1];

	location.encounter_steps -= terrain.encounter_rate;

	if (location.encounter_steps <= 0) {
		ResetEncounterSteps();
		PrepareEncounter();
	}
}

void Game_Map::ResetEncounterSteps() {
	int rate = GetEncounterRate();
	if (rate > 0) {
		int throw_one = rand() / (RAND_MAX / rate + 1);
		int throw_two = rand() / (RAND_MAX / rate + 1);

		// *100 to handle terrain rate better
		location.encounter_steps = (throw_one + throw_two + 1) * 100;
	}
}

void Game_Map::GetEncountersAt(int x, int y, std::vector<int>& out) {
	for (unsigned int i = 0; i < Data::treemap.maps.size(); ++i) {
		RPG::MapInfo& map = Data::treemap.maps[i];

		if (map.ID == location.map_id) {
			std::vector<RPG::Encounter>& encounters = map.encounters;
			for (std::vector<RPG::Encounter>::iterator it = encounters.begin();
				it != encounters.end(); ++it) {
					out.push_back((*it).troop_id);
			}
		} else if (map.parent_map == location.map_id && map.type == 2) {
			// Area
			Rect area_rect(map.area_rect.l, map.area_rect.t, map.area_rect.r - map.area_rect.l, map.area_rect.b - map.area_rect.t);
			Rect player_rect(x, y, 1, 1);

			if (!player_rect.IsOutOfBounds(area_rect)) {
				std::vector<RPG::Encounter>& encounters = map.encounters;
				for (std::vector<RPG::Encounter>::iterator it = encounters.begin();
					it != encounters.end(); ++it) {
						out.push_back((*it).troop_id);
				}
			}
		}
	}
}

bool Game_Map::PrepareEncounter() {
	if (GetEncounterRate() <= 0) {
		return false;
	}

	int x = Main_Data::game_player->GetX();
	int y = Main_Data::game_player->GetY();

	std::vector<int> encounters;
	GetEncountersAt(x, y, encounters);

	if (encounters.empty()) {
		// No enemies on this map :(
		return false;
	}

	Game_Temp::battle_terrain_id = Game_Map::GetTerrainTag(Main_Data::game_player->GetX(), Main_Data::game_player->GetY());
	Game_Temp::battle_background = "";
	Game_Temp::battle_troop_id = encounters[rand() / (RAND_MAX / encounters.size() + 1)];
	Game_Temp::battle_escape_mode = -1;
	Game_Temp::battle_calling = true;

	return true;
}

std::vector<short>& Game_Map::GetMapDataDown() {
	return map->lower_layer;
}

std::vector<short>& Game_Map::GetMapDataUp() {
	return map->upper_layer;
}

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

int Game_Map::GetDisplayX() {
	return map_info.position_x;
}
void Game_Map::SetDisplayX(int new_display_x) {
	map_info.position_x = new_display_x;
}

int Game_Map::GetDisplayY() {
	return map_info.position_y;
}
void Game_Map::SetDisplayY(int new_display_y) {
	map_info.position_y = new_display_y;
}

bool Game_Map::GetNeedRefresh() {
	return need_refresh;
}
void Game_Map::SetNeedRefresh(bool new_need_refresh) {
	need_refresh = new_need_refresh;
}

std::vector<unsigned char>& Game_Map::GetPassagesDown() {
	return passages_down;
}

std::vector<unsigned char>& Game_Map::GetPassagesUp() {
	return passages_up;
}

std::vector<short>& Game_Map::GetTerrainTags() {
	return Data::chipsets[map_info.chipset_id - 1].terrain_data;
}

tEventHash& Game_Map::GetEvents() {
	return events;
}

tCommonEventHash& Game_Map::GetCommonEvents() {
	return common_events;
}

void Game_Map::SetParallaxName(const std::string& name) {
	map_info.parallax_name = name;
}

void Game_Map::SetParallaxScroll(bool horz, bool vert,
								 bool horz_auto, bool vert_auto,
								 int horz_speed, int vert_speed) {
	map_info.parallax_horz = horz;
	map_info.parallax_vert = vert;
	map_info.parallax_horz_auto = horz_auto;
	map_info.parallax_vert_auto = vert_auto;
	map_info.parallax_horz_speed = horz_speed;
	map_info.parallax_vert_speed = vert_speed;
	parallax_auto_x = 0;
	parallax_auto_y = 0;
	parallax_x = 0;
	parallax_y = 0;
}

int Game_Map::GetMapIndex(int id) {
	for (unsigned int i = 0; i < Data::treemap.maps.size(); ++i) {
		if (Data::treemap.maps[i].ID == id) {
			return i;
		}
	}
	//nothing found
	return -1;
}

void Game_Map::SetChipset(int id) {
	map_info.chipset_id = id;
	RPG::Chipset &chipset = Data::chipsets[map_info.chipset_id - 1];
	chipset_name = chipset.chipset_name;
	passages_down = chipset.passable_data_lower;
	passages_up = chipset.passable_data_upper;
	if (passages_down.size() < 162)
		passages_down.resize(162, (unsigned char) 0x0F);
	if (passages_up.size() < 144)
		passages_up.resize(144, (unsigned char) 0x0F);
	for (uint8_t i = 0; i < 144; i++) {
		map_info.lower_tiles[i] = i;
		map_info.upper_tiles[i] = i;
	}
}

Game_Vehicle* Game_Map::GetVehicle(Game_Vehicle::Type which) {
	return vehicles[which];
}

void Game_Map::SubstituteDown(int old_id, int new_id) {
	for (size_t i = 0; i < map_info.lower_tiles.size(); ++i) {
		if (map_info.lower_tiles[i] == old_id) {
			map_info.lower_tiles[i] = (uint8_t) new_id;
		}
	}
}

void Game_Map::SubstituteUp(int old_id, int new_id) {
	for (size_t i = 0; i < map_info.upper_tiles.size(); ++i) {
		if (map_info.upper_tiles[i] == old_id) {
			map_info.upper_tiles[i] = (uint8_t) new_id;
		}
	}
}

void Game_Map::LockPan() {
	pan_locked = true;
}

void Game_Map::UnlockPan() {
	pan_locked = false;
}

void Game_Map::StartPan(int direction, int distance, int speed, bool wait) {
	distance *= SCREEN_TILE_WIDTH;
	switch (direction) {
		case PanUp:		location.pan_finish_y -= distance;		break;
		case PanRight:	location.pan_finish_x += distance;		break;
		case PanDown:	location.pan_finish_y += distance;		break;
		case PanLeft:	location.pan_finish_x -= distance;		break;
	}
	pan_speed = speed;
	pan_wait = wait;
}

void Game_Map::ResetPan(int speed, bool wait) {
	location.pan_finish_x = 0;
	location.pan_finish_y = 0;
	pan_speed = speed;
	pan_wait = wait;
}

void Game_Map::UpdatePan() {
	if (!IsPanActive())
		return;

	int step = 2 << (pan_speed - 1);
	int dx = location.pan_finish_x - location.pan_current_x;
	int dy = location.pan_finish_y - location.pan_current_y;

	if (dx > 0)
		location.pan_current_x += std::min(step, dx);
	else if (dx < 0)
		location.pan_current_x -= std::min(step, -dx);

	if (dy > 0)
		location.pan_current_y += std::min(step, dy);
	else if (dy < 0)
		location.pan_current_y -= std::min(step, -dy);
}

bool Game_Map::IsPanActive() {
	return location.pan_current_x != location.pan_finish_x || location.pan_current_y != location.pan_finish_y;
}

bool Game_Map::IsPanWaiting() {
	return IsPanActive() && pan_wait;
}

bool Game_Map::IsPanLocked() {
	return pan_locked;
}

int Game_Map::GetPanX() {
	return location.pan_current_x;
}

int Game_Map::GetPanY() {
	return location.pan_current_y;
}

void Game_Map::UpdateParallax() {
	if (map_info.parallax_name.empty())
		return;

	if (map_info.parallax_horz) {
		if (map_info.parallax_horz_auto) {
			int step =
				(map_info.parallax_horz_speed > 0) ? 1 << map_info.parallax_horz_speed :
				(map_info.parallax_horz_speed < 0) ? 1 << -map_info.parallax_horz_speed :
				0;
			parallax_auto_x += step;
		}
		parallax_x = map_info.position_x * 2 + parallax_auto_x;
	} else
		parallax_x = 0;

	if (map_info.parallax_vert) {
		if (map_info.parallax_vert_auto) {
			int step =
				(map_info.parallax_vert_speed > 0) ? 1 << map_info.parallax_vert_speed :
				(map_info.parallax_vert_speed < 0) ? 1 << -map_info.parallax_vert_speed :
				0;
			parallax_auto_y += step;
		}
		parallax_y = map_info.position_y * 2 + parallax_auto_y;
	} else
		parallax_y = 0;
}

int Game_Map::GetParallaxX() {
	int px = parallax_x - map_info.position_x * (SCREEN_TILE_WIDTH / 64);
	return (px < 0) ? -(-px / 64) : (px / 64);
}

int Game_Map::GetParallaxY() {
	int py = parallax_y - map_info.position_y * (SCREEN_TILE_WIDTH / 64);
	return (py < 0) ? -(-py / 64) : (py / 64);
}

const std::string& Game_Map::GetParallaxName() {
	return map_info.parallax_name;
}
