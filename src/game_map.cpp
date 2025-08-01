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
#include <initializer_list>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <climits>
#include <numeric>
#include <unordered_set>

#include "async_handler.h"
#include "options.h"
#include "system.h"
#include "game_battle.h"
#include "game_battler.h"
#include "game_map.h"
#include "game_interpreter_map.h"
#include "game_switches.h"
#include "game_player.h"
#include "game_party.h"
#include "game_message.h"
#include "game_screen.h"
#include "game_pictures.h"
#include "game_variables.h"
#include "scene_battle.h"
#include "scene_map.h"
#include <lcf/lmu/reader.h>
#include <lcf/reader_lcf.h>
#include "map_data.h"
#include "main_data.h"
#include "output.h"
#include "util_macro.h"
#include "game_system.h"
#include "filefinder.h"
#include "player.h"
#include "input.h"
#include "utils.h"
#include "rand.h"
#include <lcf/scope_guard.h>
#include <lcf/rpg/save.h>
#include "scene_gameover.h"
#include "feature.h"

namespace {
	// Intended bad value, Game_Map::Init sets them correctly
	int screen_width = -1;
	int screen_height = -1;

	lcf::rpg::SaveMapInfo map_info;
	lcf::rpg::SavePanorama panorama;

	bool need_refresh;

	int animation_type;
	bool animation_fast;
	std::vector<unsigned char> passages_down;
	std::vector<unsigned char> passages_up;
	std::vector<Game_Event> events;
	std::vector<Game_CommonEvent> common_events;
	std::unique_ptr<Game_Map::Caching::MapCache> map_cache;

	std::unique_ptr<lcf::rpg::Map> map;

	std::unique_ptr<Game_Interpreter_Map> interpreter;
	std::vector<Game_Vehicle> vehicles;

	lcf::rpg::Chipset* chipset;

	//FIXME: Find a better way to do this.
	bool panorama_on_map_init = true;
	bool reset_panorama_x_on_next_init = true;
	bool reset_panorama_y_on_next_init = true;

	bool translation_changed = false;

	// Used when the current map is not in the maptree
	const lcf::rpg::MapInfo empty_map_info;
}

namespace Game_Map {
void SetupCommon();
}

void Game_Map::OnContinueFromBattle() {
	Main_Data::game_system->BgmPlay(Main_Data::game_system->GetBeforeBattleMusic());
}

static Game_Map::Parallax::Params GetParallaxParams();

void Game_Map::Init() {
	Dispose();

	map_info = {};
	panorama = {};
	SetNeedRefresh(true);

	interpreter.reset(new Game_Interpreter_Map(true));
	map_cache.reset(new Caching::MapCache());

	InitCommonEvents();

	vehicles.clear();
	vehicles.emplace_back(Game_Vehicle::Boat);
	vehicles.emplace_back(Game_Vehicle::Ship);
	vehicles.emplace_back(Game_Vehicle::Airship);
}

void Game_Map::InitCommonEvents() {
	common_events.clear();
	common_events.reserve(lcf::Data::commonevents.size());
	for (const lcf::rpg::CommonEvent& ev : lcf::Data::commonevents) {
		common_events.emplace_back(ev.ID);
	}
	translation_changed = false;
}

void Game_Map::Dispose() {
	events.clear();
	map.reset();
	map_info = {};
	panorama = {};
}

void Game_Map::Quit() {
	Dispose();
	common_events.clear();
	interpreter.reset();
	map_cache.reset();
}

int Game_Map::GetMapSaveCount() {
	return (Player::IsRPG2k3() && map->save_count_2k3e > 0)
		? map->save_count_2k3e
		: map->save_count;
}

void Game_Map::Setup(std::unique_ptr<lcf::rpg::Map> map_in) {
	Dispose();

	map = std::move(map_in);

	SetupCommon();

	panorama_on_map_init = true;
	Parallax::ClearChangedBG();

	SetEncounterSteps(GetMapInfo().encounter_steps);
	SetChipset(map->chipset_id);

	std::iota(map_info.lower_tiles.begin(), map_info.lower_tiles.end(), 0);
	std::iota(map_info.upper_tiles.begin(), map_info.upper_tiles.end(), 0);

	// Save allowed
	const auto* current_info = &GetMapInfo();
	int current_index = current_info->ID;
	int can_save = current_info->save;
	int can_escape = current_info->escape;
	int can_teleport = current_info->teleport;

	while (can_save == lcf::rpg::MapInfo::TriState_parent
			|| can_escape == lcf::rpg::MapInfo::TriState_parent
			|| can_teleport == lcf::rpg::MapInfo::TriState_parent)
	{
		const auto* parent_info = &GetParentMapInfo(*current_info);
		int parent_index = parent_info->ID;
		if (parent_index == 0) {
			// If parent is 0 and flag is parent, it's implicitly enabled.
			break;
		}
		if (parent_index == current_index) {
			Output::Warning("Map {} has parent pointing to itself!", current_index);
			break;
		}
		current_info = parent_info;
		if (can_save == lcf::rpg::MapInfo::TriState_parent) {
			can_save = current_info->save;
		}
		if (can_escape == lcf::rpg::MapInfo::TriState_parent) {
			can_escape = current_info->escape;
		}
		if (can_teleport == lcf::rpg::MapInfo::TriState_parent) {
			can_teleport = current_info->teleport;
		}
	}
	Main_Data::game_system->SetAllowSave(can_save != lcf::rpg::MapInfo::TriState_forbid);
	Main_Data::game_system->SetAllowEscape(can_escape != lcf::rpg::MapInfo::TriState_forbid);
	Main_Data::game_system->SetAllowTeleport(can_teleport != lcf::rpg::MapInfo::TriState_forbid);

	auto& player = *Main_Data::game_player;

	SetPositionX(player.GetX() * SCREEN_TILE_SIZE - player.GetPanX());
	SetPositionY(player.GetY() * SCREEN_TILE_SIZE - player.GetPanY());

	// Update the save counts so that if the player saves the game
	// events will properly resume upon loading.
	Main_Data::game_player->UpdateSaveCounts(lcf::Data::system.save_count, GetMapSaveCount());
}

void Game_Map::SetupFromSave(
		std::unique_ptr<lcf::rpg::Map> map_in,
		lcf::rpg::SaveMapInfo save_map,
		lcf::rpg::SaveVehicleLocation save_boat,
		lcf::rpg::SaveVehicleLocation save_ship,
		lcf::rpg::SaveVehicleLocation save_airship,
		lcf::rpg::SaveEventExecState save_fg_exec,
		lcf::rpg::SavePanorama save_pan,
		std::vector<lcf::rpg::SaveCommonEvent> save_ce) {

	map = std::move(map_in);
	map_info = std::move(save_map);
	panorama = std::move(save_pan);

	SetupCommon();

	const bool is_db_save_compat = Main_Data::game_player->IsDatabaseCompatibleWithSave(lcf::Data::system.save_count);
	const bool is_map_save_compat = Main_Data::game_player->IsMapCompatibleWithSave(GetMapSaveCount());

	InitCommonEvents();

	if (is_db_save_compat && is_map_save_compat) {
		for (size_t i = 0; i < std::min(save_ce.size(), common_events.size()); ++i) {
			common_events[i].SetSaveData(save_ce[i].parallel_event_execstate);
		}
	}

	if (is_map_save_compat) {
		std::vector<int> destroyed_event_ids;

		for (size_t i = 0, j = 0; i < events.size() && j < map_info.events.size(); ++i) {
			auto& ev = events[i];
			auto& save_ev = map_info.events[j];
			if (ev.GetId() == save_ev.ID) {
				ev.SetSaveData(save_ev);
				++j;
			} else {
				if (save_ev.ID > ev.GetId()) {
					// assume that the event has been destroyed during gameplay via "DestroyMapEvent"
					destroyed_event_ids.emplace_back(ev.GetId());
				} else {
					Output::Debug("SetupFromSave: Unexpected ID {}/{}", save_ev.ID, ev.GetId());
				}
			}
		}
		for (size_t i = 0; i < destroyed_event_ids.size(); ++i) {
			DestroyMapEvent(destroyed_event_ids[i], true);
		}
		if (destroyed_event_ids.size() > 0) {
			UpdateUnderlyingEventReferences();
		}
	}

	// Handle cloned events in a separate loop, regardless of "is_map_save_compat"
	if (Player::HasEasyRpgExtensions()) {
		for (size_t i = 0; i < map_info.events.size(); ++i) {
			auto& save_ev = map_info.events[i];
			bool is_cloned_evt = save_ev.easyrpg_clone_map_id > 0 || save_ev.easyrpg_clone_event_id > 0;
			if (is_cloned_evt && CloneMapEvent(
				save_ev.easyrpg_clone_map_id, save_ev.easyrpg_clone_event_id,
				save_ev.position_x, save_ev.position_y,
				save_ev.ID, "")) { // FIXME: Customized event names for saved events aren't part of liblcf/SaveMapEvent at the moment & thus cannot be restored
				if (auto new_event = GetEvent(save_ev.ID); new_event != nullptr) {
					new_event->SetSaveData(save_ev);
				}
			}
		}
		UpdateUnderlyingEventReferences();
	}
	map_info.events.clear();
	interpreter->Clear();

	GetVehicle(Game_Vehicle::Boat)->SetSaveData(std::move(save_boat));
	GetVehicle(Game_Vehicle::Ship)->SetSaveData(std::move(save_ship));
	GetVehicle(Game_Vehicle::Airship)->SetSaveData(std::move(save_airship));

	if (is_map_save_compat) {
		// Make main interpreter "busy" if save contained events to prevent auto-events from starting
		interpreter->SetState(std::move(save_fg_exec));
	}

	SetEncounterSteps(map_info.encounter_steps);

	// RPG_RT bug: Chipset is not loaded. Fixed in 2k3E
	if (Player::IsRPG2k3E()) {
		SetChipset(map_info.chipset_id);
	} else {
		SetChipset(0);
	}

	if (!is_map_save_compat) {
		panorama = {};
	}

	// We want to support loading rm2k3e panning chunks
	// but also not break other saves which don't have them.
	// To solve this problem, we reuse the scrolling methods
	// which always reset the position anyways when scroll_horz/vert
	// is false.
	// This produces compatible behavior for old RPG_RT saves, namely
	// the pan_x/y is always forced to 0.
	// If the later async code will load panorama, set the flag to not clear the offsets.
	// FIXME: RPG_RT compatibility bug: Everytime we load a savegame with default panorama chunks,
	// this causes them to get overwritten
	// FIXME: RPG_RT compatibility bug: On async platforms, panorama async loading can
	// cause panorama chunks to be out of sync.
	Game_Map::Parallax::ChangeBG(GetParallaxParams());
}

