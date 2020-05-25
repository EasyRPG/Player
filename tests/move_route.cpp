#include "game_player.h"
#include "doctest.h"
#include "options.h"
#include "game_map.h"
#include "game_vehicle.h"
#include "main_data.h"
#include "game_switches.h"
#include <climits>
#include <initializer_list>

TEST_SUITE_BEGIN("MoveRoute");

namespace {
struct MapGuard {
	MapGuard() {
		Data::treemap = {};
		Data::treemap.maps.push_back(RPG::MapInfo());
		Data::treemap.maps.back().type = RPG::TreeMap::MapType_root;
		Data::treemap.maps.push_back(RPG::MapInfo());
		Data::treemap.maps.back().ID = 1;
		Data::treemap.maps.back().type = RPG::TreeMap::MapType_map;

		Data::chipsets.push_back({});

		Game_Map::Init();
		Main_Data::game_switches = std::make_unique<Game_Switches>();
		Main_Data::game_player = std::make_unique<Game_Player>();
		Main_Data::game_player->SetMapId(1);

		auto map = std::make_unique<RPG::Map>();

		Game_Map::Setup(std::move(map));
	}

	~MapGuard() {
		Main_Data::game_switches = {};
		Main_Data::game_player = {};
		Game_Map::Quit();
		Data::treemap = {};
		Data::chipsets = {};
	}
};

constexpr auto Up = Game_Character::Up;
constexpr auto Right = Game_Character::Right;
constexpr auto Down = Game_Character::Down;
constexpr auto Left = Game_Character::Left;
}

static Game_Vehicle MakeCharacter() {
	auto ch = Game_Vehicle(Game_Vehicle::Boat);
	ch.SetDirection(Game_Character::Down);
	ch.SetSpriteDirection(Game_Character::Down);
	return ch;
}

template <typename T>
static void ForceUpdate(T& ch) {
	ch.SetProcessed(false);
	ch.Update();
}

RPG::MoveRoute MakeRoute(std::initializer_list<RPG::MoveCommand> cmds, bool repeat = false, bool skip = false) {
	RPG::MoveRoute mr;
	mr.move_commands = cmds;
	mr.repeat = repeat;
	mr.skippable = skip;
	return mr;
}

static void testMoveRoute(
		const Game_Character& ch,
		bool paused, int move_frequency,
		int stop_count, int max_stop_count,
		int move_route_idx, bool overwritten, bool done,
		const RPG::MoveRoute& mr)
{
	REQUIRE_EQ(ch.IsPaused(), paused);
	REQUIRE_EQ(ch.GetMoveFrequency(), move_frequency);
	REQUIRE_EQ(ch.GetStopCount(), stop_count);
	REQUIRE_EQ(ch.GetMaxStopCount(), max_stop_count);
	REQUIRE_EQ(ch.GetMoveRouteIndex(), move_route_idx);
	REQUIRE_EQ(ch.IsMoveRouteOverwritten(), overwritten);
	REQUIRE_EQ(ch.IsMoveRouteRepeated(), done);
	REQUIRE_EQ(ch.GetMoveRoute(), mr);
}

template <typename... Args>
static void testMoveRouteDir(const Game_Character& ch,
		int dir, int facing,
		Args&&... args) {

	REQUIRE_EQ(ch.GetDirection(), dir);
	REQUIRE_EQ(ch.GetSpriteDirection(), facing);
	testMoveRoute(ch, std::forward<Args>(args)...);
}

TEST_CASE("DefaultMoveRoute") {
	auto ch = MakeCharacter();
	testMoveRoute(ch, false, 2, 0, 0, 0, false, false, RPG::MoveRoute());
}

TEST_CASE("ForceMoveRouteEmpty") {
	auto ch = MakeCharacter();
	ch.ForceMoveRoute(RPG::MoveRoute(), 2);
	testMoveRoute(ch, false, 2, 0xFFFF, 128, 0, false, false, RPG::MoveRoute());
}

TEST_CASE("ForceMoveRouteSameFreq") {
	auto ch = MakeCharacter();
	RPG::MoveRoute mr;
	mr.move_commands.push_back({});

	ch.ForceMoveRoute(mr, 2);
	testMoveRoute(ch, false, 2, 0xFFFF, 0, 0, true, false, mr);

	ch.CancelMoveRoute();
	testMoveRoute(ch, false, 2, 0xFFFF, 128, 0, false, false, mr);
}

