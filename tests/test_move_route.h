#ifndef EP_TEST_MOVE_ROUTE_H
#define EP_TEST_MOVE_ROUTE_H

#include "game_map.h"
#include "game_vehicle.h"
#include "game_event.h"
#include "game_player.h"
#include "main_data.h"
#include "game_switches.h"
#include <lcf/data.h>
#include <lcf/rpg/moveroute.h>

namespace {

class MoveRouteCharacter : public Game_Vehicle {
	public:
		MoveRouteCharacter() : Game_Vehicle(Game_Vehicle::Boat) {
			SetDirection(Game_Character::Down);
			SetSpriteDirection(Game_Character::Down);
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
	MapGuard() {
		lcf::Data::treemap = {};
		lcf::Data::treemap.maps.push_back(lcf::rpg::MapInfo());
		lcf::Data::treemap.maps.back().type = lcf::rpg::TreeMap::MapType_root;
		lcf::Data::treemap.maps.push_back(lcf::rpg::MapInfo());
		lcf::Data::treemap.maps.back().ID = 1;
		lcf::Data::treemap.maps.back().type = lcf::rpg::TreeMap::MapType_map;

		lcf::Data::chipsets.push_back({});

		Game_Map::Init();
		Main_Data::game_switches = std::make_unique<Game_Switches>();
		Main_Data::game_player = std::make_unique<Game_Player>();
		Main_Data::game_player->SetMapId(1);

		auto map = std::make_unique<lcf::rpg::Map>();

		Game_Map::Setup(std::move(map));
	}

	~MapGuard() {
		Main_Data::game_switches = {};
		Main_Data::game_player = {};
		Game_Map::Quit();
		lcf::Data::treemap = {};
		lcf::Data::chipsets = {};
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

static auto MakeCharacter() {
	return MoveRouteCharacter();
}

template <typename T>
static void ForceUpdate(T& ch) {
	ch.SetProcessed(false);
	ch.Update();
}

#endif