std::unique_ptr<lcf::rpg::Map> Game_Map::LoadMapFile(int map_id) {
	std::unique_ptr<lcf::rpg::Map> map;

	// Try loading EasyRPG map files first, then fallback to normal RPG Maker
	// FIXME: Assert map was cached for async platforms
	std::string map_name = Game_Map::ConstructMapName(map_id, true);
	std::string map_file = FileFinder::Game().FindFile(map_name);
	if (map_file.empty()) {
		map_name = Game_Map::ConstructMapName(map_id, false);
		map_file = FileFinder::Game().FindFile(map_name);

		if (map_file.empty()) {
			Output::Error("Loading of Map {} failed.\nThe map was not found.", map_name);
			return nullptr;
		}

		auto map_stream = FileFinder::Game().OpenInputStream(map_file);
		if (!map_stream) {
			Output::Error("Loading of Map {} failed.\nMap not readable.", map_name);
			return nullptr;
		}

		map = lcf::LMU_Reader::Load(map_stream, Player::encoding);

		if (Input::IsRecording()) {
			map_stream.clear();
			map_stream.seekg(0);
			Input::AddRecordingData(Input::RecordingData::Hash,
						   fmt::format("map{:04} {:#08x}", map_id, Utils::CRC32(map_stream)));
		}
	} else {
		auto map_stream = FileFinder::Game().OpenInputStream(map_file);
		if (!map_stream) {
			Output::Error("Loading of Map {} failed.\nMap not readable.", map_name);
			return nullptr;
		}
		map = lcf::LMU_Reader::LoadXml(map_stream);
	}

	Output::Debug("Loaded Map {}", map_name);

	if (map.get() == NULL) {
		Output::ErrorStr(lcf::LcfReader::GetError());
	}

	return map;
}

void Game_Map::SetupCommon() {
	screen_width = (Player::screen_width / 16.0) * SCREEN_TILE_SIZE;
	screen_height = (Player::screen_height / 16.0) * SCREEN_TILE_SIZE;

	if (!Tr::GetCurrentTranslationId().empty()) {
		TranslateMapMessages(GetMapId(), *map);
	}
	SetNeedRefresh(true);

	PrintPathToMap();

	if (translation_changed) {
		InitCommonEvents();
	}

	map_cache->Clear();

	CreateMapEvents();
}

void Game_Map::CreateMapEvents() {
	events.reserve(map->events.size());
	for (auto& ev : map->events) {
		events.emplace_back(GetMapId(), &ev);
		AddEventToCache(ev);
	}
}

void Game_Map::AddEventToCache(const lcf::rpg::Event& ev) {
	using Op = Caching::ObservedVarOps;

	for (const auto& pg : ev.pages) {
		if (pg.condition.flags.switch_a) {
			map_cache->AddEventAsRefreshTarget<Op::SwitchSet>(pg.condition.switch_a_id, ev);
		}
		if (pg.condition.flags.switch_b) {
			map_cache->AddEventAsRefreshTarget<Op::SwitchSet>(pg.condition.switch_b_id, ev);
		}
		if (pg.condition.flags.variable) {
			map_cache->AddEventAsRefreshTarget<Op::VarSet>(pg.condition.variable_id, ev);
		}
	}
}

void Game_Map::RemoveEventFromCache(const lcf::rpg::Event& ev) {
	using Op = Caching::ObservedVarOps;

	for (const auto& pg : ev.pages) {
		if (pg.condition.flags.switch_a) {
			map_cache->RemoveEventAsRefreshTarget<Op::SwitchSet>(pg.condition.switch_a_id, ev);
		}
		if (pg.condition.flags.switch_b) {
			map_cache->RemoveEventAsRefreshTarget<Op::SwitchSet>(pg.condition.switch_b_id, ev);
		}
		if (pg.condition.flags.variable) {
			map_cache->RemoveEventAsRefreshTarget<Op::VarSet>(pg.condition.variable_id, ev);
		}
	}
}

void Game_Map::Caching::MapCache::Clear() {
	for (int i = 0; i < static_cast<int>(ObservedVarOps_END); i++) {
		refresh_targets_by_varid[i].clear();
	}
}

bool Game_Map::CloneMapEvent(int src_map_id, int src_event_id, int target_x, int target_y, int target_event_id, std::string_view target_name) {
	std::unique_ptr<lcf::rpg::Map> source_map_storage;
	const lcf::rpg::Map* source_map;

	if (src_map_id == GetMapId()) {
		source_map = &GetMap();
	} else {
		source_map_storage = Game_Map::LoadMapFile(src_map_id);
		source_map = source_map_storage.get();

		if (source_map_storage == nullptr) {
			Output::Warning("CloneMapEvent: Invalid source map ID {}", src_map_id);
			return false;
		}

		if (!Tr::GetCurrentTranslationId().empty()) {
			TranslateMapMessages(src_map_id, *source_map_storage);
		}
	}

	const lcf::rpg::Event* source_event = FindEventById(source_map->events, src_event_id);
	if (source_event == nullptr) {
		Output::Warning("CloneMapEvent: Event ID {} not found on source map {}", src_event_id, src_map_id);
		return false;
	}

	lcf::rpg::Event new_event = *source_event;
	if (target_event_id > 0) {
		DestroyMapEvent(target_event_id, true);
		new_event.ID = target_event_id;
	} else {
		new_event.ID = GetNextAvailableEventId();
	}
	new_event.x = target_x;
	new_event.y = target_y;

	if (!target_name.empty()) {
		new_event.name = lcf::DBString(target_name);
	}

	// sorted insert
	auto insert_it = map->events.insert(
		std::upper_bound(map->events.begin(), map->events.end(), new_event, [](const auto& e, const auto& e2) {
			return e.ID < e2.ID;
		}), new_event);

	auto game_event = Game_Event(GetMapId(), &*insert_it);
	game_event.data()->easyrpg_clone_event_id = src_event_id;
	game_event.data()->easyrpg_clone_map_id = src_map_id;

	events.insert(
		std::upper_bound(events.begin(), events.end(), game_event, [](const auto& e, const auto& e2) {
			return e.GetId() < e2.GetId();
		}), std::move(game_event));

	UpdateUnderlyingEventReferences();

	AddEventToCache(new_event);

	Scene_Map* scene = (Scene_Map*)Scene::Find(Scene::Map).get();
	if (scene) {
		scene->spriteset->Refresh();
		SetNeedRefresh(true);
	}

	return true;
}

bool Game_Map::DestroyMapEvent(const int event_id, bool from_clone) {
	const lcf::rpg::Event* event = FindEventById(map->events, event_id);

	if (event == nullptr) {
		if (!from_clone) {
			Output::Warning("DestroyMapEvent: Event ID {} not found on current map", event_id);
		}
		return true;
	}

	// Remove event from cache
	RemoveEventFromCache(*event);

	// Remove event from events vector
	for (auto it = events.begin(); it != events.end(); ++it) {
		if (it->GetId() == event_id) {
			events.erase(it);
			break;
		}
	}

	// Remove event from map
	for (auto it = map->events.begin(); it != map->events.end(); ++it) {
		if (it->ID == event_id) {
			map->events.erase(it);
			break;
		}
	}

	if (!from_clone) {
		UpdateUnderlyingEventReferences();

		Scene_Map* scene = (Scene_Map*)Scene::Find(Scene::Map).get();
		scene->spriteset->Refresh();
		SetNeedRefresh(true);
	}

	if (GetInterpreter().GetOriginalEventId() == event_id) {
		// Prevent triggering "invalid event on stack" sanity check
		GetInterpreter().ClearOriginalEventId();
	}

	return true;
}

