#include "doctest.h"
#include "options.h"
#include "game_map.h"
#include "game_vehicle.h"
#include "main_data.h"
#include "game_switches.h"
#include <climits>
#include <initializer_list>

#include "mock_game.h"

static constexpr auto map_id = MockMap::ePassBlock20x15;

static_assert(Game_Character::GetStationaryAnimFrames(1) == 12, "AnimFrames broken");
static_assert(Game_Character::GetStationaryAnimFrames(2) == 10, "AnimFrames broken");
static_assert(Game_Character::GetStationaryAnimFrames(3) == 8, "AnimFrames broken");
static_assert(Game_Character::GetStationaryAnimFrames(4) == 6, "AnimFrames broken");
static_assert(Game_Character::GetStationaryAnimFrames(5) == 5, "AnimFrames broken");
static_assert(Game_Character::GetStationaryAnimFrames(6) == 4, "AnimFrames broken");

static_assert(Game_Character::GetContinuousAnimFrames(1) == 16, "AnimFrames broken");
static_assert(Game_Character::GetContinuousAnimFrames(2) == 12, "AnimFrames broken");
static_assert(Game_Character::GetContinuousAnimFrames(3) == 10, "AnimFrames broken");
static_assert(Game_Character::GetContinuousAnimFrames(4) == 8, "AnimFrames broken");
static_assert(Game_Character::GetContinuousAnimFrames(5) == 7, "AnimFrames broken");
static_assert(Game_Character::GetContinuousAnimFrames(6) == 6, "AnimFrames broken");

static_assert(Game_Character::GetSpinAnimFrames(1) == 24, "AnimFrames broken");
static_assert(Game_Character::GetSpinAnimFrames(2) == 16, "AnimFrames broken");
static_assert(Game_Character::GetSpinAnimFrames(3) == 12, "AnimFrames broken");
static_assert(Game_Character::GetSpinAnimFrames(4) == 8, "AnimFrames broken");
static_assert(Game_Character::GetSpinAnimFrames(5) == 6, "AnimFrames broken");
static_assert(Game_Character::GetSpinAnimFrames(6) == 4, "AnimFrames broken");

TEST_SUITE_BEGIN("Game_Character_Anim");

static void testChar( const Game_Character& ch, int anim_count, int anim_frame)
{
	CAPTURE(anim_count);
	CAPTURE(anim_frame);

	REQUIRE_EQ(ch.GetAnimCount(), anim_count);
	REQUIRE_EQ(ch.GetAnimFrame(), anim_frame);
}

static auto& GetEvent(AnimType at, int speed) {
	auto& ch = *MockGame::GetEvent(1);
	ch.SetAnimationType(at);
	ch.SetMoveSpeed(speed);
	ch.SetX(8);
	ch.SetY(8);
	return ch;
}

static void testAnimLimit(Game_Event& ch) {
	const auto limit = Game_Character::GetStationaryAnimFrames(ch.GetMoveSpeed());

	// For events not moving, anim count ticks up to the limit and then freezes until
	// the character moves.
	for (int i = 0; i < 255; ++i) {
		testChar(ch, std::min(i, limit - 1), 1);
		ForceUpdate(ch);
	}
}

static void testAnimContinuous(Game_Event& ch) {
	const auto limit = Game_Character::GetContinuousAnimFrames(ch.GetMoveSpeed());

	// Continuous always animates
	for (int i = 0; i < 255; ++i) {
		auto count = i % limit;
		auto frame = ((i / limit) + 1)  % 4;
		testChar(ch, count, frame);
		ForceUpdate(ch);
	}
}

static void testAnimSpin(Game_Event& ch, bool move = false, bool jump = false) {
	const auto limit = Game_Character::GetSpinAnimFrames(ch.GetMoveSpeed());

	// Spin always animates, even while moving or jumping
	for (int i = 0; i < 255; ++i) {
		if (move && ch.IsStopping()) {
			ch.SetX(8);
			ch.Move(Right);
		}
		if (jump && ch.IsStopping()) {
			ch.Jump(ch.GetX(), ch.GetY());
		}
		auto count = i % limit;
		auto frame = 1;
		auto dir = ((i / limit) + 2) % 4;
		testChar(ch, count, frame);
		REQUIRE_EQ(ch.GetFacing(), dir);
		ForceUpdate(ch);
	}
}

