#include "doctest.h"
#include "options.h"
#include "game_map.h"
#include "game_vehicle.h"
#include "main_data.h"
#include "game_switches.h"
#include <climits>
#include <initializer_list>

#include "test_move_route.h"

TEST_SUITE_BEGIN("Game_Character_Anim");

constexpr int spin_limits[] = { 23, 14, 11, 7, 5, 3 };
constexpr int stationary_limits[] = { 11, 9, 7, 5, 4, 3 };
constexpr int continuous_limits[] = { 15, 11, 9, 7, 6, 5 };

static void testChar( const Game_Character& ch, int anim_count, int anim_frame)
{
	CAPTURE(anim_count);
	CAPTURE(anim_frame);

	REQUIRE_EQ(ch.GetAnimCount(), anim_count);
	REQUIRE_EQ(ch.GetAnimFrame(), anim_frame);
}

static auto MakeEvent(AnimType at, int speed) {
	Game_Event ch(1, &Game_Map::GetMap().events[0]);
	ch.SetAnimationType(at);
	ch.SetMoveSpeed(speed);
	ch.SetX(8);
	ch.SetY(8);
	return ch;
}

static void testAnimLimit(Game_Event& ch) {
	const auto limit = stationary_limits[ch.GetMoveSpeed() - 1];

	for (int i = 0; i < 255; ++i) {
		testChar(ch, std::min(i, limit), 1);
		ForceUpdate(ch);
	}
}

static void testAnimContinuous(Game_Event& ch) {
	const auto limit = continuous_limits[ch.GetMoveSpeed() - 1] + 1;

	for (int i = 0; i < 255; ++i) {
		auto count = i % limit;
		auto frame = ((i / limit) + 1)  % 4;
		testChar(ch, count, frame);
		ForceUpdate(ch);
	}
}

static void testAnimSpin(Game_Event& ch) {
	const auto limit = spin_limits[ch.GetMoveSpeed() - 1] + 1;

	for (int i = 0; i < 255; ++i) {
		auto count = i % limit;
		auto frame = 1;
		auto dir = ((i / limit) + 2) % 4;
		testChar(ch, count, frame);
		REQUIRE_EQ(ch.GetSpriteDirection(), dir);
		ForceUpdate(ch);
	}
}

static void testAnimFixed(Game_Event& ch) {
	for (int i = 0; i < 8; ++i) {
		testChar(ch, 0, 1);
		ForceUpdate(ch);
	}
}

static void testAnimJump(Game_Event& ch) {
	testChar(ch, 0, 1);
	ForceUpdate(ch);

	for (int i = 0; i < 255; ++i) {
		if (ch.IsJumping()) {
			testChar(ch, 0, 1);
		} else {
			testChar(ch, ch.IsAnimated(), 1);
		}
		if (ch.IsStopping()) {
			ch.Jump(ch.GetX(), ch.GetY());
		}
		ForceUpdate(ch);
	}
}

static void testStanding(AnimType at, int speed) {
	const MapGuard mg;

	auto ch = MakeEvent(at, speed);

	CAPTURE(at);
	CAPTURE(speed);

	if (ch.IsSpinning()) {
		testAnimSpin(ch);
	} else if (ch.IsContinuous()) {
		testAnimContinuous(ch);
	} else if (ch.IsAnimated()) {
		testAnimLimit(ch);
	} else {
		testAnimFixed(ch);
	}
}

static void testPaused(AnimType at, int speed) {
	const MapGuard mg;

	auto ch = MakeEvent(at, speed);
	ch.SetAnimPaused(true);
	REQUIRE(!ch.IsAnimated());

	if (at == lcf::rpg::EventPage::AnimType_spin) {
		testAnimSpin(ch);
	} else {
		testAnimFixed(ch);
	}
}

static void testJumping(AnimType at, int speed) {
	const MapGuard mg;

	auto ch = MakeEvent(at, speed);

	CAPTURE(at);
	CAPTURE(speed);

	if (at == lcf::rpg::EventPage::AnimType_spin) {
		testAnimSpin(ch);
	} else {
		testAnimJump(ch);
	}
}

static void testMoving(AnimType at, int speed) {
	const MapGuard mg;

	auto ch = MakeEvent(at, speed);

	const auto limit = stationary_limits[ch.GetMoveSpeed() - 1] + 1;

	CAPTURE(at);
	CAPTURE(speed);

	if (at == lcf::rpg::EventPage::AnimType_spin) {
		testAnimSpin(ch);
		return;
	}

	for (int i = 0; i < 255; ++i) {
		// HACK To make stop count 0 each time to emulate movement
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
	const MapGuard mg;

	auto ch = MakeEvent(at, speed);

	const auto slimit = stationary_limits[ch.GetMoveSpeed() - 1] + 1;
	const auto climit = continuous_limits[ch.GetMoveSpeed() - 1] + 1;

	CAPTURE(at);
	CAPTURE(speed);
	CAPTURE(frame);

	if (at == lcf::rpg::EventPage::AnimType_spin) {
		testAnimSpin(ch);
		return;
	}

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

TEST_SUITE_END();
