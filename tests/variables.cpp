#include "game_variables.h"
#include "doctest.h"

TEST_SUITE_BEGIN("Variables");

constexpr int max_vars = 5;
constexpr int minval = Game_Variables::min_2k3;
constexpr int maxval = Game_Variables::max_2k3;

static Game_Variables make() {
	Data::variables.resize(max_vars);
	Game_Variables v(minval, maxval);
	v.SetWarning(0);
	return v;
}

TEST_CASE("Set") {
	constexpr int n = max_vars * 2;
	auto s = make();

	REQUIRE_EQ(s.Get(0), 0);
	REQUIRE_EQ(s.Set(0, 42), 0);
	REQUIRE_EQ(s.Get(0), 0);

	REQUIRE_EQ(s.Get(-1), 0);
	REQUIRE_EQ(s.Set(-1, 42), 0);
	REQUIRE_EQ(s.Get(-1), 0);
	for (int i = 1; i <= n; ++i) {
		REQUIRE_EQ(s.Get(i), 0);
		REQUIRE_EQ(s.Set(i, 42 + i), 42 + i);
		REQUIRE_EQ(s.Get(i), 42 + i);
	}
	REQUIRE_EQ(s.Get(n + 1), 0);
}

TEST_CASE("SetRange") {
	constexpr int n = max_vars * 2;
	auto s = make();
	for (int i = 0; i < n; ++i) {
		REQUIRE_EQ(s.Get(i), 0);
	}

	s.SetRange(-1, n, 42);

	REQUIRE_EQ(s.Get(-1), 0);
	REQUIRE_EQ(s.Get(0), 0);
	for (int i = 1; i <= n; ++i) {
		REQUIRE_EQ(s.Get(i), 42);
	}
	REQUIRE_EQ(s.Get(n + 1), 0);

	s.SetRange(2, n-2, 20);

	REQUIRE_EQ(s.Get(-1), 0);
	REQUIRE_EQ(s.Get(0), 0);
	REQUIRE_EQ(s.Get(1), 42);
	for (int i = 2; i <= n-2; ++i) {
		REQUIRE_EQ(s.Get(i), 20);
	}
	REQUIRE_EQ(s.Get(n - 1), 42);
	REQUIRE_EQ(s.Get(n), 42);
	REQUIRE_EQ(s.Get(n + 1), 0);
}

TEST_CASE("Add") {
	auto s = make();

	REQUIRE_EQ(s.Get(1), 0);

	REQUIRE_EQ(s.Set(1, 20), 20);
	REQUIRE_EQ(s.Get(1), 20);

	REQUIRE_EQ(s.Add(1, 22), 42);
	REQUIRE_EQ(s.Get(1), 42);

	s.SetRange(1, 2, 20);
	REQUIRE_EQ(s.Get(1), 20);
	REQUIRE_EQ(s.Get(2), 20);

	s.AddRange(1, 2, 22);
	REQUIRE_EQ(s.Get(1), 42);
	REQUIRE_EQ(s.Get(2), 42);
}

TEST_CASE("Sub") {
	auto s = make();

	REQUIRE_EQ(s.Get(1), 0);

	REQUIRE_EQ(s.Set(1, 20), 20);
	REQUIRE_EQ(s.Get(1), 20);

	REQUIRE_EQ(s.Sub(1, 22), -2);
	REQUIRE_EQ(s.Get(1), -2);

	s.SetRange(1, 2, 20);
	REQUIRE_EQ(s.Get(1), 20);
	REQUIRE_EQ(s.Get(2), 20);

	s.SubRange(1, 2, 22);
	REQUIRE_EQ(s.Get(1), -2);
	REQUIRE_EQ(s.Get(2), -2);
}

TEST_CASE("Mult") {
	auto s = make();

	REQUIRE_EQ(s.Get(1), 0);

	REQUIRE_EQ(s.Set(1, 20), 20);
	REQUIRE_EQ(s.Get(1), 20);

	REQUIRE_EQ(s.Mult(1, 22), 440);
	REQUIRE_EQ(s.Get(1), 440);

	s.SetRange(1, 2, 20);
	REQUIRE_EQ(s.Get(1), 20);
	REQUIRE_EQ(s.Get(2), 20);

	s.MultRange(1, 2, 22);
	REQUIRE_EQ(s.Get(1), 440);
	REQUIRE_EQ(s.Get(2), 440);
}

TEST_CASE("Div") {
	auto s = make();

	REQUIRE_EQ(s.Get(1), 0);

	REQUIRE_EQ(s.Set(1, 20), 20);
	REQUIRE_EQ(s.Get(1), 20);

	REQUIRE_EQ(s.Div(1, 2), 10);
	REQUIRE_EQ(s.Get(1), 10);

	REQUIRE_EQ(s.Div(1, 0), 10);
	REQUIRE_EQ(s.Get(1), 10);

	s.SetRange(1, 2, 20);
	REQUIRE_EQ(s.Get(1), 20);
	REQUIRE_EQ(s.Get(2), 20);

	s.DivRange(1, 2, 2);
	REQUIRE_EQ(s.Get(1), 10);
	REQUIRE_EQ(s.Get(2), 10);

	s.DivRange(1, 2, 0);
	REQUIRE_EQ(s.Get(1), 10);
	REQUIRE_EQ(s.Get(2), 10);
}

TEST_CASE("Mod") {
	auto s = make();

	REQUIRE_EQ(s.Get(1), 0);

	REQUIRE_EQ(s.Set(1, 8), 8);
	REQUIRE_EQ(s.Get(1), 8);

	REQUIRE_EQ(s.Mod(1, 5), 3);
	REQUIRE_EQ(s.Get(1), 3);

	REQUIRE_EQ(s.Mod(1, 0), 0);
	REQUIRE_EQ(s.Get(1), 0);

	s.SetRange(1, 2, 8);
	REQUIRE_EQ(s.Get(1), 8);
	REQUIRE_EQ(s.Get(2), 8);

	s.ModRange(1, 2, 5);
	REQUIRE_EQ(s.Get(1), 3);
	REQUIRE_EQ(s.Get(2), 3);

	s.ModRange(1, 2, 0);
	REQUIRE_EQ(s.Get(1), 0);
	REQUIRE_EQ(s.Get(2), 0);
}

TEST_SUITE_END();
