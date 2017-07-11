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
#include <algorithm>

#include "async_handler.h"
#include "system.h"
#include "battle_animation.h"
#include "game_battle.h"
#include "game_battler.h"
#include "game_map.h"
#include "game_interpreter_map.h"
#include "game_switches.h"
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
#include "utils.h"

namespace {
	RPG::SaveMapInfo& map_info = Main_Data::game_data.map_info;
	RPG::SavePartyLocation& location = Main_Data::game_data.party_location;

	std::string chipset_name;
	std::string battleback_name;
	Game_Map::RefreshMode refresh_type;

	int animation_type;
	bool animation_fast;

	std::vector<unsigned char> passages_down;
	std::vector<unsigned char> passages_up;
	std::vector<Game_Event> events;
	std::vector<Game_CommonEvent> common_events;

	std::unique_ptr<RPG::Map> map;

	std::unique_ptr<Game_Interpreter_Map> interpreter;
	std::vector<std::shared_ptr<Game_Interpreter> > free_interpreters;
	std::vector<std::shared_ptr<Game_Vehicle> > vehicles;
	std::vector<Game_Character*> pending;

	std::unique_ptr<BattleAnimation> animation;

	bool pan_locked;
	bool pan_wait;
	int pan_speed;

	int last_map_id;

	bool teleport_delay;
}

void Game_Map::Init() {
	Dispose();

	map_info.position_x = 0;
	map_info.position_y = 0;
	refresh_type = Refresh_All;

	location.map_id = 0;
	interpreter.reset(new Game_Interpreter_Map(0, true));
	map_info.encounter_rate = 0;

	common_events.clear();
	common_events.reserve(Data::commonevents.size());
	for (const RPG::CommonEvent& ev : Data::commonevents) {
		common_events.emplace_back(ev.ID);
	}

	vehicles.clear();
	for (int i = 0; i < 3; i++)
		vehicles.push_back(std::make_shared<Game_Vehicle>((Game_Vehicle::Type) (i + 1)));

	pan_locked = false;
	pan_wait = false;
	pan_speed = 0;
	location.pan_finish_x = 0;
	location.pan_finish_y = 0;
	location.pan_current_x = 0;
	location.pan_current_y = 0;
	last_map_id = -1;

	teleport_delay = false;
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

	Parallax::ClearChangedBG();

	SetChipset(map->chipset_id);

	events.reserve(map->events.size());
	for (const RPG::Event& ev : map->events) {
		events.emplace_back(location.map_id, ev);
	}

	location.pan_finish_x = 0;
	location.pan_finish_y = 0;
	location.pan_current_x = 0;
	location.pan_current_y = 0;

	// Save allowed
	int current_index = GetMapIndex(location.map_id);
	while (Data::treemap.maps[current_index].save == 0 && GetMapIndex(Data::treemap.maps[current_index].parent_map) != current_index) {
		current_index = GetMapIndex(Data::treemap.maps[current_index].parent_map);
	}
	Game_System::SetAllowSave(Data::treemap.maps[current_index].save == 1);

	// Escape allowed
	current_index = GetMapIndex(location.map_id);
	while (Data::treemap.maps[current_index].escape == 0 && GetMapIndex(Data::treemap.maps[current_index].parent_map) != current_index) {
		current_index = GetMapIndex(Data::treemap.maps[current_index].parent_map);
	}
	Game_System::SetAllowEscape(Data::treemap.maps[current_index].escape == 1);

	// Teleport allowed
	current_index = GetMapIndex(location.map_id);
	while (Data::treemap.maps[current_index].teleport == 0 && GetMapIndex(Data::treemap.maps[current_index].parent_map) != current_index) {
		current_index = GetMapIndex(Data::treemap.maps[current_index].parent_map);
	}
	Game_System::SetAllowTeleport(Data::treemap.maps[current_index].teleport == 1);
}