void Game_Map::TranslateMapMessages(int mapId, lcf::rpg::Map& map) {
	std::stringstream ss;
	ss << "map" << std::setfill('0') << std::setw(4) << mapId << ".po";
	Player::translation.RewriteMapMessages(ss.str(), map);
}


void Game_Map::UpdateUnderlyingEventReferences() {
	// Update references because modifying the vector can reallocate
	size_t idx = 0;
	for (auto& ev : events) {
		ev.SetUnderlyingEvent(&map->events.at(idx++));
	}

	Main_Data::game_screen->UpdateUnderlyingEventReferences();
}

const lcf::rpg::Event* Game_Map::FindEventById(const std::vector<lcf::rpg::Event>& events, int eventId) {
	for (const auto& ev : events) {
		if (ev.ID == eventId) {
			return &ev;
		}
	}
	return nullptr;
}

int Game_Map::GetNextAvailableEventId() {
	return map->events.back().ID + 1;
}

void Game_Map::PrepareSave(lcf::rpg::Save& save) {
	save.foreground_event_execstate = interpreter->GetSaveState();

	save.airship_location = GetVehicle(Game_Vehicle::Airship)->GetSaveData();
	save.ship_location = GetVehicle(Game_Vehicle::Ship)->GetSaveData();
	save.boat_location = GetVehicle(Game_Vehicle::Boat)->GetSaveData();

	save.map_info = map_info;
	save.map_info.chipset_id = GetChipset();
	if (save.map_info.chipset_id == GetOriginalChipset()) {
		// This emulates RPG_RT behavior, where chipset id == 0 means use the default map chipset.
		save.map_info.chipset_id = 0;
	}
	if (save.map_info.encounter_steps == GetOriginalEncounterSteps()) {
		save.map_info.encounter_steps = -1;
	}
	// Note: RPG_RT does not use a sentinel for parallax parameters. Once the parallax BG is changed, it stays that way forever.

	save.map_info.events.clear();
	save.map_info.events.reserve(events.size());
	for (Game_Event& ev : events) {
		save.map_info.events.push_back(ev.GetSaveData());
	}

	save.panorama = panorama;

	save.common_events.clear();
	save.common_events.reserve(common_events.size());
	for (Game_CommonEvent& ev : common_events) {
		save.common_events.push_back(lcf::rpg::SaveCommonEvent());
		save.common_events.back().ID = ev.GetIndex();
		save.common_events.back().parallel_event_execstate = ev.GetSaveData();
	}
}

void Game_Map::PlayBgm() {
	const auto* current_info = &GetMapInfo();
	while (current_info->music_type == 0 && GetParentMapInfo(*current_info).ID != current_info->ID) {
		current_info = &GetParentMapInfo(*current_info);
	}

	if ((current_info->ID > 0) && !current_info->music.name.empty()) {
		if (current_info->music_type == 1) {
			return;
		}
		auto& music = current_info->music;
		if (!Main_Data::game_player->IsAboard()) {
			Main_Data::game_system->BgmPlay(music);
		} else {
			Main_Data::game_system->SetBeforeVehicleMusic(music);
		}
	}
}

std::vector<uint8_t> Game_Map::GetTilesLayer(int layer) {
	return layer >= 1 ? map_info.upper_tiles : map_info.lower_tiles;
}

void Game_Map::Refresh() {
	if (GetMapId() > 0) {
		for (Game_Event& ev : events) {
			ev.RefreshPage();
		}
	}

	need_refresh = false;
}

Game_Interpreter_Map& Game_Map::GetInterpreter() {
	assert(interpreter);
	return *interpreter;
}

void Game_Map::Scroll(int dx, int dy) {
	int x = map_info.position_x;
	AddScreenX(x, dx);
	map_info.position_x = x;

	int y = map_info.position_y;
	AddScreenY(y, dy);
	map_info.position_y = y;

	if (dx == 0 && dy == 0) {
		return;
	}

	Main_Data::game_screen->OnMapScrolled(dx, dy);
	Main_Data::game_pictures->OnMapScrolled(dx, dy);
	Game_Map::Parallax::ScrollRight(dx);
	Game_Map::Parallax::ScrollDown(dy);
}

// Add inc to acc, clamping the result into the range [low, high].
// If the result is clamped, inc is also modified to be actual amount
// that acc changed by.
static void ClampingAdd(int low, int high, int& acc, int& inc) {
	int original_acc = acc;
	// Do not use std::clamp here. When the map is smaller than the screen the
	// upper bound is smaller than the lower bound making the function fail.
	acc = std::max(low, std::min(high, acc + inc));
	inc = acc - original_acc;
}

void Game_Map::AddScreenX(int& screen_x, int& inc) {
	int map_width = GetTilesX() * SCREEN_TILE_SIZE;
	if (LoopHorizontal()) {
		screen_x = (screen_x + inc) % map_width;
	} else {
		ClampingAdd(0, map_width - screen_width, screen_x, inc);
	}
}

void Game_Map::AddScreenY(int& screen_y, int& inc) {
	int map_height = GetTilesY() * SCREEN_TILE_SIZE;
	if (LoopVertical()) {
		screen_y = (screen_y + inc) % map_height;
	} else {
		ClampingAdd(0, map_height - screen_height, screen_y, inc);
	}
}

bool Game_Map::IsValid(int x, int y) {
	return (x >= 0 && x < GetTilesX() && y >= 0 && y < GetTilesY());
}

static int GetPassableMask(int old_x, int old_y, int new_x, int new_y) {
	int bit = 0;
	if (new_x > old_x) { bit |= Passable::Right; }
	if (new_x < old_x) { bit |= Passable::Left; }
	if (new_y > old_y) { bit |= Passable::Down; }
	if (new_y < old_y) { bit |= Passable::Up; }
	return bit;
}

static bool WouldCollide(const Game_Character& self, const Game_Character& other, bool self_conflict) {
	if (self.GetThrough() || other.GetThrough()) {
		return false;
	}

	if (self.IsFlying() || other.IsFlying()) {
		return false;
	}

	if (!self.IsActive() || !other.IsActive()) {
		return false;
	}

	if (self.GetType() == Game_Character::Event
			&& other.GetType() == Game_Character::Event
			&& (self.IsOverlapForbidden() || other.IsOverlapForbidden())) {
		return true;
	}

	if (other.GetLayer() == lcf::rpg::EventPage::Layers_same && self_conflict) {
		return true;
	}

	if (self.GetLayer() == other.GetLayer()) {
		return true;
	}

	return false;
}

template <typename T>
static void MakeWayUpdate(T& other) {
	other.Update();
}

static void MakeWayUpdate(Game_Event& other) {
	other.Update(false);
}

template <typename T>
static bool CheckWayTestCollideEvent(int x, int y, const Game_Character& self, T& other, bool self_conflict) {
	if (&self == &other) {
		return false;
	}

	if (!other.IsInPosition(x, y)) {
		return false;
	}

	return WouldCollide(self, other, self_conflict);
}

template <typename T>
static bool MakeWayCollideEvent(int x, int y, const Game_Character& self, T& other, bool self_conflict) {
	if (&self == &other) {
		return false;
	}

	if (!other.IsInPosition(x, y)) {
		return false;
	}

	// Force the other event to update, allowing them to possibly move out of the way.
	MakeWayUpdate(other);

	if (!other.IsInPosition(x, y)) {
		return false;
	}

	return WouldCollide(self, other, self_conflict);
}

static Game_Vehicle::Type GetCollisionVehicleType(const Game_Character* ch) {
	if (ch && ch->GetType() == Game_Character::Vehicle) {
		return static_cast<Game_Vehicle::Type>(static_cast<const Game_Vehicle*>(ch)->GetVehicleType());
	}
	return Game_Vehicle::None;
}

bool Game_Map::CheckWay(const Game_Character& self,
		int from_x, int from_y,
		int to_x, int to_y
		)
{
	return CheckOrMakeWayEx(
		self, from_x, from_y, to_x, to_y, true, {}, false
	);
}

bool Game_Map::CheckWay(const Game_Character& self,
		int from_x, int from_y,
		int to_x, int to_y,
		bool check_events_and_vehicles,
		Span<int> ignore_some_events_by_id) {
	return CheckOrMakeWayEx(
		self, from_x, from_y, to_x, to_y,
		check_events_and_vehicles,
		ignore_some_events_by_id, false
	);
}