template <typename T>
static void testAnimFixed(T& ch) {
	// Fixed animation scenarios never change
	for (int i = 0; i < 8; ++i) {
		testChar(ch, 0, 1);
		ForceUpdate(ch);
	}
}

template <typename T>
static void testAnimJump(T& ch) {
	testChar(ch, 0, 1);
	ForceUpdate(ch);

	for (int i = 0; i < 255; ++i) {
		if (ch.IsJumping()) {
			// Animation gets reset every frame of an active jump
			testChar(ch, 0, 1);
		} else {
			// Animation ticks up normally on the frame when the jump finishes and the flag is cleared.
			testChar(ch, ch.IsAnimated(), 1);
		}
		if (ch.IsStopping()) {
			ch.Jump(ch.GetX(), ch.GetY());
		}
		ForceUpdate(ch);
	}
}

static void testStanding(AnimType at, int speed) {
	const MockGame mg(map_id);

	auto& ch = GetEvent(at, speed);

	CAPTURE(at);
	CAPTURE(speed);

	// Baseline behavior when event is not moving
	if (ch.IsSpinning()) {
		testAnimSpin(ch, false, false);
	} else if (ch.IsContinuous()) {
		testAnimContinuous(ch);
	} else if (ch.IsAnimated()) {
		testAnimLimit(ch);
	} else {
		testAnimFixed(ch);
	}
}

static void testPaused(AnimType at, int speed) {
	const MockGame mg(map_id);

	auto& ch = GetEvent(at, speed);
	ch.SetAnimPaused(true);
	REQUIRE(!ch.IsAnimated());

	// When event has paused flag
	if (at == lcf::rpg::EventPage::AnimType_spin) {
		// Continues spinning even when paused flag is set
		testAnimSpin(ch, false, false);
	} else {
		testAnimFixed(ch);
	}
}

static void testJumping(AnimType at, int speed) {
	const MockGame mg(map_id);

	auto& ch = GetEvent(at, speed);

	CAPTURE(at);
	CAPTURE(speed);

	// When event is jumping
	if (at == lcf::rpg::EventPage::AnimType_spin) {
		// Continues spinning even when jumping
		testAnimSpin(ch, false, true);
	} else {
		testAnimJump(ch);
	}
}

static void testMoving(AnimType at, int speed) {
	const MockGame mg(map_id);

	auto& ch = GetEvent(at, speed);

	const auto limit = Game_Character::GetStationaryAnimFrames(ch.GetMoveSpeed());

	CAPTURE(at);
	CAPTURE(speed);

	// When event is moving
	if (at == lcf::rpg::EventPage::AnimType_spin) {
		// Continues spinning even when moving
		testAnimSpin(ch, true, false);
		return;
	}

	for (int i = 0; i < 255; ++i) {
		// HACK To make stop count increment to 0 each frame to emulate movement
		// FIXME: Verify this matches movement
		ch.SetStopCount(-1);

		if (ch.IsAnimated()) {
			auto count = i % limit;
			auto frame = ((i / limit) + 1)  % 4;
			testChar(ch, count, frame);
		} else {
			testChar(ch, 0, 1);
		}
		ForceUpdate(ch);
	}
}

static void testCenterStep(AnimType at, int speed, int frame) {
	const MockGame mg(map_id);

	auto& ch = GetEvent(at, speed);

	const auto slimit = Game_Character::GetStationaryAnimFrames(ch.GetMoveSpeed());
	const auto climit = Game_Character::GetContinuousAnimFrames(ch.GetMoveSpeed());

	CAPTURE(at);
	CAPTURE(speed);
	CAPTURE(frame);

	if (at == lcf::rpg::EventPage::AnimType_spin) {
		// Spins regardless of frame
		testAnimSpin(ch);
		return;
	}

	// When event has left or right step, they will always animate back to center step regardless of movement or not
	if (frame == lcf::rpg::EventPage::Frame_left || frame == lcf::rpg::EventPage::Frame_right) {
		for (int i = 0; i < climit; ++i) {
			testChar(ch, ch.IsAnimated() ? i : 0, frame);
			ForceUpdate(ch);
		}
		if (ch.IsAnimated()) {
			frame = (frame + 1) % 4;
		}
	}

	for (int i = 0; i < slimit; ++i) {
		testChar(ch, ch.IsAnimated() ? i : 0, frame);
		ForceUpdate(ch);
	}

	if (ch.IsAnimated()) {
		if (ch.IsContinuous()) {
			testChar(ch, slimit, frame);
		} else {
			testChar(ch, slimit - 1, frame);
		}
	} else {
		testChar(ch, 0, frame);
	}
}