TEST_CASE("ForceMoveRouteDiffFreq") {
	auto ch = MakeCharacter();
	auto mr = MakeRoute({RPG::MoveCommand{}});
	mr.move_commands.push_back({});

	ch.ForceMoveRoute(mr, 3);
	testMoveRoute(ch, false, 3, 0xFFFF, 64, 0, true, false, mr);

	// FIXME: Test original move frequency is robust to save games
	ch.CancelMoveRoute();
	testMoveRoute(ch, false, 2, 0xFFFF, 128, 0, false, false, mr);
}

TEST_CASE("CommandMove") {
	// FIXME: Requires mocked out map.
}

static void testTurn(RPG::MoveCommand::Code::Index code, int orig_dir, int dir, int face, int x = 0, int y = 0, int px = 0, int py = 0) {
	Main_Data::game_player->SetX(px);
	Main_Data::game_player->SetY(py);

	auto ch = MakeCharacter();
	ch.SetX(x);
	ch.SetY(y);
	ch.SetDirection(orig_dir);
	ch.SetSpriteDirection(orig_dir);
	auto mr = MakeRoute({{ code }});

	CAPTURE(code);
	CAPTURE(orig_dir);
	CAPTURE(dir);
	CAPTURE(face);
	CAPTURE(x);
	CAPTURE(y);
	CAPTURE(px);
	CAPTURE(py);

	ch.ForceMoveRoute(mr, 2);
	testMoveRouteDir(ch, orig_dir, orig_dir, false, 2, 0xFFFF, 0, 0, true, false, mr);

	for(int i = 1; i <= 64; ++i) {
		ForceUpdate(ch);
		testMoveRouteDir(ch, dir, face, false, 2, i, 64, 1, true, false, mr);
	}

	ForceUpdate(ch);
	testMoveRouteDir(ch, dir, face, false, 2, 65, 128, 1, false, false, mr);
}

TEST_CASE("CommandTurn") {
	const MapGuard mg;

	testTurn(RPG::MoveCommand::Code::face_up, Down, Up, Up);
	testTurn(RPG::MoveCommand::Code::face_left, Down, Left, Left);
	testTurn(RPG::MoveCommand::Code::face_right, Down, Right, Right);
	testTurn(RPG::MoveCommand::Code::face_down, Down, Down, Down);

	testTurn(RPG::MoveCommand::Code::turn_180_degree, Down, Up, Up);
	testTurn(RPG::MoveCommand::Code::turn_180_degree, Up, Down, Down);
	testTurn(RPG::MoveCommand::Code::turn_180_degree, Left, Right, Right);
	testTurn(RPG::MoveCommand::Code::turn_180_degree, Right, Left, Left);

	testTurn(RPG::MoveCommand::Code::turn_90_degree_right, Down, Left, Left);
	testTurn(RPG::MoveCommand::Code::turn_90_degree_right, Left, Up, Up);
	testTurn(RPG::MoveCommand::Code::turn_90_degree_right, Up, Right, Right);
	testTurn(RPG::MoveCommand::Code::turn_90_degree_right, Right, Down, Down);

	testTurn(RPG::MoveCommand::Code::turn_90_degree_left, Down, Right, Right);
	testTurn(RPG::MoveCommand::Code::turn_90_degree_left, Right, Up, Up);
	testTurn(RPG::MoveCommand::Code::turn_90_degree_left, Up, Left, Left);
	testTurn(RPG::MoveCommand::Code::turn_90_degree_left, Left, Down, Down);
}

TEST_CASE("CommandTurnRandom") {
	const MapGuard mg;

	for (int i = 0; i < 10; ++i) {
		auto ch = MakeCharacter();
		auto mr = MakeRoute({{ RPG::MoveCommand::Code::face_random_direction }});

		ch.ForceMoveRoute(mr, 3);
		testMoveRouteDir(ch, Down, Down, false, 3, 0xFFFF, 64, 0, true, false, mr);

		ForceUpdate(ch);
		testMoveRoute(ch, false, 3, 1, 32, 1, true, false, mr);

		REQUIRE_GE(ch.GetDirection(), Up);
		REQUIRE_LE(ch.GetDirection(), Left);
	}
}

