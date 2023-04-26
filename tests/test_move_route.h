#ifndef EP_TEST_MOVE_ROUTE_H
#define EP_TEST_MOVE_ROUTE_H

#include "game_map.h"
#include "game_vehicle.h"
#include "game_event.h"
#include "game_player.h"
#include "game_party.h"
#include "main_data.h"
#include "map_data.h"
#include "game_switches.h"
#include "game_variables.h"
#include "game_screen.h"
#include "game_pictures.h"
#include "input.h"
#include "output.h"
#include <lcf/data.h>
#include <lcf/rpg/moveroute.h>

namespace {

using AnimType = lcf::rpg::EventPage::AnimType;
using MoveType = lcf::rpg::EventPage::MoveType;

class MoveRouteVehicle : public Game_Vehicle {
	public:
		MoveRouteVehicle(Game_Vehicle::Type vt = Game_Vehicle::Boat) : Game_Vehicle(vt) {
			SetDirection(Game_Character::Down);
			SetFacing(Game_Character::Down);
			SetX(8);
			SetY(8);
		}

		bool MakeWay(int, int, int, int) override {
			return allow_movement;
		}

		void SetAllowMovement(bool allow) {
			allow_movement = allow;
		}
	private:
		bool allow_movement = true;
};

class MoveRouteEvent : public Game_Event {
	public:
		MoveRouteEvent() : Game_Event(1, &Game_Map::GetMap().events[0]) {
			SetDirection(Game_Character::Down);
			SetFacing(Game_Character::Down);
		}

		bool MakeWay(int, int, int, int) override {
			return allow_movement;
		}

		void SetAllowMovement(bool allow) {
			allow_movement = allow;
		}
	private:
		bool allow_movement = true;
};

struct MapGuard {
	MapGuard(int w = 20, int h = 15) {
		Input::ResetKeys();
		Main_Data::game_party = std::make_unique<Game_Party>();

		auto& treemap = lcf::Data::treemap;
		treemap = {};
		treemap.maps.push_back(lcf::rpg::MapInfo());
		treemap.maps.back().type = lcf::rpg::TreeMap::MapType_root;
		treemap.maps.push_back(lcf::rpg::MapInfo());
		treemap.maps.back().ID = 1;
		treemap.maps.back().type = lcf::rpg::TreeMap::MapType_map;

		lcf::Data::chipsets.push_back({});
		lcf::Data::terrains.push_back({});

		Game_Map::Init();
		Main_Data::game_switches = std::make_unique<Game_Switches>();
		Main_Data::game_variables = std::make_unique<Game_Variables>(Game_Variables::min_2k3, Game_Variables::max_2k3);
		Main_Data::game_pictures = std::make_unique<Game_Pictures>();
		Main_Data::game_screen = std::make_unique<Game_Screen>();
		Main_Data::game_player = std::make_unique<Game_Player>();
		Main_Data::game_player->SetMapId(1);

		auto map = std::make_unique<lcf::rpg::Map>();
		map->events.push_back({});
		map->events.back().ID = 1;
		map->events.back().pages.push_back({});
		map->events.back().pages.back().ID = 1;
		map->events.back().pages.back().move_type = lcf::rpg::EventPage::MoveType_stationary;
		map->events.back().pages.back().character_pattern = 1;
		// FIXME: Add a SetSize(w, h) method?
		map->width = w;
		map->height = h;
		map->upper_layer.resize(w * h, BLOCK_F);
		map->lower_layer.resize(w * h, 0);

		Game_Map::Setup(std::move(map));
	}

	~MapGuard() {
		Main_Data::game_switches = {};
		Main_Data::game_variables = {};
		Main_Data::game_player = {};
		Main_Data::game_screen = {};
		Main_Data::game_pictures = {};
		Game_Map::Quit();
		lcf::Data::treemap = {};
		lcf::Data::chipsets = {};
		lcf::Data::terrains = {};

		Main_Data::game_party.reset();
		Input::ResetKeys();
	}
};

constexpr auto Up = Game_Character::Up;
constexpr auto Right = Game_Character::Right;
constexpr auto Down = Game_Character::Down;
constexpr auto Left = Game_Character::Left;
constexpr auto UpRight = Game_Character::UpRight;
constexpr auto DownRight = Game_Character::DownRight;
constexpr auto DownLeft = Game_Character::DownLeft;
constexpr auto UpLeft = Game_Character::UpLeft;
}

static void ForceUpdate(Game_Vehicle& ch) {
	ch.SetProcessed(false);
	ch.Update();
}

static void ForceUpdate(Game_Player& ch) {
	ch.SetProcessed(false);
	ch.Update();
}

static void ForceUpdate(Game_Event& ch) {
	ch.SetProcessed(false);
	ch.Update(false);
}

#endif
