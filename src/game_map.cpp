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
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <algorithm>

#include "async_handler.h"
#include "system.h"
#include "battle_animation.h"
#include "game_battle.h"
#include "game_battler.h"
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

namespace {
	RPG::SaveMapInfo& map_info = Main_Data::game_data.map_info;
	RPG::SavePartyLocation& location = Main_Data::game_data.party_location;

	std::string chipset_name;
	std::string battleback_name;
	bool need_refresh;

	int parallax_x;
	int parallax_y;
	int parallax_width;
	int parallax_height;
	int animation_type;
	bool animation_fast;

	std::vector<unsigned char> passages_down;
	std::vector<unsigned char> passages_up;
	tEventHash events;
	tCommonEventHash common_events;

	std::auto_ptr<RPG::Map> map;
	int scroll_direction;
	int scroll_rest;
	int scroll_speed;

	boost::scoped_ptr<Game_Interpreter> interpreter;
	std::vector<EASYRPG_SHARED_PTR<Game_Interpreter> > free_interpreters;
	std::vector<EASYRPG_SHARED_PTR<Game_Vehicle> > vehicles;
	std::vector<Game_Character*> pending;

	boost::scoped_ptr<BattleAnimation> animation;

	bool pan_locked;
	bool pan_wait;
	int pan_speed;
	bool ready;
}

void Game_Map::Init() {
	Dispose();

	map_info.position_x = 0;
	map_info.position_y = 0;
	need_refresh = true;

	location.map_id = 0;
	scroll_direction = 0;
	scroll_rest = 0;
	scroll_speed = 0;
	interpreter.reset(new Game_Interpreter_Map(0, true));
	map_info.encounter_rate = 0;

	for (size_t i = 0; i < Data::commonevents.size(); ++i) {
		common_events.insert(std::make_pair(Data::commonevents[i].ID, EASYRPG_MAKE_SHARED<Game_CommonEvent>(Data::commonevents[i].ID)));
	}

	vehicles.clear();
	for (int i = 0; i < 3; i++)
		vehicles.push_back(EASYRPG_MAKE_SHARED<Game_Vehicle>((Game_Vehicle::Type) (i + 1)));

	pan_locked = false;
	pan_wait = false;
	pan_speed = 0;
	location.pan_finish_x = 0;
	location.pan_finish_y = 0;
	location.pan_current_x = 0;
	location.pan_current_y = 0;
	ready = false;
}

void Game_Map::Dispose() {
	events.clear();
	pending.clear();

	if (Main_Data::game_screen) {
		Main_Data::game_screen->Reset();
	}

	map.reset();
	animation.reset();
}

void Game_Map::Quit() {
	Dispose();

	common_events.clear();
	interpreter.reset();
}

void Game_Map::Setup(int _id) {
	SetupCommon(_id);

	for (size_t i = 0; i < map->events.size(); ++i) {
		events.insert(std::make_pair(map->events[i].ID, EASYRPG_MAKE_SHARED<Game_Event>(location.map_id, map->events[i])));
	}

	location.pan_finish_x = 0;
	location.pan_finish_y = 0;
	location.pan_current_x = 0;
	location.pan_current_y = 0;
	ready = true;
}