TEST_CASE("CommandTurnHero") {
	const MapGuard mg;

	int ch_x = 8;
	int ch_y = 8;
	for (int dx = -2; dx <= 2; ++dx) {
		for (int dy = -2; dy <= 2; ++dy) {
			auto dir = Down;
			if (std::abs(dx) > std::abs(dy)) {
				dir = (dx > 0 ? Right : Left);
			} else {
				dir = (dy >= 0 ? Down : Up);
			}
			testTurn(RPG::MoveCommand::Code::face_hero, Left, dir, dir, ch_x, ch_y, ch_x + dx, ch_y + dy);
		}
	}
}

TEST_CASE("CommandWait") {
	const MapGuard mg;

	auto ch = MakeCharacter();
	auto mr = MakeRoute({{ RPG::MoveCommand::Code::wait }});

	ch.ForceMoveRoute(mr, 2);
	testMoveRouteDir(ch, Down, Down, false, 2, 0xFFFF, 0, 0, true, false, mr);

	for(int i = 1; i <= 84; ++i) {
		ForceUpdate(ch);
		testMoveRouteDir(ch, Down, Down, false, 2, i, 84, 1, true, false, mr);
	}

	ForceUpdate(ch);
	testMoveRouteDir(ch, Down, Down, false, 2, 85, 128, 1, false, false, mr);
}

TEST_CASE("CommandJump") {
	// FIXME: Requires mocked out map.
}

void testLockFacing(RPG::EventPage::AnimType at) {
	const MapGuard mg;

	auto ch = MakeCharacter();
	ch.SetAnimationType(at);
	auto mr = MakeRoute({{ RPG::MoveCommand::Code::lock_facing }});

	ch.ForceMoveRoute(mr, 2);
	testMoveRoute(ch, false, 2, 0xFFFF, 0, 0, true, false, mr);
	REQUIRE_EQ(ch.IsFacingLocked(), Game_Character::IsDirectionFixedAnimationType(at));

	ForceUpdate(ch);
	testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
	REQUIRE(ch.IsFacingLocked());

	mr = MakeRoute({{ RPG::MoveCommand::Code::unlock_facing }});
	ch.ForceMoveRoute(mr, 2);
	testMoveRoute(ch, false, 2, 0xFFFF, 128, 0, true, false, mr);
	REQUIRE(ch.IsFacingLocked());

	ForceUpdate(ch);
	testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
	REQUIRE_EQ(ch.IsFacingLocked(), Game_Character::IsDirectionFixedAnimationType(at));
}

TEST_CASE("CommandLockFacing") {
	for (int i = 0; i <= static_cast<int>(RPG::EventPage::AnimType_step_frame_fix); ++i) {
		testLockFacing(static_cast<RPG::EventPage::AnimType>(i));
	}
}

TEST_CASE("CommandSpeedChange") {
	const MapGuard mg;

	auto ch = MakeCharacter();
	auto mr = MakeRoute({{ RPG::MoveCommand::Code::increase_movement_speed }});
	const int n = 10;

	ch.SetMoveSpeed(1);
	int prev = ch.GetMoveSpeed();
	for (int i = 0; i < 10; ++i) {
		ch.ForceMoveRoute(mr, 2);
		testMoveRoute(ch, false, 2, 0xFFFF, (i == 0 ? 0 : 128), 0, true, false, mr);
		REQUIRE_EQ(ch.GetMoveSpeed(), prev);

		ForceUpdate(ch);
		testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
		REQUIRE_EQ(ch.GetMoveSpeed(), std::min(prev + 1, 6));

		prev= ch.GetMoveSpeed();
	}

	mr = MakeRoute({{ RPG::MoveCommand::Code::decrease_movement_speed }});
	for (int i = 0; i < 10; ++i) {
		ch.ForceMoveRoute(mr, 2);
		testMoveRoute(ch, false, 2, 0xFFFF, 128, 0, true, false, mr);
		REQUIRE_EQ(ch.GetMoveSpeed(), prev);

		ForceUpdate(ch);
		testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
		REQUIRE_EQ(ch.GetMoveSpeed(), std::max(prev - 1, 1));

		prev= ch.GetMoveSpeed();
	}
}