void Game_Map::SetupFromSave() {
	SetupCommon(location.map_id);

	// Make main interpreter "busy" if save contained events to prevent auto-events from starting
	interpreter->SetupFromSave(Main_Data::game_data.events.commands);

	events.reserve(map->events.size());
	for (size_t i = 0; i < map->events.size(); ++i) {
		if (i < map_info.events.size()) {
			events.emplace_back(location.map_id, map->events[i], map_info.events[i]);
		}
		else {
			events.emplace_back(location.map_id, map->events[i]);
		}

		if (events.back().IsMoveRouteOverwritten())
			pending.push_back(&events.back());
	}

	for (size_t i = 0; i < Main_Data::game_data.common_events.size() && i < common_events.size(); ++i) {
		common_events[i].SetSaveData(Main_Data::game_data.common_events[i].event_data);
	}

	for (size_t i = 0; i < 3; i++)
		if (vehicles[i]->IsMoveRouteOverwritten())
			pending.push_back(vehicles[i].get());

	map_info.Fixup(*map.get());
	SetChipset(map_info.chipset_id);

	// FIXME: Handle Pan correctly
	location.pan_current_x = 0;
	location.pan_current_y = 0;
	location.pan_finish_x = 0;
	location.pan_finish_y = 0;
}

void Game_Map::SetupCommon(int _id) {
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

		map = LMU_Reader::Load(map_file, Player::encoding);
	} else {
		map = LMU_Reader::LoadXml(map_file);
	}
	Output::Debug("Loading Map %s", ss.str().c_str());

	if (map.get() == NULL) {
		Output::ErrorStr(LcfReader::GetError());
	}

	refresh_type = Refresh_All;

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
	Main_Data::game_data.events.commands = interpreter->GetSaveData();

	map_info.events.clear();
	map_info.events.reserve(events.size());
	for (Game_Event& ev : events) {
		map_info.events.push_back(ev.GetSaveData());
	}

	std::vector<RPG::SaveCommonEvent>& save_common_events = Main_Data::game_data.common_events;
	save_common_events.clear();
	save_common_events.reserve(common_events.size());
	for (Game_CommonEvent& ev : common_events) {
		save_common_events.push_back(RPG::SaveCommonEvent());
		save_common_events.back().ID = ev.GetIndex();
		save_common_events.back().event_data = ev.GetSaveData();
	}
}

void Game_Map::PlayBgm() {
	if (last_map_id == location.map_id) {
		// Don't change BGM when the map stayed the same
		// e.g. when returning from menu or teleporting on same map
		return;
	}

	int current_index = GetMapIndex(location.map_id);
	last_map_id = current_index;

	while (Data::treemap.maps[current_index].music_type == 0 && GetMapIndex(Data::treemap.maps[current_index].parent_map) != current_index) {
		current_index = GetMapIndex(Data::treemap.maps[current_index].parent_map);
	}

	if ((current_index > -1) && !Data::treemap.maps[current_index].music.name.empty()) {
		if (Data::treemap.maps[current_index].music_type == 1) {
			return;
		}
		Game_System::BgmPlay(Data::treemap.maps[current_index].music);
	}
}

void Game_Map::Refresh() {
	if (location.map_id > 0) {
		for (Game_Event& ev : events) {
			ev.Refresh();
		}

		if (refresh_type == Refresh_All) {
			for (Game_CommonEvent& ev : common_events) {
				ev.Refresh();
			}
		}
	}

	refresh_type = Refresh_None;
}

Game_Interpreter& Game_Map::GetInterpreter() {
	assert(interpreter);
	return *interpreter;
}

void Game_Map::ReserveInterpreterDeletion(std::shared_ptr<Game_Interpreter> interpreter) {
	free_interpreters.push_back(interpreter);
}

void Game_Map::ScrollRight(int distance) {
	AddScreenX(map_info.position_x, distance);
	Parallax::Scroll(distance, 0);
}

void Game_Map::ScrollDown(int distance) {
	AddScreenY(map_info.position_y, distance);
	Parallax::Scroll(0, distance);
}

// Add inc to acc, clamping the result into the range [low, high].
// If the result is clamped, inc is also modified to be actual amount
// that acc changed by.
static void ClampingAdd(int low, int high, int& acc, int& inc) {
	int original_acc = acc;
	acc = std::max(low, std::min(high, acc + inc));
	inc = acc - original_acc;
}

