#include "game_switches.h"
#include "doctest.h"

TEST_SUITE_BEGIN("Switches");

constexpr int max_switches = 5;

static Game_Switches make() {
	lcf::Data::switches.resize(max_switches);
	Game_Switches s;
	s.SetLowerLimit(max_switches);
	s.SetWarning(0);
	return s;
}

TEST_CASE("Set") {
	constexpr int n = max_switches * 2;
	auto s = make();

	REQUIRE_FALSE(s.Get(0));
	REQUIRE_FALSE(s.Set(0, true));
	REQUIRE_FALSE(s.Get(0));

	REQUIRE_FALSE(s.Get(-1));
	REQUIRE_FALSE(s.Set(-1, true));
	REQUIRE_FALSE(s.Get(-1));
	for (int i = 1; i <= n; ++i) {
		REQUIRE_FALSE(s.Get(i));
		REQUIRE(s.Set(i, true));
		REQUIRE(s.Get(i));
	}
	REQUIRE_FALSE(s.Get(n + 1));

	for (int i = 1; i <= n; ++i) {
		REQUIRE(s.Get(i));
		REQUIRE_FALSE(s.Set(i, false));
		REQUIRE_FALSE(s.Get(i));
	}
	REQUIRE_FALSE(s.Get(n + 1));
}

TEST_CASE("Flip") {
	constexpr int n = max_switches * 2;
	auto s = make();

	REQUIRE_FALSE(s.Get(0));
	REQUIRE_FALSE(s.Flip(0));
	REQUIRE_FALSE(s.Get(0));

	REQUIRE_FALSE(s.Get(-1));
	REQUIRE_FALSE(s.Flip(-1));
	REQUIRE_FALSE(s.Get(-1));
	for (int i = 1; i <= n; ++i) {
		REQUIRE_FALSE(s.Get(i));
		REQUIRE(s.Flip(i));
		REQUIRE(s.Get(i));
	}
	REQUIRE_FALSE(s.Get(n + 1));

	for (int i = 1; i <= n; ++i) {
		REQUIRE(s.Get(i));
		REQUIRE_FALSE(s.Flip(i));
		REQUIRE_FALSE(s.Get(i));
	}
	REQUIRE_FALSE(s.Get(n + 1));
}

TEST_CASE("SetRange") {
	constexpr int n = max_switches * 2;
	auto s = make();
	for (int i = 0; i < n; ++i) {
		REQUIRE_FALSE(s.Get(i));
	}

	s.SetRange(-1, n, true);

	REQUIRE_FALSE(s.Get(-1));
	REQUIRE_FALSE(s.Get(0));
	for (int i = 1; i <= n; ++i) {
		REQUIRE(s.Get(i));
	}
	REQUIRE_FALSE(s.Get(n + 1));

	s.SetRange(2, n-2, false);

	REQUIRE_FALSE(s.Get(-1));
	REQUIRE_FALSE(s.Get(0));
	REQUIRE(s.Get(1));
	for (int i = 2; i <= n-2; ++i) {
		REQUIRE_FALSE(s.Get(i));
	}
	REQUIRE(s.Get(n - 1));
	REQUIRE(s.Get(n));
	REQUIRE_FALSE(s.Get(n + 1));
}

TEST_CASE("FlipRange") {
	constexpr int n = max_switches * 2;
	auto s = make();
	for (int i = 0; i < n; ++i) {
		REQUIRE_FALSE(s.Get(i));
	}

	s.FlipRange(-1, n);

	REQUIRE_FALSE(s.Get(-1));
	REQUIRE_FALSE(s.Get(0));
	for (int i = 1; i <= n; ++i) {
		REQUIRE(s.Get(i));
	}
	REQUIRE_FALSE(s.Get(n + 1));

	s.FlipRange(2, n-2);

	REQUIRE_FALSE(s.Get(-1));
	REQUIRE_FALSE(s.Get(0));
	REQUIRE(s.Get(1));
	for (int i = 2; i <= n-2; ++i) {
		REQUIRE_FALSE(s.Get(i));
	}
	REQUIRE(s.Get(n - 1));
	REQUIRE(s.Get(n));
	REQUIRE_FALSE(s.Get(n + 1));
}

TEST_CASE("GetSize") {
	auto s = make();
	REQUIRE_EQ(s.GetSizeWithLimit(), max_switches);
	REQUIRE_EQ(s.GetSize(), 0);
}

TEST_CASE("IsValid") {
	auto s = make();
	REQUIRE_FALSE(s.IsValid(-1));
	REQUIRE_FALSE(s.IsValid(0));

	for (int i = 1; i <= max_switches; ++i) {
		REQUIRE(s.IsValid(i));
	}

	REQUIRE_FALSE(s.IsValid(max_switches + 1));
}

TEST_SUITE_END();