TEST_CASE("Flags") {
	const MockGame mg(map_id);

	auto& ch = GetEvent(lcf::rpg::EventPage::AnimType_non_continuous, 4);

	ch.SetAnimationType(lcf::rpg::EventPage::AnimType_non_continuous);

	REQUIRE(ch.IsAnimated());
	REQUIRE(!ch.IsContinuous());
	REQUIRE(!ch.IsSpinning());

	ch.SetAnimationType(lcf::rpg::EventPage::AnimType_continuous);

	REQUIRE(ch.IsAnimated());
	REQUIRE(ch.IsContinuous());
	REQUIRE(!ch.IsSpinning());

	ch.SetAnimationType(lcf::rpg::EventPage::AnimType_fixed_non_continuous);

	REQUIRE(ch.IsAnimated());
	REQUIRE(!ch.IsContinuous());
	REQUIRE(!ch.IsSpinning());

	ch.SetAnimationType(lcf::rpg::EventPage::AnimType_fixed_continuous);

	REQUIRE(ch.IsAnimated());
	REQUIRE(ch.IsContinuous());
	REQUIRE(!ch.IsSpinning());

	ch.SetAnimationType(lcf::rpg::EventPage::AnimType_fixed_graphic);

	REQUIRE(!ch.IsAnimated());
	REQUIRE(!ch.IsContinuous());
	REQUIRE(!ch.IsSpinning());

	ch.SetAnimationType(lcf::rpg::EventPage::AnimType_spin);

	REQUIRE(ch.IsAnimated());
	REQUIRE(!ch.IsContinuous());
	REQUIRE(ch.IsSpinning());

	ch.SetAnimationType(lcf::rpg::EventPage::AnimType_step_frame_fix);

	REQUIRE(!ch.IsAnimated());
	REQUIRE(!ch.IsContinuous());
	REQUIRE(!ch.IsSpinning());
}

TEST_CASE("Standing") {
	for (int speed = 1; speed <= 6; ++speed) {
		for (int ati = 0; ati < static_cast<int>(lcf::rpg::EventPage::AnimType_step_frame_fix); ++ati) {
			auto at = static_cast<lcf::rpg::EventPage::AnimType>(ati);
			testStanding(at, speed);
		}
	}
}

TEST_CASE("Moving") {
	for (int speed = 1; speed <= 6; ++speed) {
		for (int ati = 0; ati < static_cast<int>(lcf::rpg::EventPage::AnimType_step_frame_fix); ++ati) {
			auto at = static_cast<lcf::rpg::EventPage::AnimType>(ati);
			testMoving(at, speed);
		}
	}
}

TEST_CASE("Paused") {
	for (int speed = 1; speed <= 6; ++speed) {
		for (int ati = 0; ati < static_cast<int>(lcf::rpg::EventPage::AnimType_step_frame_fix); ++ati) {
			auto at = static_cast<lcf::rpg::EventPage::AnimType>(ati);
			testPaused(at, speed);
		}
	}
}

TEST_CASE("Jumping") {
	for (int speed = 1; speed <= 6; ++speed) {
		for (int ati = 0; ati < static_cast<int>(lcf::rpg::EventPage::AnimType_step_frame_fix); ++ati) {
			auto at = static_cast<lcf::rpg::EventPage::AnimType>(ati);
			testJumping(at, speed);
		}
	}
}