void Game_Map::AddScreenX(int& screen_x, int& inc) {
	int map_width = GetWidth() * SCREEN_TILE_WIDTH;
	if (LoopHorizontal()) {
		screen_x = Utils::PositiveModulo(screen_x + inc, map_width);
	} else {
		ClampingAdd(0, map_width - SCREEN_WIDTH, screen_x, inc);
	}
}

void Game_Map::AddScreenY(int& screen_y, int& inc) {
	int map_height = GetHeight() * SCREEN_TILE_WIDTH;
	if (LoopVertical()) {
		screen_y = Utils::PositiveModulo(screen_y + inc, map_height);
	} else {
		ClampingAdd(0, map_height - SCREEN_HEIGHT, screen_y, inc);
	}
}

bool Game_Map::IsValid(int x, int y) {
	return (x >= 0 && x < GetWidth() && y >= 0 && y < GetHeight());
}

static int DirToMask(int d) {
	switch (d)
	{
		case RPG::EventPage::Direction_down:
			return Passable::Down;

		case RPG::EventPage::Direction_up:
			return Passable::Up;

		case RPG::EventPage::Direction_left:
			return Passable::Left;

		case RPG::EventPage::Direction_right:
			return Passable::Right;

		default:
			assert(false);
	}
}

enum CollisionResult {
	/** No collision occured. */
	NoCollision,
	/** Collision occured. */
	Collision,
	/**
	 * The other event was a tile event below self that self could
	 * leave by stepping across.
	 */
	CanStepOffCurrentTile,
	/**
	 * The other event was a tile event beneath self's new position
	 * that self could step onto.
	 */
	CanStepOntoNewTile
};

/**
 * Checks whether a collision occurs between self and other if self
 * moves from (x,y) to (new_x, new_y) in direction d.
 *
 * If other is a tile event, also indicates if the player can use it
 * as a "bridge" to step across without hitting the underlying tile
 * layer.
 */
static CollisionResult TestCollisionDuringMove(
	int x,
	int y,
	int new_x,
	int new_y,
	int d,
	const Game_Character& self,
	const Game_Event& other
) {
	if (!other.GetActive()) {
		return NoCollision;
	}

	if (&self == &other) {
		return NoCollision;
	}

	if (other.GetThrough()) {
		return NoCollision;
	}

	if (!other.IsInPosition(x, y) && !other.IsInPosition(new_x, new_y)) {
		return NoCollision;
	}

	if (&self != Main_Data::game_player.get()) {
		if (other.IsInPosition(new_x, new_y) && (self.IsOverlapForbidden() || other.IsOverlapForbidden())) {
			return Collision;
		}
	}

	if (other.IsInPosition(new_x, new_y) && self.GetLayer() == other.GetLayer()) {
		return Collision;
	}

	if (other.GetLayer() == RPG::EventPage::Layers_below) {
		int tile_id = other.GetTileId();
		if ((passages_up[tile_id] & Passable::Above) != 0) {
			return NoCollision;
		}
		if (other.IsInPosition(x,y) && (passages_up[tile_id] & DirToMask(d)) != 0) {
			return CanStepOffCurrentTile;
		}
		else if (other.IsInPosition(new_x, new_y) && (passages_up[tile_id] & DirToMask(Game_Character::ReverseDir(d))) != 0) {
			return CanStepOntoNewTile;
		} else {
			return Collision;
		}
	}

	return NoCollision;
}