TEST_CASE("CommandFreqChange") {
	const MapGuard mg;

	auto ch = MakeCharacter();
	auto mr = MakeRoute({{ RPG::MoveCommand::Code::increase_movement_frequence }});
	const int n = 10;

	for (int i = 1; i < 10; ++i) {
		const int freq = Utils::Clamp(i, 1, 8);

		ch.ForceMoveRoute(mr, freq);
		testMoveRoute(ch, false, freq, 0xFFFF, (i == 0 && freq == 2 ? 0 : Game_Character::GetMaxStopCountForStep(freq)), 0, true, false, mr);

		const int next_freq = Utils::Clamp(freq + 1, 1, 8);
		ForceUpdate(ch);
		// FIXME: Need another command for the frequency to not get reset when move route is done.
		//testMoveRoute(ch, false, next_freq, 0xFFFF + 1, Game_Character::GetMaxStopCountForStep(next_freq), 1, false, false, mr);

		testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);

	}

	mr = MakeRoute({{ RPG::MoveCommand::Code::decrease_movement_frequence }});

	for (int i = 1; i < 10; ++i) {
		const int freq = Utils::Clamp(i, 1, 8);

		ch.ForceMoveRoute(mr, freq);
		testMoveRoute(ch, false, freq, 0xFFFF, Game_Character::GetMaxStopCountForStep(freq), 0, true, false, mr);

		const int next_freq = Utils::Clamp(freq - 1, 1, 8);
		ForceUpdate(ch);
		// FIXME: Need another command for the frequency to not get reset when move route is done.
		//testMoveRoute(ch, false, next_freq, 0xFFFF + 1, Game_Character::GetMaxStopCountForStep(next_freq), 1, false, false, mr);
		testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
	}
}

TEST_CASE("CommandTranspChange") {
	const MapGuard mg;

	auto ch = MakeCharacter();
	auto mr = MakeRoute({{ RPG::MoveCommand::Code::increase_transp }});
	const int n = 10;

	ch.SetTransparency(0);
	int prev = ch.GetTransparency();
	for (int i = 0; i < 10; ++i) {
		ch.ForceMoveRoute(mr, 2);
		testMoveRoute(ch, false, 2, 0xFFFF, (i == 0 ? 0 : 128), 0, true, false, mr);
		REQUIRE_EQ(ch.GetTransparency(), prev);

		ForceUpdate(ch);
		testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
		REQUIRE_EQ(ch.GetTransparency(), std::min(prev + 1, 7));

		prev = ch.GetTransparency();
	}

	mr = MakeRoute({{ RPG::MoveCommand::Code::decrease_transp }});
	for (int i = 0; i < 10; ++i) {
		ch.ForceMoveRoute(mr, 2);
		testMoveRoute(ch, false, 2, 0xFFFF, 128, 0, true, false, mr);
		REQUIRE_EQ(ch.GetTransparency(), prev);

		ForceUpdate(ch);
		testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
		REQUIRE_EQ(ch.GetTransparency(), std::max(prev - 1, 0));

		prev = ch.GetTransparency();
	}
}