bool Game_Map::CheckOrMakeWayEx(const Game_Character& self,
		int from_x, int from_y,
		int to_x, int to_y,
		bool check_events_and_vehicles,
		Span<int> ignore_some_events_by_id,
		bool make_way
		)
{
	// Infer directions before we do any rounding.
	const int bit_from = GetPassableMask(from_x, from_y, to_x, to_y);
	const int bit_to = GetPassableMask(to_x, to_y, from_x, from_y);

	// Now round for looping maps.
	to_x = Game_Map::RoundX(to_x);
	to_y = Game_Map::RoundY(to_y);

	// Note, even for diagonal, if the tile is invalid we still check vertical/horizontal first!
	if (!Game_Map::IsValid(to_x, to_y)) {
		return false;
	}

	if (self.GetThrough()) {
		return true;
	}

	const auto vehicle_type = GetCollisionVehicleType(&self);
	bool self_conflict = false;

	// Depending on whether we're supposed to call MakeWayCollideEvent
	// (which might change the map) or not, choose what to call:
	auto CheckOrMakeCollideEvent = [&](auto& other) {
		if (make_way) {
			return MakeWayCollideEvent(to_x, to_y, self, other, self_conflict);
		} else {
			return CheckWayTestCollideEvent(
				to_x, to_y, self, other, self_conflict
			);
		}
	};

	if (!self.IsJumping()) {
		// Check for self conflict.
		// If this event has a tile graphic and the tile itself has passage blocked in the direction
		// we want to move, flag it as "self conflicting" for use later.
		if (self.GetLayer() == lcf::rpg::EventPage::Layers_below && self.GetTileId() != 0) {
			int tile_id = self.GetTileId();
			if ((passages_up[tile_id] & bit_from) == 0) {
				self_conflict = true;
			}
		}

		if (vehicle_type == Game_Vehicle::None) {
			// Check that we are allowed to step off of the current tile.
			// Note: Vehicles can always step off a tile.

			// The current coordinate can be invalid due to an out-of-bounds teleport or a "Set Location" event.
			// Round it for looping maps to ensure the check passes
			// This is not fully bug compatible to RPG_RT. Assuming the Y-Coordinate is out-of-bounds: When moving
			// left or right the invalid Y will stay in RPG_RT preventing events from being triggered, but we wrap it
			// inbounds after the first move.
			from_x = Game_Map::RoundX(from_x);
			from_y = Game_Map::RoundY(from_y);
			if (!IsPassableTile(&self, bit_from, from_x, from_y)) {
				return false;
			}
		}
	}
	if (vehicle_type != Game_Vehicle::Airship && check_events_and_vehicles) {
		// Check for collision with events on the target tile.
		if (ignore_some_events_by_id.empty()) {
			for (auto& other: GetEvents()) {
				if (CheckOrMakeCollideEvent(other)) {
					return false;
				}
			}
		} else {
			for (auto& other: GetEvents()) {
				if (std::find(ignore_some_events_by_id.begin(), ignore_some_events_by_id.end(), other.GetId()) != ignore_some_events_by_id.end())
					continue;
				if (CheckOrMakeCollideEvent(other)) {
					return false;
				}
			}
		}

		auto& player = Main_Data::game_player;
		if (player->GetVehicleType() == Game_Vehicle::None) {
			if (CheckOrMakeCollideEvent(*Main_Data::game_player)) {
				return false;
			}
		}
		for (auto vid: { Game_Vehicle::Boat, Game_Vehicle::Ship}) {
			auto& other = vehicles[vid - 1];
			if (other.IsInCurrentMap()) {
				if (CheckOrMakeCollideEvent(other)) {
					return false;
				}
			}
		}
		auto& airship = vehicles[Game_Vehicle::Airship - 1];
		if (airship.IsInCurrentMap() && self.GetType() != Game_Character::Player) {
			if (CheckOrMakeCollideEvent(airship)) {
				return false;
			}
		}
	}
	int bit = bit_to;
	if (self.IsJumping()) {
		bit = Passable::Down | Passable::Up | Passable::Left | Passable::Right;
	}

	return IsPassableTile(
		&self, bit, to_x, to_y, check_events_and_vehicles, true
		);
}

bool Game_Map::MakeWay(const Game_Character& self,
		int from_x, int from_y,
		int to_x, int to_y
		)
{
	return CheckOrMakeWayEx(
		self, from_x, from_y, to_x, to_y, true, {}, true
		);
}


bool Game_Map::CanLandAirship(int x, int y) {
	if (!Game_Map::IsValid(x, y)) return false;

	const auto* terrain = lcf::ReaderUtil::GetElement(lcf::Data::terrains, GetTerrainTag(x, y));
	if (!terrain) {
		Output::Warning("CanLandAirship: Invalid terrain at ({}, {})", x, y);
		return false;
	}
	if (!terrain->airship_land) {
		return false;
	}

	for (auto& ev: events) {
		if (ev.IsInPosition(x, y)
				&& ev.IsActive()
				&& ev.GetActivePage() != nullptr) {
			return false;
		}
	}
	for (auto vid: { Game_Vehicle::Boat, Game_Vehicle::Ship }) {
		auto& vehicle = vehicles[vid - 1];
		if (vehicle.IsInCurrentMap() && vehicle.IsInPosition(x, y)) {
			return false;
		}
	}

	const int bit = Passable::Down | Passable::Right | Passable::Left | Passable::Up;

	int tile_index = x + y * GetTilesX();

	if (!IsPassableLowerTile(bit, tile_index)) {
		return false;
	}

	int tile_id = map->upper_layer[tile_index] - BLOCK_F;
	tile_id = map_info.upper_tiles[tile_id];

	return (passages_up[tile_id] & bit) != 0;
}

bool Game_Map::CanEmbarkShip(Game_Player& player, int x, int y) {
	auto bit = GetPassableMask(player.GetX(), player.GetY(), x, y);
	return IsPassableTile(&player, bit, player.GetX(), player.GetY());
}

bool Game_Map::CanDisembarkShip(Game_Player& player, int x, int y) {
	if (!Game_Map::IsValid(x, y)) {
		return false;
	}

	for (auto& ev: GetEvents()) {
		if (ev.IsInPosition(x, y)
			&& ev.GetLayer() == lcf::rpg::EventPage::Layers_same
			&& ev.IsActive()
			&& ev.GetActivePage() != nullptr) {
			return false;
		}
	}

	int bit = GetPassableMask(x, y, player.GetX(), player.GetY());

	return IsPassableTile(nullptr, bit, x, y);
}

bool Game_Map::IsPassableLowerTile(int bit, int tile_index) {
	int tile_raw_id = map->lower_layer[tile_index];
	int tile_id = 0;

	if (tile_raw_id >= BLOCK_E) {
		tile_id = tile_raw_id - BLOCK_E;
		tile_id = map_info.lower_tiles[tile_id] + BLOCK_E_INDEX;

	} else if (tile_raw_id >= BLOCK_D) {
		tile_id = (tile_raw_id - BLOCK_D) / BLOCK_D_STRIDE + BLOCK_D_INDEX;
		int autotile_id = (tile_raw_id - BLOCK_D) % BLOCK_D_STRIDE;

		if (((passages_down[tile_id] & Passable::Wall) != 0) && (
				(autotile_id >= 20 && autotile_id <= 23) ||
				(autotile_id >= 33 && autotile_id <= 37) ||
				autotile_id == 42 || autotile_id == 43 ||
				autotile_id == 45 || autotile_id == 46))
			return true;

	} else if (tile_raw_id >= BLOCK_C) {
		tile_id = (tile_raw_id - BLOCK_C) / BLOCK_C_STRIDE + BLOCK_C_INDEX;

	} else if (map->lower_layer[tile_index] < BLOCK_C) {
		tile_id = tile_raw_id / BLOCK_B_STRIDE;
	}

	return (passages_down[tile_id] & bit) != 0;
}

bool Game_Map::IsPassableTile(
		const Game_Character* self, int bit, int x, int y
		) {
	return IsPassableTile(
		self, bit, x, y, true, true
	);
}

bool Game_Map::IsPassableTile(
		const Game_Character* self, int bit, int x, int y,
		bool check_events_and_vehicles, bool check_map_geometry
		) {
	if (!IsValid(x, y)) return false;

	const auto vehicle_type = GetCollisionVehicleType(self);
	if (check_events_and_vehicles) {
		if (vehicle_type != Game_Vehicle::None) {
			const auto* terrain = lcf::ReaderUtil::GetElement(lcf::Data::terrains, GetTerrainTag(x, y));
			if (!terrain) {
				Output::Warning("IsPassableTile: Invalid terrain at ({}, {})", x, y);
				return false;
			}
			if (vehicle_type == Game_Vehicle::Boat && !terrain->boat_pass) {
				return false;
			}
			if (vehicle_type == Game_Vehicle::Ship && !terrain->ship_pass) {
				return false;
			}
			if (vehicle_type == Game_Vehicle::Airship) {
				return terrain->airship_pass;
			}
		}

		// Highest ID event with layer=below, not through, and a tile graphic wins.
		int event_tile_id = 0;
		for (auto& ev: events) {
			if (self == &ev) {
				continue;
			}
			if (!ev.IsActive() || ev.GetActivePage() == nullptr || ev.GetThrough()) {
				continue;
			}
			if (ev.IsInPosition(x, y) && ev.GetLayer() == lcf::rpg::EventPage::Layers_below) {
				if (ev.HasTileSprite()) {
					event_tile_id = ev.GetTileId();
				}
			}
		}

		// If there was a below tile event, and the tile is not above
		// Override the chipset with event tile behavior.
		if (event_tile_id > 0
				&& ((passages_up[event_tile_id] & Passable::Above) == 0)) {
			switch (vehicle_type) {
				case Game_Vehicle::None:
					return ((passages_up[event_tile_id] & bit) != 0);
				case Game_Vehicle::Boat:
				case Game_Vehicle::Ship:
					return false;
				case Game_Vehicle::Airship:
					break;
			};
		}
	}

	if (check_map_geometry) {
		int tile_index = x + y * GetTilesX();
		int tile_id = map->upper_layer[tile_index] - BLOCK_F;
		tile_id = map_info.upper_tiles[tile_id];

		if (vehicle_type == Game_Vehicle::Boat || vehicle_type == Game_Vehicle::Ship) {
			if ((passages_up[tile_id] & Passable::Above) == 0)
				return false;
			return true;
		}

		if ((passages_up[tile_id] & bit) == 0)
			return false;

		if ((passages_up[tile_id] & Passable::Above) == 0)
			return true;

		return IsPassableLowerTile(bit, tile_index);
	} else {
		return true;
	}
}