bool Game_Map::MakeWay(int x, int y, int d, const Game_Character& self) {
	int new_x = RoundX(x + (d == Game_Character::Right ? 1 : d == Game_Character::Left ? -1 : 0));
	int new_y = RoundY(y + (d == Game_Character::Down ? 1 : d == Game_Character::Up ? -1 : 0));

	if (!Game_Map::IsValid(new_x, new_y))
		return false;

	if (self.GetThrough()) return true;

	// A character can move to a position with an impassable tile by
	// standing on top of an event below it. These flags track whether
	// we stepped off an event and therefore don't need to check the
	// passability of the tile layer below.
	bool stepped_off_event = false;
	bool stepped_onto_event = false;

	for (Game_Event& other : GetEvents()) {
		CollisionResult result = TestCollisionDuringMove(x, y, new_x, new_y, d, self, other);
		if (result == Collision) {
			// Try updating the offending event to give it a chance to move out of the
			// way and recheck.
			other.UpdateParallel();
			if (TestCollisionDuringMove(x, y, new_x, new_y, d, self, other) == Collision) {
				return false;
			}
		}
		else if (result == CanStepOffCurrentTile) {
			stepped_off_event = true;
		} else if (result == CanStepOntoNewTile) {
			stepped_onto_event = true;
		}
	}

	if (!self.IsInPosition(x, y) && (vehicles[0]->IsInPosition(x, y) || vehicles[1]->IsInPosition(x, y)))
		return false;

	if (Main_Data::game_player->IsInPosition(new_x, new_y)
			&& !Main_Data::game_player->GetThrough()
			&& self.GetLayer() == RPG::EventPage::Layers_same) {
		// Update the Player to see if they'll move and recheck.
		Main_Data::game_player->Update();
		if (Main_Data::game_player->IsInPosition(new_x, new_y)) {
			return false;
		}
	}

	return
		(stepped_off_event || IsPassableTile(DirToMask(d), x + y * GetWidth()))
		&& (stepped_onto_event || IsPassableTile(DirToMask(Game_Character::ReverseDir(d)), new_x + new_y * GetWidth()));
}

