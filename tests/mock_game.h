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
#include "player.h"
#include <lcf/data.h>
#include <lcf/rpg/moveroute.h>

enum class MockMap {
	eNone,
	ePassBlock20x15, // Left half is passable, right half is blocked
	ePass40x30,
	eMapCount
};

std::unique_ptr<lcf::rpg::Map> MakeMockMap(MockMap maptag);

class MockGame {
public:
	explicit MockGame(MockMap maptag);

	MockGame(const MockGame&) = delete;
	MockGame& operator=(const MockGame&) = delete;

	MockGame(MockGame&& o) : _restore(std::move(o._restore)) {
		std::swap(_active, o._active);
	}
	MockGame& operator=(MockGame&&) = delete;

	static Game_Player* GetPlayer();
	static Game_Vehicle* GetVehicle(Game_Vehicle::Type veh);
	static Game_Event* GetEvent(int id);

	void Reset();

	~MockGame() { Reset(); }
private:
	void DoReset();
private:
	struct {
	} _restore;
	bool _active = false;
};

class DebugGuard {
public:
	DebugGuard(bool f) {
		_flag = Player::debug_flag;
		Player::debug_flag = f;
	}

	DebugGuard(const DebugGuard&) = delete;
	DebugGuard& operator=(const DebugGuard&) = delete;

	~DebugGuard() {
		Player::debug_flag = _flag;
	}
private:
	bool _flag = false;
};

namespace {

using AnimType = lcf::rpg::EventPage::AnimType;
using MoveType = lcf::rpg::EventPage::MoveType;

constexpr auto Up = Game_Character::Up;
constexpr auto Right = Game_Character::Right;
constexpr auto Down = Game_Character::Down;
constexpr auto Left = Game_Character::Left;
constexpr auto UpRight = Game_Character::UpRight;
constexpr auto DownRight = Game_Character::DownRight;
constexpr auto DownLeft = Game_Character::DownLeft;
constexpr auto UpLeft = Game_Character::UpLeft;
}

inline void ForceUpdate(Game_Vehicle& ch) {
	ch.SetProcessed(false);
	ch.Update();
}

inline void ForceUpdate(Game_Player& ch) {
	ch.SetProcessed(false);
	ch.Update();
}

inline void ForceUpdate(Game_Event& ch) {
	ch.SetProcessed(false);
	ch.Update(false);
}

#endif