int Game_Map::GetBushDepth(int x, int y) {
	if (!Game_Map::IsValid(x, y)) return 0;

	const lcf::rpg::Terrain* terrain = lcf::ReaderUtil::GetElement(lcf::Data::terrains, GetTerrainTag(x,y));
	if (!terrain) {
		Output::Warning("GetBushDepth: Invalid terrain at ({}, {})", x, y);
		return 0;
	}
	return terrain->bush_depth;
}

bool Game_Map::IsCounter(int x, int y) {
	if (!Game_Map::IsValid(x, y)) return false;

	int const tile_id = map->upper_layer[x + y * GetTilesX()];
	if (tile_id < BLOCK_F) return false;
	int const index = map_info.upper_tiles[tile_id - BLOCK_F];
	return !!(passages_up[index] & Passable::Counter);
}

int Game_Map::GetTerrainTag(int x, int y) {
	if (!chipset) {
		// FIXME: Is this ever possible?
		return 1;
	}

	auto& terrain_data = chipset->terrain_data;

	if (terrain_data.empty()) {
		// RPG_RT optimisation: When the terrain is all 1, no terrain data is stored
		return 1;
	}

	// Terrain tag wraps on looping maps
	if (Game_Map::LoopHorizontal()) {
		x = RoundX(x);
	}
	if (Game_Map::LoopVertical()) {
		y = RoundY(y);
	}

	// RPG_RT always uses the terrain of the first lower tile
	// for out of bounds coordinates.
	unsigned chip_index = 0;

	if (Game_Map::IsValid(x, y)) {
		const auto chip_id = map->lower_layer[x + y * GetTilesX()];
		chip_index = ChipIdToIndex(chip_id);

		// Apply tile substitution
		if (chip_index >= BLOCK_E_INDEX && chip_index < NUM_LOWER_TILES) {
			chip_index = map_info.lower_tiles[chip_index - BLOCK_E_INDEX] + BLOCK_E_INDEX;
		}
	}

	assert(chip_index < terrain_data.size());

	return terrain_data[chip_index];
}

Game_Event* Game_Map::GetEventAt(int x, int y, bool require_active) {
	auto& events = GetEvents();
	for (auto iter = events.rbegin(); iter != events.rend(); ++iter) {
		auto& ev = *iter;
		if (ev.IsInPosition(x, y) && (!require_active || ev.IsActive())) {
			return &ev;
		}
	}
	return nullptr;
}

bool Game_Map::LoopHorizontal() {
	return map->scroll_type == lcf::rpg::Map::ScrollType_horizontal || map->scroll_type == lcf::rpg::Map::ScrollType_both;
}

bool Game_Map::LoopVertical() {
	return map->scroll_type == lcf::rpg::Map::ScrollType_vertical || map->scroll_type == lcf::rpg::Map::ScrollType_both;
}

int Game_Map::RoundX(int x, int units) {
	if (LoopHorizontal()) {
		return Utils::PositiveModulo(x, GetTilesX() * units);
	} else {
		return x;
	}
}

int Game_Map::RoundY(int y, int units) {
	if (LoopVertical()) {
		return Utils::PositiveModulo(y, GetTilesY() * units);
	} else {
		return y;
	}
}

int Game_Map::RoundDx(int dx, int units) {
	if (LoopHorizontal()) {
		return Utils::PositiveModulo(std::abs(dx), GetTilesX() * units) * Utils::Sign(dx);
	} else {
		return dx;
	}
}

int Game_Map::RoundDy(int dy, int units) {
	if (LoopVertical()) {
		return Utils::PositiveModulo(std::abs(dy), GetTilesY() * units) * Utils::Sign(dy);
	} else {
		return dy;
	}
}

int Game_Map::XwithDirection(int x, int direction) {
	return RoundX(x + (direction == lcf::rpg::EventPage::Direction_right ? 1 : direction == lcf::rpg::EventPage::Direction_left ? -1 : 0));
}

int Game_Map::YwithDirection(int y, int direction) {
	return RoundY(y + (direction == lcf::rpg::EventPage::Direction_down ? 1 : direction == lcf::rpg::EventPage::Direction_up ? -1 : 0));
}

int Game_Map::CheckEvent(int x, int y) {
	for (const Game_Event& ev : events) {
		if (ev.IsInPosition(x, y)) {
			return ev.GetId();
		}
	}

	return 0;
}

void Game_Map::Update(MapUpdateAsyncContext& actx, bool is_preupdate) {
	if (GetNeedRefresh()) {
		Refresh();
	}

	if (!actx.IsActive()) {
		//If not resuming from async op ...
		UpdateProcessedFlags(is_preupdate);
	}

	if (!actx.IsActive() || actx.IsParallelCommonEvent()) {
		if (!UpdateCommonEvents(actx)) {
			// Suspend due to common event async op ...
			return;
		}
	}

	if (!actx.IsActive() || actx.IsParallelMapEvent()) {
		if (!UpdateMapEvents(actx)) {
			// Suspend due to map event async op ...
			return;
		}
	}

	if (is_preupdate) {
		return;
	}

	if (!actx.IsActive()) {
		//If not resuming from async op ...
		Main_Data::game_player->Update();

		for (auto& vehicle: vehicles) {
			if (vehicle.GetMapId() == GetMapId()) {
				vehicle.Update();
			}
		}
	}

	if (!actx.IsActive() || actx.IsMessage()) {
		if (!UpdateMessage(actx)) {
			// Suspend due to message async op ...
			return;
		}
	}

	if (!actx.IsActive()) {
		Main_Data::game_party->UpdateTimers();
		Main_Data::game_screen->Update();
		Main_Data::game_pictures->Update(false);
	}

	if (!actx.IsActive() || actx.IsForegroundEvent()) {
		if (!UpdateForegroundEvents(actx)) {
			// Suspend due to foreground event async op ...
			return;
		}
	}

	Parallax::Update();

	actx = {};
}

void Game_Map::UpdateProcessedFlags(bool is_preupdate) {
	for (Game_Event& ev : events) {
		ev.SetProcessed(false);
	}
	if (!is_preupdate) {
		Main_Data::game_player->SetProcessed(false);
		for (auto& vehicle: vehicles) {
			if (vehicle.IsInCurrentMap()) {
				vehicle.SetProcessed(false);
			}
		}
	}
}


bool Game_Map::UpdateCommonEvents(MapUpdateAsyncContext& actx) {
	int resume_ce = actx.GetParallelCommonEvent();

	for (Game_CommonEvent& ev : common_events) {
		bool resume_async = false;
		if (resume_ce != 0) {
			// If resuming, skip all until the event to resume from ..
			if (ev.GetIndex() != resume_ce) {
				continue;
			} else {
				resume_ce = 0;
				resume_async = true;
			}
		}

		auto aop = ev.Update(resume_async);
		if (aop.IsActive()) {
			// Suspend due to this event ..
			actx = MapUpdateAsyncContext::FromCommonEvent(ev.GetIndex(), aop);
			return false;
		}
	}

	actx = {};
	return true;
}

bool Game_Map::UpdateMapEvents(MapUpdateAsyncContext& actx) {
	int resume_ev = actx.GetParallelMapEvent();

	for (Game_Event& ev : events) {
		bool resume_async = false;
		if (resume_ev != 0) {
			// If resuming, skip all until the event to resume from ..
			if (ev.GetId() != resume_ev) {
				continue;
			} else {
				resume_ev = 0;
				resume_async = true;
			}
		}

		auto aop = ev.Update(resume_async);
		if (aop.IsActive()) {
			// Suspend due to this event ..
			actx = MapUpdateAsyncContext::FromMapEvent(ev.GetId(), aop);
			return false;
		}
	}

	actx = {};
	return true;
}

bool Game_Map::UpdateMessage(MapUpdateAsyncContext& actx) {
	// Message system does not support suspend and resume internally. So if the last frame the message
	// produced an async event, the message loop finished completely. Therefore this frame we should
	// resume *after* the message and not run it again.
	if (!actx.IsActive()) {
		auto aop = Game_Message::Update();
		if (aop.IsActive()) {
			actx = MapUpdateAsyncContext::FromMessage(aop);
			return false;
		}
	}

	actx = {};
	return true;
}