TEST_CASE("CommandThrough") {
	const MapGuard mg;

	auto ch = MakeCharacter();
	auto mr = MakeRoute({{ RPG::MoveCommand::Code::walk_everywhere_on }});

	auto testoff = [&]() {
		REQUIRE(!ch.GetThrough());
		ch.SetThrough(true);
		REQUIRE(ch.GetThrough());
		ch.ResetThrough();
		REQUIRE(!ch.GetThrough());
	};

	auto teston = [&]() {
		REQUIRE(ch.GetThrough());
		ch.SetThrough(false);
		REQUIRE(!ch.GetThrough());
		ch.ResetThrough();
		REQUIRE(ch.GetThrough());
	};

	ch.ForceMoveRoute(mr, 2);
	testMoveRoute(ch, false, 2, 0xFFFF, 0, 0, true, false, mr);
	testoff();

	ForceUpdate(ch);
	testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
	teston();

	mr = MakeRoute({{ RPG::MoveCommand::Code::walk_everywhere_off }});
	ch.ForceMoveRoute(mr, 2);
	testMoveRoute(ch, false, 2, 0xFFFF, 128, 0, true, false, mr);
	teston();

	ForceUpdate(ch);
	testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
	testoff();

	ch.SetThrough(true);

	mr = MakeRoute({{ RPG::MoveCommand::Code::walk_everywhere_off }});
	ch.ForceMoveRoute(mr, 2);
	testMoveRoute(ch, false, 2, 0xFFFF, 128, 0, true, false, mr);
	REQUIRE(ch.GetThrough());

	ForceUpdate(ch);
	testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
	testoff();

	ch.SetThrough(true);
	mr = MakeRoute({{ RPG::MoveCommand::Code::walk_everywhere_on }});
	ch.ForceMoveRoute(mr, 2);
	REQUIRE(ch.GetThrough());

	ForceUpdate(ch);
	testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
	teston();
}

TEST_CASE("CommandStopAnimation") {
	const MapGuard mg;

	auto ch = MakeCharacter();
	auto mr = MakeRoute({{ RPG::MoveCommand::Code::stop_animation }});

	ch.ForceMoveRoute(mr, 2);
	testMoveRoute(ch, false, 2, 0xFFFF, 0, 0, true, false, mr);
	REQUIRE(!ch.IsAnimPaused());

	ForceUpdate(ch);
	testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
	REQUIRE(ch.IsAnimPaused());

	mr = MakeRoute({{ RPG::MoveCommand::Code::start_animation }});
	ch.ForceMoveRoute(mr, 2);
	testMoveRoute(ch, false, 2, 0xFFFF, 128, 0, true, false, mr);
	REQUIRE(ch.IsAnimPaused());

	ForceUpdate(ch);
	testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
	REQUIRE(!ch.IsAnimPaused());
}

TEST_CASE("CommandSwitchToggle") {
	const MapGuard mg;

	auto ch = MakeCharacter();
	auto mr = MakeRoute({{ RPG::MoveCommand::Code::switch_on, "", 3 }});

	REQUIRE_NE(Main_Data::game_switches, nullptr);

	ch.ForceMoveRoute(mr, 2);
	testMoveRoute(ch, false, 2, 0xFFFF, 0, 0, true, false, mr);
	REQUIRE(!Main_Data::game_switches->Get(3));

	ForceUpdate(ch);
	testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
	REQUIRE(Main_Data::game_switches->Get(3));

	mr = MakeRoute({{ RPG::MoveCommand::Code::switch_off, "", 3 }});
	ch.ForceMoveRoute(mr, 2);
	testMoveRoute(ch, false, 2, 0xFFFF, 128, 0, true, false, mr);
	REQUIRE(Main_Data::game_switches->Get(3));

	ForceUpdate(ch);
	testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
	REQUIRE(!Main_Data::game_switches->Get(3));
}

TEST_CASE("CommandChangeGraphic") {
	const MapGuard mg;

	auto ch = MakeCharacter();
	auto mr = MakeRoute({{ RPG::MoveCommand::Code::change_graphic, "x", 3 }});

	ch.ForceMoveRoute(mr, 2);
	testMoveRoute(ch, false, 2, 0xFFFF, 0, 0, true, false, mr);
	REQUIRE_EQ(ch.GetSpriteName(), "");
	REQUIRE_EQ(ch.GetSpriteIndex(), 0);

	ForceUpdate(ch);
	testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
	REQUIRE_EQ(ch.GetSpriteName(), "x");
	REQUIRE_EQ(ch.GetSpriteIndex(), 3);
}

TEST_CASE("CommandPlaySound") {
	const MapGuard mg;

	auto ch = MakeCharacter();
	auto mr = MakeRoute({{ RPG::MoveCommand::Code::play_sound_effect, "", 100, 100, 100 }});

	ch.ForceMoveRoute(mr, 2);
	testMoveRoute(ch, false, 2, 0xFFFF, 0, 0, true, false, mr);

	ForceUpdate(ch);
	testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);

	// FIXME: Check mocked audio subsystem?
}

TEST_SUITE_END();