bool Game_Map::IsPassable(int x, int y, int d, const Game_Character* self_event) {
	// TODO: this and MakeWay share a lot of code.
	if (!Game_Map::IsValid(x, y)) return false;

	int bit = DirToMask(d);

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

			if (self_event != Main_Data::game_player.get()) {
				if (self_event->IsOverlapForbidden() || (*it)->IsOverlapForbidden())
					return false;
			}

			if ((*it)->GetLayer() == self_event->GetLayer()) {
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
		for (Game_Event& ev : events) {
			if (&ev != self_event && ev.IsInPosition(x, y)) {
				if (!ev.GetThrough()) {
					if (ev.GetLayer() == RPG::EventPage::Layers_same) {
						return false;
					} else if (ev.GetTileId() >= 0 && ev.GetLayer() == RPG::EventPage::Layers_below) {
						// Event layer Chipset Tile
						tile_id = ev.GetTileId();
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

int Game_Map::GetTerrainTag(int x, int y) {
	// Terrain tag wraps on looping maps
	x = RoundX(x);
	y = RoundY(y);

	if (!Game_Map::IsValid(x, y)) return 9;

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

	auto& terrain_data = Data::data.chipsets[chipset_index].terrain_data;

	if (terrain_data.empty()) {
		// RPG_RT optimisation: When the terrain is all 1, no terrain data is stored
		return 1;
	}

	assert(chip_index < terrain_data.size());

	return terrain_data[chip_index];
}

bool Game_Map::AirshipLandOk(int const x, int const y) {
	return Data::data.terrains[GetTerrainTag(x, y) - 1].airship_land;
}

void Game_Map::GetEventsXY(std::vector<Game_Event*>& events, int x, int y) {
	for (Game_Event& ev : GetEvents()) {
		if (ev.IsInPosition(x, y) && ev.GetActive()) {
			events.push_back(&ev);
		}
	}
}

bool Game_Map::LoopHorizontal() {
	return map->scroll_type == RPG::Map::ScrollType_horizontal || map->scroll_type == RPG::Map::ScrollType_both;
}

bool Game_Map::LoopVertical() {
	return map->scroll_type == RPG::Map::ScrollType_vertical || map->scroll_type == RPG::Map::ScrollType_both;
}

int Game_Map::RoundX(int x) {
	if (LoopHorizontal()) {
		return Utils::PositiveModulo(x, GetWidth());
	} else {
		return x;
	}
}

int Game_Map::RoundY(int y) {
	if (LoopVertical()) {
		return Utils::PositiveModulo(y, GetHeight());
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
	for (const Game_Event& ev : events) {
		if (ev.IsInPosition(x, y)) {
			return ev.GetId();
		}
	}

	return 0;
}

void Game_Map::Update(bool only_parallel) {
	if (GetNeedRefresh() != Refresh_None) Refresh();
	UpdatePan();
	Parallax::Update();
	if (animation) {
		animation->Update();
		if (animation->IsDone()) {
			animation.reset();
		}
	}

	for (Game_CommonEvent& ev : common_events) {
		ev.UpdateParallel();
	}

	for (Game_Event& ev : events) {
		ev.UpdateParallel();
	}

	if (only_parallel)
		return;

	for (Game_Event& ev : events) {
		ev.CheckEventTriggers();
	}

	Main_Data::game_player->Update();
	GetInterpreter().Update();

	for (Game_Event& ev : events) {
		ev.Update();
	}

	for (Game_CommonEvent& ev : common_events) {
		ev.Update();
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

	if(Main_Data::game_player->InAirship()) {
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
		int throw_one = Utils::GetRandomNumber(0, rate - 1);
		int throw_two = Utils::GetRandomNumber(0, rate - 1);

		// *100 to handle terrain rate better
		location.encounter_steps = (throw_one + throw_two + 1) * 100;
	}
}

std::vector<int> Game_Map::GetEncountersAt(int x, int y) {
	int terrain_tag = GetTerrainTag(Main_Data::game_player->GetX(), Main_Data::game_player->GetY());

	std::function<bool(int)> is_acceptable = [=](int troop_id) {
		std::vector<bool>& terrain_set = Data::troops[troop_id - 1].terrain_set;

		// RPG_RT optimisation: Omitted entries are the default value (true)
		return terrain_set.size() <= (unsigned)(terrain_tag - 1) ||
				terrain_set[terrain_tag - 1];
	};

	std::vector<int> out;

	for (unsigned int i = 0; i < Data::treemap.maps.size(); ++i) {
		RPG::MapInfo& map = Data::treemap.maps[i];

		if (map.ID == location.map_id) {
			for (const RPG::Encounter& enc : map.encounters) {
				if (is_acceptable(enc.troop_id)) {
					out.push_back(enc.troop_id);
				}
			}
		} else if (map.parent_map == location.map_id && map.type == 2) {
			// Area
			Rect area_rect(map.area_rect.l, map.area_rect.t, map.area_rect.r - map.area_rect.l, map.area_rect.b - map.area_rect.t);
			Rect player_rect(x, y, 1, 1);

			if (!player_rect.IsOutOfBounds(area_rect)) {
				for (const RPG::Encounter& enc : map.encounters) {
					if (is_acceptable(enc.troop_id)) {
						out.push_back(enc.troop_id);
					}
				}
			}
		}
	}

	return out;
}

bool Game_Map::PrepareEncounter() {
	if (GetEncounterRate() <= 0) {
		return false;
	}

	int x = Main_Data::game_player->GetX();
	int y = Main_Data::game_player->GetY();

	std::vector<int> encounters = GetEncountersAt(x, y);

	if (encounters.empty()) {
		// No enemies on this map :(
		return false;
	}

	Game_Temp::battle_troop_id = encounters[Utils::GetRandomNumber(0, encounters.size() - 1)];
	Game_Temp::battle_calling = true;

	SetupBattle();

	return true;
}

void Game_Map::SetupBattle() {
	int x = Main_Data::game_player->GetX();
	int y = Main_Data::game_player->GetY();

	Game_Battle::SetTerrainId(GetTerrainTag(x, y));
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
}

void Game_Map::ShowBattleAnimation(int animation_id, int target_id, bool global) {
	Main_Data::game_data.screen.battleanim_id = animation_id;
	Main_Data::game_data.screen.battleanim_target = target_id;
	Main_Data::game_data.screen.battleanim_global = global;

	const RPG::Animation& anim = Data::animations[animation_id - 1];
	Game_Character* chara = Game_Character::GetCharacter(target_id, target_id);

	if (chara) {
		chara->SetFlashTimeLeft(0); 	// Any flash always ends
		if (global) {
			animation.reset(new BattleAnimationGlobal(anim));
		} else {
			animation.reset(new BattleAnimationChara(anim, *chara));
		}
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

int Game_Map::GetChipset() {
	return map_info.chipset_id;
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

int Game_Map::GetPositionX() {
	return map_info.position_x;
}

int Game_Map::GetDisplayX() {
	int shake_in_pixels = Main_Data::game_data.screen.shake_position;
	return map_info.position_x + shake_in_pixels * 16;
}

void Game_Map::SetPositionX(int new_position_x) {
	map_info.position_x = new_position_x;
}

int Game_Map::GetPositionY() {
	return map_info.position_y;
}

int Game_Map::GetDisplayY() {
	return map_info.position_y;
}

void Game_Map::SetPositionY(int new_position_y) {
	map_info.position_y = new_position_y;
}

Game_Map::RefreshMode Game_Map::GetNeedRefresh() {
	return refresh_type;
}
void Game_Map::SetNeedRefresh(Game_Map::RefreshMode refresh_mode) {
	refresh_type = refresh_mode;
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

std::vector<Game_Event>& Game_Map::GetEvents() {
	return events;
}

Game_Event* Game_Map::GetEvent(int event_id) {
	auto it = std::find_if(events.begin(), events.end(),
			[&event_id](Game_Event& ev) {return ev.GetId() == event_id;});
	return it == events.end() ? nullptr : &(*it);
}

std::vector<Game_CommonEvent>& Game_Map::GetCommonEvents() {
	return common_events;
}

int Game_Map::GetMapIndex(int id) {
	for (unsigned int i = 0; i < Data::treemap.maps.size(); ++i) {
		if (Data::treemap.maps[i].ID == id) {
			return i;
		}
	}
	// nothing found
	return -1;
}

std::string Game_Map::GetMapName(int id) {
	for (unsigned int i = 0; i < Data::treemap.maps.size(); ++i) {
		if (Data::treemap.maps[i].ID == id) {
			return Data::treemap.maps[i].name;
		}
	}
	// nothing found
	return "";
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

bool Game_Map::IsAnyEventStarting() {
	for (Game_Event& ev : events)
		if (ev.GetStarting() && !ev.GetList().empty() && ev.GetActive())
			return true;

	for (Game_CommonEvent& ev : common_events)
		if ((ev.GetTrigger() == RPG::EventPage::Trigger_auto_start) &&
			(ev.GetSwitchFlag() ? Game_Switches[ev.GetSwitchId()] : true) &&
			(!ev.GetList().empty()))
				return true;

	return false;
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

int Game_Map::GetTargetPanX() {
	return location.pan_finish_x;
}

int Game_Map::GetTargetPanY() {
	return location.pan_finish_y;
}

bool Game_Map::IsTeleportDelayed() {
	return teleport_delay;
}

void Game_Map::SetTeleportDelayed(bool delay) {
	teleport_delay = delay;
}

FileRequestAsync* Game_Map::RequestMap(int map_id) {
	std::stringstream ss;
	ss << "Map" << std::setfill('0') << std::setw(4) << map_id << ".lmu";

	return AsyncHandler::RequestFile(ss.str());
}

// Parallax
/////////////

namespace {
	int parallax_x;
	int parallax_y;
	int parallax_width;
	int parallax_height;
}

/* Helper function to get the current parallax parameters. If the default
 * parallax for the current map was overridden by a "Change Parallax BG"
 * command, the result is filled out from those values in the SaveMapInfo.
 * Otherwise, the result is filled out from the default for the current map.
 */
static Game_Map::Parallax::Params GetParallaxParams() {
	Game_Map::Parallax::Params params;

	if (!map_info.parallax_name.empty()) {
		params.name = map_info.parallax_name;
		params.scroll_horz = map_info.parallax_horz;
		params.scroll_horz_auto = map_info.parallax_horz_auto;
		params.scroll_horz_speed = map_info.parallax_horz_speed;
		params.scroll_vert = map_info.parallax_vert;
		params.scroll_vert_auto = map_info.parallax_vert_auto;
		params.scroll_vert_speed = map_info.parallax_vert_speed;
	} else if (map->parallax_flag) {
		params.name = map->parallax_name;
		params.scroll_horz = map->parallax_loop_x;
		params.scroll_horz_auto = map->parallax_auto_loop_x;
		params.scroll_horz_speed = map->parallax_sx;
		params.scroll_vert = map->parallax_loop_y;
		params.scroll_vert_auto = map->parallax_auto_loop_y;
		params.scroll_vert_speed = map->parallax_sy;
	} else {
		// No BG; use default-constructed Param
	}

	return params;
}

std::string Game_Map::Parallax::GetName() {
	return GetParallaxParams().name;
}

int Game_Map::Parallax::GetX() {
	return parallax_x / TILE_SIZE;
}

int Game_Map::Parallax::GetY() {
	return parallax_y / TILE_SIZE;
}

void Game_Map::Parallax::Initialize(int width, int height) {
	parallax_width = width;
	parallax_height = height;

	ResetPosition();
}

void Game_Map::Parallax::ResetPosition() {
	Params params = GetParallaxParams();

	if (params.scroll_horz)
		parallax_x = -map_info.position_x / 2;
	else if (GetWidth() > 20 && parallax_width > SCREEN_TARGET_WIDTH)
		parallax_x = -std::min(map_info.position_x,
			(map_info.position_x / (SCREEN_TILE_WIDTH / TILE_SIZE)) * (parallax_width - SCREEN_TARGET_WIDTH) / (GetWidth() - 20));
	else
		parallax_x = 0;

	if (params.scroll_vert)
		parallax_y = -map_info.position_y / 2;
	else if (GetHeight() > 15 && parallax_height > SCREEN_TARGET_HEIGHT)
		parallax_y = -std::min(map_info.position_y,
			(map_info.position_y / (SCREEN_TILE_WIDTH / TILE_SIZE)) * (parallax_height - SCREEN_TARGET_HEIGHT) / (GetHeight() - 15));
	else
		parallax_y = 0;
}

void Game_Map::Parallax::Update() {
	Params params = GetParallaxParams();

	if (params.name.empty())
		return;

	auto scroll_amt = [](int speed) {
		if (speed > 0) return 1 << (speed - 1);
		if (speed < 0) return -(1 << (-speed - 1));
		return 0;
	};

	if (params.scroll_horz && params.scroll_horz_auto) {
		parallax_x += scroll_amt(params.scroll_horz_speed);
	}
	if (params.scroll_vert && params.scroll_vert_auto) {
		parallax_y += scroll_amt(params.scroll_vert_speed);
	}
}

/** Return the argument that is closer to zero. */
static int closer_to_zero(int x, int y) {
	return (std::abs(x) < std::abs(y)) ? x : y;
}

void Game_Map::Parallax::Scroll(int distance_right, int distance_down) {
	Params params = GetParallaxParams();

	// TODO: understand and then doc this function :)

	if (params.scroll_vert) {
		parallax_y -= distance_down / 2;
	} else if (
		!LoopVertical() &&
		GetHeight() > 15 && parallax_height > SCREEN_TARGET_HEIGHT
	) {
		parallax_y -=
			closer_to_zero(
				distance_down,
				distance_down * (parallax_height - SCREEN_TARGET_HEIGHT) / (GetHeight() - 15) / (SCREEN_TILE_WIDTH / TILE_SIZE)
			);
	}

	if (params.scroll_horz) {
		parallax_x -= distance_right / 2;
	} else if (
		!LoopHorizontal() &&
		GetWidth() > 20 && parallax_width > SCREEN_TARGET_WIDTH
	) {
		parallax_x -=
			closer_to_zero(
				distance_right,
				distance_right * (parallax_width - SCREEN_TARGET_WIDTH) / (GetWidth() - 20) / (SCREEN_TILE_WIDTH / TILE_SIZE)
			);
	}
}

void Game_Map::Parallax::ChangeBG(const Params& params) {
	map_info.parallax_name = params.name;
	map_info.parallax_horz = params.scroll_horz;
	map_info.parallax_horz_auto = params.scroll_horz_auto;
	map_info.parallax_horz_speed = params.scroll_horz_speed;
	map_info.parallax_vert = params.scroll_vert;
	map_info.parallax_vert_auto = params.scroll_vert_auto;
	map_info.parallax_vert_speed = params.scroll_vert_speed;
}

void Game_Map::Parallax::ClearChangedBG() {
	Params params; // default Param indicates no override
	ChangeBG(params);
}