bool Game_Map::UpdateForegroundEvents(MapUpdateAsyncContext& actx) {
	auto& interp = GetInterpreter();

	// If we resume from async op, we don't clear the loop index.
	const bool resume_fg = actx.IsForegroundEvent();

	// Run any event loaded from last frame.
	interp.Update(!resume_fg);
	if (interp.IsAsyncPending()) {
		// Suspend due to this event ..
		actx = MapUpdateAsyncContext::FromForegroundEvent(interp.GetAsyncOp());
		return false;
	}

	while (!interp.IsRunning() && !interp.ReachedLoopLimit()) {
		interp.Clear();

		// This logic is probably one big loop in RPG_RT. We have to replicate
		// it here because once we stop executing from this we should not
		// clear anymore waiting flags.
		if (Scene::instance->HasRequestedScene() && interp.GetLoopCount() > 0) {
			break;
		}
		Game_CommonEvent* run_ce = nullptr;

		for (auto& ce: common_events) {
			if (ce.IsWaitingForegroundExecution()) {
				run_ce = &ce;
				break;
			}
		}
		if (run_ce) {
			interp.Push<InterpreterExecutionType::AutoStart>(run_ce);
		}

		Game_Event* run_ev = nullptr;
		for (auto& ev: events) {
			if (ev.IsWaitingForegroundExecution()) {
				if (!ev.IsActive()) {
					ev.ClearWaitingForegroundExecution();
					continue;
				}
				run_ev = &ev;
				break;
			}
		}
		if (run_ev) {
			if (run_ev->WasStartedByDecisionKey()) {
				interp.Push<InterpreterExecutionType::Action>(run_ev);
			} else {
				switch (run_ev->GetTrigger()) {
					case lcf::rpg::EventPage::Trigger_touched:
						interp.Push<InterpreterExecutionType::Touch>(run_ev);
						break;
					case lcf::rpg::EventPage::Trigger_collision:
						interp.Push<InterpreterExecutionType::Collision>(run_ev);
						break;
					case lcf::rpg::EventPage::Trigger_auto_start:
						interp.Push<InterpreterExecutionType::AutoStart>(run_ev);
						break;
					case lcf::rpg::EventPage::Trigger_action:
					default:
						interp.Push<InterpreterExecutionType::Action>(run_ev);
						break;
				}
			}
			run_ev->ClearWaitingForegroundExecution();
		}

		// If no events to run we're finished.
		if (!interp.IsRunning()) {
			break;
		}

		interp.Update(false);
		if (interp.IsAsyncPending()) {
			// Suspend due to this event ..
			actx = MapUpdateAsyncContext::FromForegroundEvent(interp.GetAsyncOp());
			return false;
		}
	}

	actx = {};
	return true;
}

lcf::rpg::MapInfo const& Game_Map::GetMapInfo() {
	return GetMapInfo(GetMapId());
}

lcf::rpg::MapInfo const& Game_Map::GetMapInfo(int map_id) {
	for (const auto& mi: lcf::Data::treemap.maps) {
		if (mi.ID == map_id) {
			return mi;
		}
	}

	Output::Debug("Map {} not in Maptree", map_id);
	return empty_map_info;
}

const lcf::rpg::MapInfo& Game_Map::GetParentMapInfo() {
	return GetParentMapInfo(GetMapInfo());
}

const lcf::rpg::MapInfo& Game_Map::GetParentMapInfo(const lcf::rpg::MapInfo& map_info) {
	return GetMapInfo(map_info.parent_map);
}

lcf::rpg::Map const& Game_Map::GetMap() {
	return *map;
}

int Game_Map::GetMapId() {
	return Main_Data::game_player->GetMapId();
}

void Game_Map::PrintPathToMap() {
	const auto* current_info = &GetMapInfo();
	std::ostringstream ss;
	ss << current_info->name;

	current_info = &GetParentMapInfo(*current_info);
	while (current_info->ID != 0 && current_info->ID != GetMapId()) {
		ss << " < " << current_info->name;
		current_info = &GetParentMapInfo(*current_info);
	}

	Output::Debug("Tree: {}", ss.str());
}

int Game_Map::GetTilesX() {
	return map->width;
}

int Game_Map::GetTilesY() {
	return map->height;
}

int Game_Map::GetOriginalEncounterSteps() {
	return GetMapInfo().encounter_steps;
}

int Game_Map::GetEncounterSteps() {
	return map_info.encounter_steps;
}

void Game_Map::SetEncounterSteps(int step) {
	if (step < 0) {
		step = GetOriginalEncounterSteps();
	}
	map_info.encounter_steps = step;
}

std::vector<int> Game_Map::GetEncountersAt(int x, int y) {
	int terrain_tag = GetTerrainTag(Main_Data::game_player->GetX(), Main_Data::game_player->GetY());

	std::function<bool(int)> is_acceptable = [=](int troop_id) {
		const lcf::rpg::Troop* troop = lcf::ReaderUtil::GetElement(lcf::Data::troops, troop_id);
		if (!troop) {
			Output::Warning("GetEncountersAt: Invalid troop ID {} in encounter list", troop_id);
			return false;
		}

		const auto& terrain_set = troop->terrain_set;

		// RPG_RT optimisation: Omitted entries are the default value (true)
		return terrain_set.size() <= (unsigned)(terrain_tag - 1) ||
				terrain_set[terrain_tag - 1];
	};

	std::vector<int> out;

	for (unsigned int i = 0; i < lcf::Data::treemap.maps.size(); ++i) {
		lcf::rpg::MapInfo& map = lcf::Data::treemap.maps[i];

		if (map.ID == GetMapId()) {
			for (const auto& enc : map.encounters) {
				if (is_acceptable(enc.troop_id)) {
					out.push_back(enc.troop_id);
				}
			}
		} else if (map.parent_map == GetMapId() && map.type == lcf::rpg::TreeMap::MapType_area) {
			// Area
			Rect area_rect(map.area_rect.l, map.area_rect.t, map.area_rect.r - map.area_rect.l, map.area_rect.b - map.area_rect.t);
			Rect player_rect(x, y, 1, 1);

			if (!player_rect.IsOutOfBounds(area_rect)) {
				for (const lcf::rpg::Encounter& enc : map.encounters) {
					if (is_acceptable(enc.troop_id)) {
						out.push_back(enc.troop_id);
					}
				}
			}
		}
	}

	return out;
}

static void OnEncounterEnd(BattleResult result) {
	if (result != BattleResult::Defeat) {
		return;
	}

	if (!Game_Battle::HasDeathHandler()) {
		Scene::Push(std::make_shared<Scene_Gameover>());
		return;
	}

	//2k3 death handler

	auto* ce = lcf::ReaderUtil::GetElement(common_events, Game_Battle::GetDeathHandlerCommonEvent());
	if (ce) {
		auto& interp = Game_Map::GetInterpreter();
		interp.Push<InterpreterExecutionType::DeathHandler>(ce);
	}

	auto tt = Game_Battle::GetDeathHandlerTeleport();
	if (tt.IsActive()) {
		Main_Data::game_player->ReserveTeleport(tt.GetMapId(), tt.GetX(), tt.GetY(), tt.GetDirection(), tt.GetType());
	}
}

bool Game_Map::PrepareEncounter(BattleArgs& args) {
	int x = Main_Data::game_player->GetX();
	int y = Main_Data::game_player->GetY();

	std::vector<int> encounters = GetEncountersAt(x, y);

	if (encounters.empty()) {
		// No enemies on this map :(
		return false;
	}

	args.troop_id = encounters[Rand::GetRandomNumber(0, encounters.size() - 1)];

	if (RuntimePatches::EncounterRandomnessAlert::HandleEncounter(args.troop_id)) {
		//Cancel the battle setup
		return false;
	}

	if (Feature::HasRpg2kBattleSystem()) {
		if (Rand::ChanceOf(1, 32)) {
			args.first_strike = true;
		}
	} else {
		const auto* terrain = lcf::ReaderUtil::GetElement(lcf::Data::terrains, GetTerrainTag(x, y));
		if (!terrain) {
			Output::Warning("PrepareEncounter: Invalid terrain at ({}, {})", x, y);
		} else {
			if (terrain->special_flags.back_party && Rand::PercentChance(terrain->special_back_party)) {
				args.condition = lcf::rpg::System::BattleCondition_initiative;
			} else if (terrain->special_flags.back_enemies && Rand::PercentChance(terrain->special_back_enemies)) {
				args.condition = lcf::rpg::System::BattleCondition_back;
			} else if (terrain->special_flags.lateral_party && Rand::PercentChance(terrain->special_lateral_party)) {
				args.condition = lcf::rpg::System::BattleCondition_surround;
			} else if (terrain->special_flags.lateral_enemies && Rand::PercentChance(terrain->special_lateral_enemies)) {
				args.condition = lcf::rpg::System::BattleCondition_pincers;
			}
		}
	}

	SetupBattle(args);
	args.on_battle_end = OnEncounterEnd;
	args.allow_escape = true;

	return true;
}