void Game_Map::SetupFromSave() {
	SetupCommon(location.map_id);

	// Make main interpreter "busy" if save contained events to prevent auto-events from starting
	static_cast<Game_Interpreter_Map*>(interpreter.get())->SetupFromSave(Main_Data::game_data.events.events, 0);

	for (size_t i = 0; i < map->events.size(); ++i) {
		EASYRPG_SHARED_PTR<Game_Event> evnt;
		if (i < map_info.events.size()) {
			evnt = EASYRPG_MAKE_SHARED<Game_Event>(location.map_id, map->events[i], map_info.events[i]);
		}
		else {
			evnt = EASYRPG_MAKE_SHARED<Game_Event>(location.map_id, map->events[i]);
		}

		events.insert(std::make_pair(map->events[i].ID, evnt));

		if (evnt->IsMoveRouteOverwritten())
			pending.push_back(evnt.get());
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

	for (size_t i = 0; i < 3; i++)
		if (vehicles[i]->IsMoveRouteOverwritten())
			pending.push_back(vehicles[i].get());

	map_info.Fixup(*map.get());

	// FIXME: Handle Pan correctly
	location.pan_current_x = 0;
	location.pan_current_y = 0;
	location.pan_finish_x = 0;
	location.pan_finish_y = 0;
	ready = true;
}

void Game_Map::SetupCommon(int _id) {
	ready = false;
	Dispose();

	location.map_id = _id;

	// Try loading EasyRPG map files first, then fallback to normal RPG Maker
	std::stringstream ss;
	ss << "Map" << std::setfill('0') << std::setw(4) << location.map_id << ".emu";

	std::string map_file = FileFinder::FindDefault(ss.str());
	if (map_file.empty()) {
		ss.str("");
		ss << "Map" << std::setfill('0') << std::setw(4) << location.map_id << ".lmu";
		map_file = FileFinder::FindDefault(ss.str());

#ifdef _WIN32
		map_file = ReaderUtil::Recode(map_file, "UTF-8", ReaderUtil::GetLocaleEncoding());
#endif

		map = LMU_Reader::Load(map_file, Player::encoding);
	} else {
#ifdef _WIN32
		map_file = ReaderUtil::Recode(map_file, "UTF-8", ReaderUtil::GetLocaleEncoding());
#endif

		map = LMU_Reader::LoadXml(map_file);
	}
	Output::Debug("Loading Map %s", map_file.c_str());

	if (map.get() == NULL) {
		Output::ErrorStr(LcfReader::GetError());
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

	int current_index = GetMapIndex(location.map_id);
	map_info.encounter_rate = Data::treemap.maps[current_index].encounter_steps;

	ss.str("");
	for (int cur = current_index;
		GetMapIndex(Data::treemap.maps[cur].parent_map) != cur;
		cur = GetMapIndex(Data::treemap.maps[cur].parent_map)) {
		if (cur != current_index) {
			ss << " < ";
		}
		ss << Data::treemap.maps[cur].name.c_str();
	}
	Output::Debug("Tree: %s", ss.str().c_str());

	while (Data::treemap.maps[current_index].save == 0 && GetMapIndex(Data::treemap.maps[current_index].parent_map) != current_index) {
		current_index = GetMapIndex(Data::treemap.maps[current_index].parent_map);
	}
	Game_System::SetAllowSave(Data::treemap.maps[current_index].save == 1);

	for (size_t i = 0; i < 3; i++)
		vehicles[i]->Refresh();

	if (Main_Data::game_player->IsMoveRouteOverwritten())
		pending.push_back(Main_Data::game_player.get());

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

	for (tCommonEventHash::iterator i = common_events.begin(); i != common_events.end(); ++i) {
		save_common_events.push_back(RPG::SaveCommonEvent());
		save_common_events.back().ID = i->first;
		save_common_events.back().event_data = i->second->GetSaveData();
	}
}

void Game_Map::PlayBgm() {
	int current_index = GetMapIndex(location.map_id);

	while (Data::treemap.maps[current_index].music_type == 0 && GetMapIndex(Data::treemap.maps[current_index].parent_map) != current_index) {
		current_index = GetMapIndex(Data::treemap.maps[current_index].parent_map);
	}

	if ((current_index > -1) && !Data::treemap.maps[current_index].music.name.empty()) {
		if (Data::treemap.maps[current_index].music_type == 1) {
			return;
		}
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

void Game_Map::ReserveInterpreterDeletion(EASYRPG_SHARED_PTR<Game_Interpreter> interpreter) {
	free_interpreters.push_back(interpreter);
}

void Game_Map::ScrollDown(int distance) {
	if (LoopVertical()) {
		int height = GetHeight() * SCREEN_TILE_WIDTH;
		map_info.position_y = (map_info.position_y + distance + height) % height;
		parallax_y -= map_info.parallax_vert ? distance / 2 : 0;
	} else {
		if (map_info.position_y + distance <= (GetHeight() - 15) * SCREEN_TILE_WIDTH) {
			map_info.position_y += distance;
			if (map_info.parallax_vert)
				parallax_y -= distance / 2;
			else if (!LoopVertical() && GetHeight() > 15 && parallax_height > SCREEN_TARGET_HEIGHT)
				parallax_y -=
					std::min(distance, distance * (parallax_height - SCREEN_TARGET_HEIGHT) / (GetHeight() - 15) / (SCREEN_TILE_WIDTH / TILE_SIZE));
		}
	}
}

void Game_Map::ScrollLeft(int distance) {
	if (LoopHorizontal()) {
		int width = GetWidth() * SCREEN_TILE_WIDTH;
		map_info.position_x = (map_info.position_x - distance + width) % width;
		parallax_x += map_info.parallax_horz ? distance / 2 : 0;
	} else {
		if (map_info.position_x - distance >= 0) {
			map_info.position_x -= distance;
			if (map_info.parallax_horz)
				parallax_x += distance / 2;
			else if (!LoopHorizontal() && GetWidth() > 20 && parallax_width > SCREEN_TARGET_WIDTH)
				parallax_x +=
					std::min(distance, distance * (parallax_width - SCREEN_TARGET_WIDTH) / (GetWidth() - 20) / (SCREEN_TILE_WIDTH / TILE_SIZE));
		}
	}
}

void Game_Map::ScrollRight(int distance) {
	if (LoopHorizontal()) {
		int width = GetWidth() * SCREEN_TILE_WIDTH;
		map_info.position_x = (map_info.position_x + distance + width) % width;
		parallax_x -= map_info.parallax_horz ? distance / 2 : 0;
	} else {
		if (map_info.position_x + distance <= (GetWidth() - 20) * SCREEN_TILE_WIDTH) {
			map_info.position_x += distance;
			if (map_info.parallax_horz)
				parallax_x -= distance / 2;
			else if (!LoopHorizontal() && GetWidth() > 20 && parallax_width > SCREEN_TARGET_WIDTH)
				parallax_x -=
					std::min(distance, distance * (parallax_width - SCREEN_TARGET_WIDTH) / (GetWidth() - 20) / (SCREEN_TILE_WIDTH / TILE_SIZE));
		}
	}
}

void Game_Map::ScrollUp(int distance) {
	if (LoopVertical()) {
		int height = GetHeight() * SCREEN_TILE_WIDTH;
		map_info.position_y = (map_info.position_y - distance + height) % height;
		parallax_y += map_info.parallax_vert ? distance / 2 : 0;
	} else {
		if (map_info.position_y - distance >= 0) {
			map_info.position_y -= distance;
			if (map_info.parallax_vert)
				parallax_y += distance / 2;
			else if (!LoopVertical() && GetHeight() > 15 && parallax_height > SCREEN_TARGET_HEIGHT)
				parallax_y +=
					std::min(distance, distance * (parallax_height - SCREEN_TARGET_HEIGHT) / (GetHeight() - 15) / (SCREEN_TILE_WIDTH / TILE_SIZE));
		}
	}
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
		std::vector<Game_Event*> events;
		std::vector<Game_Event*>::iterator it;

		Game_Map::GetEventsXY(events, x, y);
		for (it = events.begin(); it != events.end(); ++it) {
			if (*it == self_event || (*it)->GetThrough()) {
				continue;
			}
			else if ((*it)->GetLayer() == self_event->GetLayer()) {
				if (self_event->IsInPosition(x, y))
					pass = true;
				else
					return false;
			}
			else if ((*it)->GetLayer() == RPG::EventPage::Layers_below) {
				// Event layer Chipset Tile
				tile_id = (*it)->GetTileId();
				if ((passages_up[tile_id] & Passable::Above) != 0)
					continue;
				if ((passages_up[tile_id] & bit) != 0)
					pass = true;
				else
					return false;
			}
		}

		if (!self_event->IsInPosition(x, y) && (vehicles[0]->IsInPosition(x, y) || vehicles[1]->IsInPosition(x, y)))
			return false;

		if (pass) // All events here are passable
			return true;
	}

	return IsPassableTile(bit, x + y * GetWidth());
}

bool Game_Map::IsPassableVehicle(int x, int y, Game_Vehicle::Type vehicle_type) {
	if (!Game_Map::IsValid(x, y)) return false;

	if (vehicle_type == Game_Vehicle::Boat) {
		if (!Data::data.terrains[GetTerrainTag(x, y) -1].boat_pass)
			return false;
	} else if (vehicle_type == Game_Vehicle::Ship) {
		if (!Data::data.terrains[GetTerrainTag(x, y) -1].ship_pass)
			return false;
	} else if (vehicle_type == Game_Vehicle::Airship) {
		return Data::data.terrains[GetTerrainTag(x, y) -1].airship_pass;
	}

	int tile_id;
	std::vector<Game_Event*> events;
	std::vector<Game_Event*>::iterator it;

	Game_Map::GetEventsXY(events, x, y);
	for (it = events.begin(); it != events.end(); ++it) {
		if ((*it)->GetThrough()) {
			continue;
		} else if ((*it)->GetLayer() == 1) {
			return false;
		} else if ((*it)->GetTileId() > 0 && (*it)->GetLayer() == RPG::EventPage::Layers_below) {
			// Event layer Chipset Tile
			tile_id = (*it)->GetTileId();
			if ((passages_up[tile_id] & Passable::Above) == 0)
				return false;
		}
	}

	int const tile_index = x + y * GetWidth();

	tile_id = map->upper_layer[tile_index] - BLOCK_F;
	tile_id = map_info.upper_tiles[tile_id];

	if ((passages_up[tile_id] & Passable::Above) == 0)
		return false;

	for (int i = 0; i < 3; i++) {
		if (i+1 == vehicle_type)
			continue;
		Game_Vehicle* vehicle = vehicles[i].get();
		if (vehicle->IsInCurrentMap() && vehicle->IsInPosition(x, y) && !vehicle->GetThrough())
			return false;
	}

	return true;
}

bool Game_Map::IsLandable(int x, int y, const Game_Character *self_event) {
	if (!Game_Map::IsValid(x, y)) return false;

	int tile_id;
	int bit = Passable::Down | Passable::Right | Passable::Left | Passable::Up;

	if (self_event) {
		for (tEventHash::iterator i = events.begin(); i != events.end(); ++i) {
			Game_Event* evnt = i->second.get();
			if (evnt != self_event && evnt->IsInPosition(x, y)) {
				if (!evnt->GetThrough()) {
					if (evnt->GetLayer() == RPG::EventPage::Layers_same) {
						return false;
					} else if (evnt->GetTileId() >= 0 && evnt->GetLayer() == RPG::EventPage::Layers_below) {
						// Event layer Chipset Tile
						tile_id = i->second->GetTileId();
						return (passages_up[tile_id] & bit) != 0;
					}
				}
			}
		}
	}

	return IsPassableTile(bit, x + y * GetWidth());
}

bool Game_Map::IsPassableTile(int bit, int tile_index) {
	int tile_id = map->upper_layer[tile_index] - BLOCK_F;
	tile_id = map_info.upper_tiles[tile_id];

	if ((passages_up[tile_id] & bit) == 0)
		return false;

	if ((passages_up[tile_id] & Passable::Above) == 0)
		return true;

	int tile_raw_id = map->lower_layer[tile_index];

	if (tile_raw_id >= BLOCK_E) {
		tile_id = tile_raw_id - BLOCK_E;
		tile_id = map_info.lower_tiles[tile_id] + 18;

	} else if (tile_raw_id >= BLOCK_D) {
		tile_id = (tile_raw_id - BLOCK_D) / 50 + 6;
		int autotile_id = (tile_raw_id - BLOCK_D) % 50;

		if (((passages_down[tile_id] & Passable::Wall) != 0) && (
				(autotile_id >= 20 && autotile_id <= 23) ||
				(autotile_id >= 33 && autotile_id <= 37) ||
				autotile_id == 42 || autotile_id == 43 ||
				autotile_id == 45 || autotile_id == 46))
			return true;

	} else if (tile_raw_id >= BLOCK_C) {
		tile_id = (tile_raw_id - BLOCK_C) / 50 + 3;

	} else if (map->lower_layer[tile_index] < BLOCK_C) {
		tile_id = tile_raw_id / 1000;
	}

	return (passages_down[tile_id] & bit) != 0;
}

int Game_Map::GetBushDepth(int x, int y) {
	if (!Game_Map::IsValid(x, y)) return 0;

	return Data::data.terrains[GetTerrainTag(x,y) - 1].bush_depth;
}

bool Game_Map::IsCounter(int x, int y) {
	if (!Game_Map::IsValid(x, y)) return false;

	int const tile_id = map->upper_layer[x + y * GetWidth()];
	if (tile_id < BLOCK_F) return false;
	int const index = map_info.upper_tiles[tile_id - BLOCK_F];
	return !!(passages_up[index] & Passable::Counter);
}

int Game_Map::GetTerrainTag(int const x, int const y) {
	if (!Game_Map::IsValid(x, y)) return 1;

	unsigned const chipID = map->lower_layer[x + y * GetWidth()];
	unsigned chip_index =
		(chipID <  3050)?  0 + chipID/1000 :
		(chipID <  4000)?  4 + (chipID-3050)/50 :
		(chipID <  5000)?  6 + (chipID-4000)/50 :
		(chipID <  5144)? 18 + (chipID-5000) :
		0;
	unsigned const chipset_index = map_info.chipset_id - 1;

	// Apply tile substitution
	if (chip_index >= 18 && chip_index <= 144)
		chip_index = map_info.lower_tiles[chip_index - 18] + 18;

	assert(chipset_index < Data::data.chipsets.size());
	assert(chip_index < Data::data.chipsets[chipset_index].terrain_data.size());

	return Data::data.chipsets[chipset_index].terrain_data[chip_index];
}

bool Game_Map::AirshipLandOk(int const x, int const y) {
	return Data::data.terrains[GetTerrainTag(x, y) - 1].airship_land;
}

void Game_Map::GetEventsXY(std::vector<Game_Event*>& events, int x, int y) {
	std::vector<Game_Event*> result;

	tEventHash::const_iterator i;
	for (i = Game_Map::GetEvents().begin(); i != Game_Map::GetEvents().end(); ++i) {
		if (i->second->IsInPosition(x, y) && i->second->GetActive()) {
			result.push_back(i->second.get());
		}
	}

	events.swap(result);
}

bool Game_Map::LoopHorizontal() {
	return map->scroll_type == RPG::Map::ScrollType_horizontal || map->scroll_type == RPG::Map::ScrollType_both;
}

bool Game_Map::LoopVertical() {
	return map->scroll_type == RPG::Map::ScrollType_vertical || map->scroll_type == RPG::Map::ScrollType_both;
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
	for (i = events.begin(); i != events.end(); ++i) {
		if (i->second->IsInPosition(x, y)) {
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
	if (GetNeedRefresh()) Refresh();
	UpdateScroll();
	UpdatePan();
	UpdateParallax();
	if (animation) {
		animation->Update();
		if (animation->IsDone()) {
			animation.reset();
		}
	}

	for (tEventHash::iterator i = events.begin();
		i != events.end(); ++i) {
		i->second->Update();
	}

	for (tCommonEventHash::iterator i = common_events.begin();
		i != common_events.end(); ++i) {
		i->second->Update();
	}

	for (int i = 0; i < 3; ++i)
		vehicles[i]->Update();

	free_interpreters.clear();
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

	Game_Battle::SetTerrainId(GetTerrainTag(x, y));
	Game_Temp::battle_troop_id = encounters[rand() / (RAND_MAX / encounters.size() + 1)];
	Game_Temp::battle_escape_mode = -1;

	int current_index = GetMapIndex(location.map_id);
	while (Data::treemap.maps[current_index].background_type == 0 && GetMapIndex(Data::treemap.maps[current_index].parent_map) != current_index) {
		current_index = GetMapIndex(Data::treemap.maps[current_index].parent_map);
	}
	if (Data::treemap.maps[current_index].background_type == 2) {
		Game_Temp::battle_background = Data::treemap.maps[current_index].background_name;
	} else {
		Game_Temp::battle_background = Data::terrains[Game_Battle::GetTerrainId() - 1].background_name;
	}

	Game_Temp::battle_calling = true;

	return true;
}

void Game_Map::ShowBattleAnimation(int animation_id, int target_id, bool global) {
	Main_Data::game_data.screen.battleanim_id = animation_id;
	Main_Data::game_data.screen.battleanim_target = target_id;
	Main_Data::game_data.screen.battleanim_global = global;

	const RPG::Animation& anim = Data::animations[animation_id - 1];
	Game_Character& chara = *Game_Character::GetCharacter(target_id, target_id);
	chara.SetFlashTimeLeft(0); 	// Any flash always ends
	if (global) {
		animation.reset(new BattleAnimationGlobal(anim));
	} else {
		animation.reset(new BattleAnimationChara(anim, chara));
	}
}

bool Game_Map::IsBattleAnimationWaiting() {
	return (bool)animation;
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

bool Game_Map::GetReady() {
	return ready;
}

std::vector<unsigned char>& Game_Map::GetPassagesDown() {
	return passages_down;
}

std::vector<unsigned char>& Game_Map::GetPassagesUp() {
	return passages_up;
}

int Game_Map::GetAnimationType() {
	return animation_type;
}

int Game_Map::GetAnimationSpeed() {
	return (animation_fast ? 12 : 24);
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
}

void Game_Map::SetParallaxSize(int width, int height) {
	parallax_width = width;
	parallax_height = height;
}

void Game_Map::InitializeParallax() {
	if (map_info.parallax_horz)
		parallax_x = -map_info.position_x / 2;
	else if (GetWidth() > 20 && parallax_width > SCREEN_TARGET_WIDTH)
		parallax_x = -std::min(map_info.position_x,
			(map_info.position_x / (SCREEN_TILE_WIDTH / TILE_SIZE)) * (parallax_width - SCREEN_TARGET_WIDTH) / (GetWidth() - 20));
	else
		parallax_x = 0;

	if (map_info.parallax_vert)
		parallax_y = -map_info.position_y / 2;
	else if (GetHeight() > 15 && parallax_height > SCREEN_TARGET_HEIGHT)
		parallax_y = -std::min(map_info.position_y,
			(map_info.position_y / (SCREEN_TILE_WIDTH / TILE_SIZE)) * (parallax_height - SCREEN_TARGET_HEIGHT) / (GetHeight() - 15));
	else
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
	animation_type = chipset.animation_type;
	animation_fast = chipset.animation_speed != 0;
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
	if (which == Game_Vehicle::None) {
		return NULL;
	}
	return vehicles[which - 1].get();
}

bool Game_Map::IsAnyMovePending() {
	std::vector<Game_Character*>::iterator it;
	for (it = pending.begin(); it != pending.end(); ++it)
		if (!(*it)->IsMoveRouteRepeated())
			return true;

	return false;
}

void Game_Map::AddPendingMove(Game_Character* character) {
	pending.push_back(character);
}

void Game_Map::RemovePendingMove(Game_Character* character) {
	pending.erase(std::remove(pending.begin(), pending.end(), character), pending.end());
}

void Game_Map::RemoveAllPendingMoves() {
	std::vector<Game_Character*>::iterator it;
	for (it = pending.begin(); it != pending.end(); ++it)
		(*it)->CancelMoveRoute();

	pending.clear();
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

	int step = (SCREEN_TILE_WIDTH/128) << pan_speed;
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

	if (map_info.parallax_horz && map_info.parallax_horz_auto) {
		parallax_x +=
			map_info.parallax_horz_speed > 0 ? 1 << (map_info.parallax_horz_speed - 1) :
			map_info.parallax_horz_speed < 0 ? -1 << (-map_info.parallax_horz_speed - 1) :
			0;
	}

	if (map_info.parallax_vert && map_info.parallax_vert_auto) {
		parallax_y +=
			map_info.parallax_vert_speed > 0 ? 1 << (map_info.parallax_vert_speed - 1) :
			map_info.parallax_vert_speed < 0 ? -1 << (-map_info.parallax_vert_speed - 1) :
			0;
	}
}

int Game_Map::GetParallaxX() {
	return parallax_x / TILE_SIZE;
}

int Game_Map::GetParallaxY() {
	return parallax_y / TILE_SIZE;
}

const std::string& Game_Map::GetParallaxName() {
	return map_info.parallax_name;
}

FileRequestAsync* Game_Map::RequestMap(int map_id) {
	std::stringstream ss;
	ss << "Map" << std::setfill('0') << std::setw(4) << map_id << ".lmu";

	return AsyncHandler::RequestFile(ss.str());
}