TEST_CASE("ResetStep") {
	for (int speed = 1; speed <= 6; ++speed) {
		for (int ati = 0; ati < static_cast<int>(lcf::rpg::EventPage::AnimType_step_frame_fix); ++ati) {
			auto at = static_cast<lcf::rpg::EventPage::AnimType>(ati);
			testCenterStep(at, speed, lcf::rpg::EventPage::Frame_middle);
		}
	}
}

TEST_CASE("SpinFacingLocked") {
	const MockGame mg(map_id);

	auto& ch = GetEvent(lcf::rpg::EventPage::AnimType_spin, 4);
	ch.SetFacingLocked(true);

	// Continues spinning even with facing locked flag set
	testAnimSpin(ch, false, false);
}

static void testBoat(Game_Vehicle::Type vt, AnimType at, int speed, bool move, bool jump, bool flying) {
	const MockGame mg(map_id);

	auto& ch = *mg.GetVehicle(vt);
	if (flying) {
		ch.SetFlying(flying);
	}

	auto limit = move ? 12 : 16;

	CAPTURE(vt);
	CAPTURE(at);
	CAPTURE(speed);

	if (vt == Game_Vehicle::Airship && !ch.IsFlying()) {
		testAnimFixed(ch);
		return;
	}

	if (jump) {
		testAnimJump(ch);
		return;
	}

	for (int i = 0; i < 255; ++i) {
		if (move) {
			// HACK To make stop count increment to 0 each frame to emulate movement
			// FIXME: Verify this matches movement
			ch.SetStopCount(-1);
		}

		auto count = i % limit;
		auto frame = ((i / limit) + 1)  % 4;
		testChar(ch, count, frame);
		ForceUpdate(ch);
	}
}

TEST_CASE("BoatShipStanding") {
	for (int speed = 1; speed <= 6; ++speed) {
		for (int ati = 0; ati < static_cast<int>(lcf::rpg::EventPage::AnimType_step_frame_fix); ++ati) {
			auto at = static_cast<lcf::rpg::EventPage::AnimType>(ati);
			testBoat(Game_Vehicle::Boat, at, speed, false, false, false);
			testBoat(Game_Vehicle::Ship, at, speed, false, false, false);
			testBoat(Game_Vehicle::Airship, at, speed, false, false, false);

			testBoat(Game_Vehicle::Boat, at, speed, false, false, true);
			testBoat(Game_Vehicle::Ship, at, speed, false, false, true);
			testBoat(Game_Vehicle::Airship, at, speed, false, false, true);
		}
	}
}

TEST_CASE("BoatShipMove") {
	for (int speed = 1; speed <= 6; ++speed) {
		for (int ati = 0; ati < static_cast<int>(lcf::rpg::EventPage::AnimType_step_frame_fix); ++ati) {
			auto at = static_cast<lcf::rpg::EventPage::AnimType>(ati);
			testBoat(Game_Vehicle::Boat, at, speed, true, false, false);
			testBoat(Game_Vehicle::Ship, at, speed, true, false, false);
			testBoat(Game_Vehicle::Airship, at, speed, true, false, false);

			testBoat(Game_Vehicle::Boat, at, speed, true, false, true);
			testBoat(Game_Vehicle::Ship, at, speed, true, false, true);
			testBoat(Game_Vehicle::Airship, at, speed, true, false, true);
		}
	}
}

TEST_CASE("BoatShipJump") {
	for (int speed = 1; speed <= 6; ++speed) {
		for (int ati = 0; ati < static_cast<int>(lcf::rpg::EventPage::AnimType_step_frame_fix); ++ati) {
			auto at = static_cast<lcf::rpg::EventPage::AnimType>(ati);
			testBoat(Game_Vehicle::Boat, at, speed, false, true, false);
			testBoat(Game_Vehicle::Ship, at, speed, false, true, false);
			testBoat(Game_Vehicle::Airship, at, speed, false, true, false);

			testBoat(Game_Vehicle::Boat, at, speed, false, true, true);
			testBoat(Game_Vehicle::Ship, at, speed, false, true, true);
			testBoat(Game_Vehicle::Airship, at, speed, false, true, true);
		}
	}
}

TEST_SUITE_END();