void Game_Map::SetupBattle(BattleArgs& args) {
	int x = Main_Data::game_player->GetX();
	int y = Main_Data::game_player->GetY();

	args.terrain_id = GetTerrainTag(x, y);

	const auto* current_info = &GetMapInfo();
	while (current_info->background_type == 0 && GetParentMapInfo(*current_info).ID != current_info->ID) {
		current_info = &GetParentMapInfo(*current_info);
	}

	if (current_info->background_type == 2) {
		args.background = ToString(current_info->background_name);
	}
}

std::vector<short>& Game_Map::GetMapDataDown() {
	return map->lower_layer;
}

std::vector<short>& Game_Map::GetMapDataUp() {
	return map->upper_layer;
}

int Game_Map::GetOriginalChipset() {
	return map != nullptr ? map->chipset_id : 0;
}

int Game_Map::GetChipset() {
	return chipset != nullptr ? chipset->ID : 0;
}

std::string_view Game_Map::GetChipsetName() {
	return chipset != nullptr
		? std::string_view(chipset->chipset_name)
		: std::string_view("");
}

int Game_Map::GetPositionX() {
	return map_info.position_x;
}

int Game_Map::GetDisplayX() {
	return map_info.position_x + Main_Data::game_screen->GetShakeOffsetX() * 16;
}

void Game_Map::SetPositionX(int x, bool reset_panorama) {
	const int map_width = GetTilesX() * SCREEN_TILE_SIZE;
	if (LoopHorizontal()) {
		x = Utils::PositiveModulo(x, map_width);
	} else {
		// Do not use std::clamp here. When the map is smaller than the screen the
		// upper bound is smaller than the lower bound making the function fail.
		x = std::max(0, std::min(map_width - screen_width, x));
	}
	map_info.position_x = x;
	if (reset_panorama) {
		Parallax::SetPositionX(map_info.position_x);
		Parallax::ResetPositionX();
	}
}

int Game_Map::GetPositionY() {
	return map_info.position_y;
}

int Game_Map::GetDisplayY() {
	return map_info.position_y + Main_Data::game_screen->GetShakeOffsetY() * 16;
}

void Game_Map::SetPositionY(int y, bool reset_panorama) {
	const int map_height = GetTilesY() * SCREEN_TILE_SIZE;
	if (LoopVertical()) {
		y = Utils::PositiveModulo(y, map_height);
	} else {
		// Do not use std::clamp here. When the map is smaller than the screen the
		// upper bound is smaller than the lower bound making the function fail.
		y = std::max(0, std::min(map_height - screen_height, y));
	}
	map_info.position_y = y;
	if (reset_panorama) {
		Parallax::SetPositionY(map_info.position_y);
		Parallax::ResetPositionY();
	}
}

bool Game_Map::GetNeedRefresh() {
	int anti_lag_switch = Player::game_config.patch_anti_lag_switch.Get();
	if (anti_lag_switch > 0 && Main_Data::game_switches->Get(anti_lag_switch)) {
		return false;
	}

	return need_refresh;
}

void Game_Map::SetNeedRefresh(bool refresh) {
	need_refresh = refresh;
}

void Game_Map::SetNeedRefreshForSwitchChange(int switch_id) {
	if (need_refresh)
		return;
	if (map_cache->GetNeedRefresh<Caching::ObservedVarOps::SwitchSet>(switch_id))
		SetNeedRefresh(true);
}

void Game_Map::SetNeedRefreshForVarChange(int var_id) {
	if (need_refresh)
		return;
	if (map_cache->GetNeedRefresh<Caching::ObservedVarOps::VarSet>(var_id))
		SetNeedRefresh(true);
}

void Game_Map::SetNeedRefreshForSwitchChange(std::initializer_list<int> switch_ids) {
	for (auto switch_id: switch_ids) {
		SetNeedRefreshForSwitchChange(switch_id);
	}
}

void Game_Map::SetNeedRefreshForVarChange(std::initializer_list<int> var_ids) {
	for (auto var_id: var_ids) {
		SetNeedRefreshForVarChange(var_id);
	}
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

std::vector<Game_Event>& Game_Map::GetEvents() {
	return events;
}

int Game_Map::GetHighestEventId() {
	int id = 0;
	for (auto& ev: events) {
		id = std::max(id, ev.GetId());
	}
	return id;
}

Game_Event* Game_Map::GetEvent(int event_id) {
	auto it = std::find_if(events.begin(), events.end(),
			[&event_id](Game_Event& ev) {return ev.GetId() == event_id;});
	return it == events.end() ? nullptr : &(*it);
}

std::vector<Game_CommonEvent>& Game_Map::GetCommonEvents() {
	return common_events;
}

std::string_view Game_Map::GetMapName(int id) {
	for (unsigned int i = 0; i < lcf::Data::treemap.maps.size(); ++i) {
		if (lcf::Data::treemap.maps[i].ID == id) {
			return lcf::Data::treemap.maps[i].name;
		}
	}
	// nothing found
	return {};
}

void Game_Map::SetChipset(int id) {
	if (id == 0) {
		// This emulates RPG_RT behavior, where chipset id == 0 means use the default map chipset.
		id = GetOriginalChipset();
	}
	map_info.chipset_id = id;

	if (!ReloadChipset()) {
		Output::Warning("SetChipset: Invalid chipset ID {}", map_info.chipset_id);
	} else {
		passages_down = chipset->passable_data_lower;
		passages_up = chipset->passable_data_upper;
		animation_type = chipset->animation_type;
		animation_fast = chipset->animation_speed != 0;
	}

	if (passages_down.size() < 162)
		passages_down.resize(162, (unsigned char) 0x0F);
	if (passages_up.size() < 144)
		passages_up.resize(144, (unsigned char) 0x0F);
}

bool Game_Map::ReloadChipset() {
	chipset = lcf::ReaderUtil::GetElement(lcf::Data::chipsets, map_info.chipset_id);
	if (!chipset) {
		return false;
	}
	return true;
}

void Game_Map::OnTranslationChanged() {
	ReloadChipset();
	// Marks common events for reload on map change
	// This is not save to do while they are executing
	translation_changed = true;
}

Game_Vehicle* Game_Map::GetVehicle(Game_Vehicle::Type which) {
	if (which == Game_Vehicle::Boat ||
		which == Game_Vehicle::Ship ||
		which == Game_Vehicle::Airship) {
		return &vehicles[which - 1];
	}

	return nullptr;
}

bool Game_Map::IsAnyEventStarting() {
	for (Game_Event& ev : events)
		if (ev.IsWaitingForegroundExecution() && !ev.GetList().empty() && ev.IsActive())
			return true;

	for (Game_CommonEvent& ev : common_events)
		if (ev.IsWaitingForegroundExecution())
			return true;

	return false;
}

bool Game_Map::IsAnyMovePending() {
	auto check = [](auto& ev) {
		return ev.IsMoveRouteOverwritten() && !ev.IsMoveRouteFinished();
	};
	const auto map_id = GetMapId();
	if (check(*Main_Data::game_player)) {
		return true;
	}
	for (auto& vh: vehicles) {
		if (vh.GetMapId() == map_id && check(vh)) {
			return true;
		}
	}
	for (auto& ev: events) {
		if (check(ev)) {
			return true;
		}
	}

	return false;
}

void Game_Map::RemoveAllPendingMoves() {
	const auto map_id = GetMapId();
	Main_Data::game_player->CancelMoveRoute();
	for (auto& vh: vehicles) {
		if (vh.GetMapId() == map_id) {
			vh.CancelMoveRoute();
		}
	}
	for (auto& ev: events) {
		ev.CancelMoveRoute();
	}
}

static int DoSubstitute(std::vector<uint8_t>& tiles, int old_id, int new_id) {
	int num_subst = 0;
	for (size_t i = 0; i < tiles.size(); ++i) {
		if (tiles[i] == old_id) {
			tiles[i] = (uint8_t) new_id;
			++num_subst;
		}
	}
	return num_subst;
}

int Game_Map::SubstituteDown(int old_id, int new_id) {
	return DoSubstitute(map_info.lower_tiles, old_id, new_id);
}

int Game_Map::SubstituteUp(int old_id, int new_id) {
	return DoSubstitute(map_info.upper_tiles, old_id, new_id);
}

void Game_Map::ReplaceTileAt(int x, int y, int new_id, int layer) {
	auto pos = x + y * map->width;
	auto& layer_vec = layer >= 1 ? map->upper_layer : map->lower_layer;
	layer_vec[pos] = static_cast<int16_t>(new_id);
}

int Game_Map::GetTileIdAt(int x, int y, int layer, bool chip_id_or_index) {
	if (x < 0 || x >= map->width || y < 0 || y >= map->height) {
		return 0;  // Return 0 for out-of-bounds coordinates
	}

	auto pos = x + y * map->width;
	auto& layer_vec = layer >= 1 ? map->upper_layer : map->lower_layer;

	int tile_output = chip_id_or_index ? layer_vec[pos] : ChipIdToIndex(layer_vec[pos]);
	if (layer >= 1) tile_output -= BLOCK_F_INDEX;

	return tile_output;
}

std::vector<int> Game_Map::GetTilesIdAt(Rect coords, int layer, bool chip_id_or_index) {
	std::vector<int> tiles_collection;
	for (int i = 0; i < coords.height; ++i) {
		for (int j = 0; j < coords.width; ++j) {
			tiles_collection.emplace_back(Game_Map::GetTileIdAt(coords.x + j, coords.y + i, layer, chip_id_or_index));
		}
	}
	return tiles_collection;
}

std::string Game_Map::ConstructMapName(int map_id, bool is_easyrpg) {
	std::stringstream ss;
	ss << "Map" << std::setfill('0') << std::setw(4) << map_id;
	if (is_easyrpg) {
		return Player::fileext_map.MakeFilename(ss.str(), SUFFIX_EMU);
	} else {
		return Player::fileext_map.MakeFilename(ss.str(), SUFFIX_LMU);
	}
}

FileRequestAsync* Game_Map::RequestMap(int map_id) {
#ifdef EMSCRIPTEN
	Player::translation.RequestAndAddMap(map_id);
#endif

	auto* request = AsyncHandler::RequestFile(Game_Map::ConstructMapName(map_id, false));
	request->SetImportantFile(true);
	return request;
}

// MapEventCache
//////////////////
void Game_Map::Caching::MapEventCache::AddEvent(const lcf::rpg::Event& ev) {
	auto id = ev.ID;

	if (std::find(event_ids.begin(), event_ids.end(), id) == event_ids.end()) {
		event_ids.emplace_back(id);
	}
}

void Game_Map::Caching::MapEventCache::RemoveEvent(const lcf::rpg::Event& ev) {
	auto id = ev.ID;

	auto it = std::find(event_ids.begin(), event_ids.end(), id);

	if (it != event_ids.end()) {
		event_ids.erase(it);
	}
}

// Parallax
/////////////

namespace {
	int parallax_width;
	int parallax_height;

	bool parallax_fake_x;
	bool parallax_fake_y;
}

/* Helper function to get the current parallax parameters. If the default
 * parallax for the current map was overridden by a "Change Parallax BG"
 * command, the result is filled out from those values in the SaveMapInfo.
 * Otherwise, the result is filled out from the default for the current map.
 */
static Game_Map::Parallax::Params GetParallaxParams() {
	Game_Map::Parallax::Params params = {};

	if (!map_info.parallax_name.empty()) {
		params.name = map_info.parallax_name;
		params.scroll_horz = map_info.parallax_horz;
		params.scroll_horz_auto = map_info.parallax_horz_auto;
		params.scroll_horz_speed = map_info.parallax_horz_speed;
		params.scroll_vert = map_info.parallax_vert;
		params.scroll_vert_auto = map_info.parallax_vert_auto;
		params.scroll_vert_speed = map_info.parallax_vert_speed;
	} else if (map->parallax_flag) {
		// Default case when map parallax hasn't been overwritten.
		params.name = ToString(map->parallax_name);
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
	return (-panorama.pan_x / TILE_SIZE) / 2;
}

int Game_Map::Parallax::GetY() {
	return (-panorama.pan_y / TILE_SIZE) / 2;
}

void Game_Map::Parallax::Initialize(int width, int height) {
	parallax_width = width;
	parallax_height = height;

	if (panorama_on_map_init) {
		SetPositionX(map_info.position_x);
		SetPositionY(map_info.position_y);
	}

	if (reset_panorama_x_on_next_init) {
		ResetPositionX();
	}
	if (reset_panorama_y_on_next_init) {
		ResetPositionY();
	}

	if (Player::IsRPG2k() && !panorama_on_map_init) {
		SetPositionX(panorama.pan_x);
		SetPositionY(panorama.pan_y);
	}

	panorama_on_map_init = false;
}

void Game_Map::Parallax::AddPositionX(int off_x) {
	SetPositionX(panorama.pan_x + off_x);
}

void Game_Map::Parallax::AddPositionY(int off_y) {
	SetPositionY(panorama.pan_y + off_y);
}

void Game_Map::Parallax::SetPositionX(int x) {
	// FIXME: Fixes a crash with ChangeBG commands in events, but not correct.
	// Real fix TBD
	if (parallax_width) {
		const int w = parallax_width * TILE_SIZE * 2;
		panorama.pan_x = (x + w) % w;
	}
}

void Game_Map::Parallax::SetPositionY(int y) {
	// FIXME: Fixes a crash with ChangeBG commands in events, but not correct.
	// Real fix TBD
	if (parallax_height) {
		const int h = parallax_height * TILE_SIZE * 2;
		panorama.pan_y = (y + h) % h;
	}
}

void Game_Map::Parallax::ResetPositionX() {
	Params params = GetParallaxParams();

	if (params.name.empty()) {
		return;
	}

	parallax_fake_x = false;

	if (!params.scroll_horz && !LoopHorizontal()) {
		int pan_screen_width = Player::screen_width;
		if (Player::game_config.fake_resolution.Get()) {
			pan_screen_width = SCREEN_TARGET_WIDTH;
		}

		int tiles_per_screen = pan_screen_width / TILE_SIZE;
		if (pan_screen_width % TILE_SIZE != 0) {
			++tiles_per_screen;
		}

		if (GetTilesX() > tiles_per_screen && parallax_width > pan_screen_width) {
			const int w = (GetTilesX() - tiles_per_screen) * TILE_SIZE;
			const int ph = 2 * std::min(w, parallax_width - pan_screen_width) * map_info.position_x / w;
			if (Player::IsRPG2k()) {
				SetPositionX(ph);
			} else {
				// 2k3 does not do the (% parallax_width * TILE_SIZE * 2) here
				panorama.pan_x = ph;
			}
		} else {
			panorama.pan_x = 0;
			parallax_fake_x = true;
		}
	} else {
		parallax_fake_x = true;
	}
}

void Game_Map::Parallax::ResetPositionY() {
	Params params = GetParallaxParams();

	if (params.name.empty()) {
		return;
	}

	parallax_fake_y = false;

	if (!params.scroll_vert && !Game_Map::LoopVertical()) {
		int pan_screen_height = Player::screen_height;
		if (Player::game_config.fake_resolution.Get()) {
			pan_screen_height = SCREEN_TARGET_HEIGHT;
		}

		int tiles_per_screen = pan_screen_height / TILE_SIZE;
		if (pan_screen_height % TILE_SIZE != 0) {
			++tiles_per_screen;
		}

		if (GetTilesY() > tiles_per_screen && parallax_height > pan_screen_height) {
			const int h = (GetTilesY() - tiles_per_screen) * TILE_SIZE;
			const int pv = 2 * std::min(h, parallax_height - pan_screen_height) * map_info.position_y / h;
			SetPositionY(pv);
		} else {
			panorama.pan_y = 0;
			parallax_fake_y = true;
		}
	} else {
		parallax_fake_y = true;
	}
}

void Game_Map::Parallax::ScrollRight(int distance) {
	if (!distance) {
		return;
	}

	Params params = GetParallaxParams();
	if (params.name.empty()) {
		return;
	}

	if (params.scroll_horz) {
		AddPositionX(distance);
		return;
	}

	if (Game_Map::LoopHorizontal()) {
		return;
	}

	ResetPositionX();
}

void Game_Map::Parallax::ScrollDown(int distance) {
	if (!distance) {
		return;
	}

	Params params = GetParallaxParams();
	if (params.name.empty()) {
		return;
	}

	if (params.scroll_vert) {
		AddPositionY(distance);
		return;
	}

	if (Game_Map::LoopVertical()) {
		return;
	}

	ResetPositionY();
}

void Game_Map::Parallax::Update() {
	Params params = GetParallaxParams();

	if (params.name.empty())
		return;

	auto scroll_amt = [](int speed) {
		return speed < 0 ? (1 << -speed) : -(1 << speed);
	};

	if (params.scroll_horz
			&& params.scroll_horz_auto
			&& params.scroll_horz_speed != 0) {
		AddPositionX(scroll_amt(params.scroll_horz_speed));
	}

	if (params.scroll_vert
			&& params.scroll_vert_auto
			&& params.scroll_vert_speed != 0) {
		if (parallax_height != 0) {
			AddPositionY(scroll_amt(params.scroll_vert_speed));
		}
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

	reset_panorama_x_on_next_init = !Game_Map::LoopHorizontal() && !map_info.parallax_horz;
	reset_panorama_y_on_next_init = !Game_Map::LoopVertical() && !map_info.parallax_vert;

	Scene_Map* scene = (Scene_Map*)Scene::Find(Scene::Map).get();
	if (!scene || !scene->spriteset)
		return;
	scene->spriteset->ParallaxUpdated();
}

void Game_Map::Parallax::ClearChangedBG() {
	Params params {}; // default Param indicates no override
	ChangeBG(params);
}

bool Game_Map::Parallax::FakeXPosition() {
	return parallax_fake_x;
}

bool Game_Map::Parallax::FakeYPosition() {
	return parallax_fake_y;
}
