#include "game_variables.h"
#include "doctest.h"

TEST_SUITE_BEGIN("Variables");

constexpr int max_vars = 5;
constexpr int minval = Game_Variables::min_2k3;
constexpr int maxval = Game_Variables::max_2k3;

static Game_Variables make() {
	lcf::Data::variables.resize(max_vars);
	Game_Variables v(minval, maxval);
	v.SetLowerLimit(max_vars);
	v.SetWarning(0);
	return v;
}

TEST_CASE("GetSize") {
	auto v = make();
	REQUIRE_EQ(v.GetSizeWithLimit(), max_vars);
	REQUIRE_EQ(v.GetSize(), 0);
}

TEST_CASE("IsValid") {
	auto v = make();
	REQUIRE_FALSE(v.IsValid(-1));
	REQUIRE_FALSE(v.IsValid(0));

	for (int i = 1; i <= max_vars; ++i) {
		REQUIRE(v.IsValid(i));
	}

	REQUIRE_FALSE(v.IsValid(max_vars + 1));
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

TEST_CASE("SetArray") {
	auto s = make();

	for (int i = 0; i < max_vars * 2; ++i) {
		s.Set(i + 1, i + 1);
	}

	s.SetArray(2, 6, 4);

	REQUIRE_EQ(s.Get(1), 1);
	REQUIRE_EQ(s.Get(2), 4);
	REQUIRE_EQ(s.Get(3), 5);
	REQUIRE_EQ(s.Get(4), 6);
	REQUIRE_EQ(s.Get(5), 7);
	REQUIRE_EQ(s.Get(6), 8);
	REQUIRE_EQ(s.Get(7), 7);
	REQUIRE_EQ(s.Get(8), 8);
	REQUIRE_EQ(s.Get(9), 9);

	for (int i = 0; i < max_vars * 2; ++i) {
		s.Set(i + 1, i + 1);
	}

	s.SetArray(4, 8, 2);

	REQUIRE_EQ(s.Get(1), 1);
	REQUIRE_EQ(s.Get(2), 2);
	REQUIRE_EQ(s.Get(3), 3);
	REQUIRE_EQ(s.Get(4), 2);
	REQUIRE_EQ(s.Get(5), 3);
	REQUIRE_EQ(s.Get(6), 4);
	REQUIRE_EQ(s.Get(7), 5);
	REQUIRE_EQ(s.Get(8), 6);
	REQUIRE_EQ(s.Get(9), 9);
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

	for (int i = 0; i < 8; ++i) {
		s.Set(i + 1, i + 1);
	}

	s.AddArray(2, 5, 4);
	REQUIRE_EQ(s.Get(2), 2 + 4);
	REQUIRE_EQ(s.Get(3), 3 + 5);
	REQUIRE_EQ(s.Get(4), 4 + 6);
	REQUIRE_EQ(s.Get(5), 5 + 7);

	for (int i = 0; i < 8; ++i) {
		s.Set(i + 1, i + 1);
	}

	s.AddArray(4, 7, 2);
	REQUIRE_EQ(s.Get(4), 2 + 4);
	REQUIRE_EQ(s.Get(5), 3 + 5);
	REQUIRE_EQ(s.Get(6), (2+4) + 6); // Test write order
	REQUIRE_EQ(s.Get(7), (3+5) + 7);
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

	for (int i = 0; i < 8; ++i) {
		s.Set(i + 1, i + 1);
	}

	s.SubArray(2, 5, 4);
	REQUIRE_EQ(s.Get(2), 2 - 4);
	REQUIRE_EQ(s.Get(3), 3 - 5);
	REQUIRE_EQ(s.Get(4), 4 - 6);
	REQUIRE_EQ(s.Get(5), 5 - 7);
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

	for (int i = 0; i < 8; ++i) {
		s.Set(i + 1, i + 1);
	}

	s.MultArray(2, 5, 4);
	REQUIRE_EQ(s.Get(2), 2 * 4);
	REQUIRE_EQ(s.Get(3), 3 * 5);
	REQUIRE_EQ(s.Get(4), 4 * 6);
	REQUIRE_EQ(s.Get(5), 5 * 7);
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

	for (int i = 0; i < 8; ++i) {
		s.Set(i + 1, i + 1);
	}

	s.DivArray(4, 5, 2);
	REQUIRE_EQ(s.Get(4), 4 / 2);
	REQUIRE_EQ(s.Get(5), 5 / 3);
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

	for (int i = 0; i < 8; ++i) {
		s.Set(i + 1, i + 1);
	}

	s.ModArray(4, 5, 2);
	REQUIRE_EQ(s.Get(4), 4 % 2);
	REQUIRE_EQ(s.Get(5), 5 % 3);
}

TEST_CASE("BitOr") {
	auto s = make();

	s.Set(1, 0xF1);

	REQUIRE_EQ(s.BitOr(1, 0x3), 0xF3);
	REQUIRE_EQ(s.Get(1), 0xF3);

	s.SetRange(1, 2, 0xF1);

	s.BitOrRange(1, 2, 0x3);
	REQUIRE_EQ(s.Get(1), 0xF3);
	REQUIRE_EQ(s.Get(2), 0xF3);

	for (int i = 0; i < 8; ++i) {
		s.Set(i + 1, i + 1);
	}

	s.BitOrArray(2, 5, 4);
	REQUIRE_EQ(s.Get(2), 2 | 4);
	REQUIRE_EQ(s.Get(3), 3 | 5);
	REQUIRE_EQ(s.Get(4), 4 | 6);
	REQUIRE_EQ(s.Get(5), 5 | 7);
}

TEST_CASE("BitAnd") {
	auto s = make();

	s.Set(1, 0xF1);

	REQUIRE_EQ(s.BitAnd(1, 0x3), 0x1);
	REQUIRE_EQ(s.Get(1), 0x1);

	s.SetRange(1, 2, 0xF1);

	s.BitAndRange(1, 2, 0x3);
	REQUIRE_EQ(s.Get(1), 0x1);
	REQUIRE_EQ(s.Get(2), 0x1);

	for (int i = 0; i < 8; ++i) {
		s.Set(i + 1, i + 1);
	}

	s.BitAndArray(2, 5, 4);
	REQUIRE_EQ(s.Get(2), 2 & 4);
	REQUIRE_EQ(s.Get(3), 3 & 5);
	REQUIRE_EQ(s.Get(4), 4 & 6);
	REQUIRE_EQ(s.Get(5), 5 & 7);
}

TEST_CASE("BitXor") {
	auto s = make();

	s.Set(1, 0xF1);

	REQUIRE_EQ(s.BitXor(1, 0xF2), 0x3);
	REQUIRE_EQ(s.Get(1), 0x3);

	s.SetRange(1, 2, 0xF1);

	s.BitXorRange(1, 2, 0xF2);
	REQUIRE_EQ(s.Get(1), 0x3);
	REQUIRE_EQ(s.Get(2), 0x3);

	for (int i = 0; i < 8; ++i) {
		s.Set(i + 1, i + 1);
	}

	s.BitXorArray(2, 5, 4);
	REQUIRE_EQ(s.Get(2), 2 ^ 4);
	REQUIRE_EQ(s.Get(3), 3 ^ 5);
	REQUIRE_EQ(s.Get(4), 4 ^ 6);
	REQUIRE_EQ(s.Get(5), 5 ^ 7);
}

TEST_CASE("BitShiftLeft") {
	auto s = make();

	s.Set(1, 0x1010);

	REQUIRE_EQ(s.BitShiftLeft(1, 0x2), 0x4040);
	REQUIRE_EQ(s.Get(1), 0x4040);

	s.SetRange(1, 2, 0x1010);

	s.BitShiftLeftRange(1, 2, 0x2);
	REQUIRE_EQ(s.Get(1), 0x4040);
	REQUIRE_EQ(s.Get(2), 0x4040);

	for (int i = 0; i < 8; ++i) {
		s.Set(i + 1, i + 1);
	}

	s.BitShiftLeftArray(2, 5, 4);
	REQUIRE_EQ(s.Get(2), 2 << 4);
	REQUIRE_EQ(s.Get(3), 3 << 5);
	REQUIRE_EQ(s.Get(4), 4 << 6);
	REQUIRE_EQ(s.Get(5), 5 << 7);
}

TEST_CASE("BitShiftRight") {
	auto s = make();

	s.Set(1, 0x4040);

	REQUIRE_EQ(s.BitShiftRight(1, 0x2), 0x1010);
	REQUIRE_EQ(s.Get(1), 0x1010);

	s.SetRange(1, 2, 0x4040);

	s.BitShiftRightRange(1, 2, 0x2);
	REQUIRE_EQ(s.Get(1), 0x1010);
	REQUIRE_EQ(s.Get(2), 0x1010);

	for (int i = 0; i < 8; ++i) {
		s.Set(i + 1, i + 1);
	}

	s.BitShiftRightArray(4, 5, 2);
	REQUIRE_EQ(s.Get(4), 4 >> 2);
	REQUIRE_EQ(s.Get(5), 5 >> 3);
}

TEST_CASE("RangeVariable") {
	constexpr int n = max_vars * 2;
	auto s = make();
	for (int i = 0; i < n; ++i) {
		REQUIRE_EQ(s.Get(i), 0);
	}

	s.SetRange(2, 5, 0);
	s.Set(2, 1);

	REQUIRE_EQ(s.Get(1), 0);
	REQUIRE_EQ(s.Get(2), 1);
	REQUIRE_EQ(s.Get(3), 0);
	REQUIRE_EQ(s.Get(4), 0);
	REQUIRE_EQ(s.Get(5), 0);
	REQUIRE_EQ(s.Get(6), 0);

	s.AddRangeVariable(3, 5, 2);

	REQUIRE_EQ(s.Get(1), 0);
	REQUIRE_EQ(s.Get(2), 1);
	REQUIRE_EQ(s.Get(3), 1);
	REQUIRE_EQ(s.Get(4), 1);
	REQUIRE_EQ(s.Get(5), 1);
	REQUIRE_EQ(s.Get(6), 0);

	s.AddRangeVariable(2, 4, 2);

	REQUIRE_EQ(s.Get(1), 0);
	REQUIRE_EQ(s.Get(2), 2);
	REQUIRE_EQ(s.Get(3), 3);
	REQUIRE_EQ(s.Get(4), 3);
	REQUIRE_EQ(s.Get(5), 1);
	REQUIRE_EQ(s.Get(6), 0);

	s.SetRange(2, 5, 0);
	s.Set(3, 1);
	s.AddRangeVariable(2, 5, 3);

	REQUIRE_EQ(s.Get(1), 0);
	REQUIRE_EQ(s.Get(2), 1);
	REQUIRE_EQ(s.Get(3), 2);
	REQUIRE_EQ(s.Get(4), 2);
	REQUIRE_EQ(s.Get(5), 2);
	REQUIRE_EQ(s.Get(6), 0);

	s.SetRange(2, 5, 0);
	s.Set(5, 1);
	s.AddRangeVariable(2, 5, 5);

	REQUIRE_EQ(s.Get(1), 0);
	REQUIRE_EQ(s.Get(2), 1);
	REQUIRE_EQ(s.Get(3), 1);
	REQUIRE_EQ(s.Get(4), 1);
	REQUIRE_EQ(s.Get(5), 2);
	REQUIRE_EQ(s.Get(6), 0);
}


TEST_CASE("RangeVariableIndirect") {
	constexpr int n = max_vars * 2;
	auto s = make();
	for (int i = 0; i < n; ++i) {
		REQUIRE_EQ(s.Get(i), 0);
	}

	s.Set(4, 10);
	s.Set(2, 4);
	s.Set(10, 42);

	s.SetRangeVariableIndirect(1, 5, 2);

	REQUIRE_EQ(s.Get(1), 10);
	REQUIRE_EQ(s.Get(2), 10);
	REQUIRE_EQ(s.Get(3), 42);
	REQUIRE_EQ(s.Get(4), 42);
	REQUIRE_EQ(s.Get(5), 42);
	REQUIRE_EQ(s.Get(6), 0);
}

TEST_CASE("RangeRandom") {
	constexpr int n = max_vars * 2;
	auto s = make();
	for (int i = 0; i < n; ++i) {
		REQUIRE_EQ(s.Get(i), 0);
	}

	s.SetRangeRandom(1,100,-999,999);

	int first_val = s.Get(1);
	int first_diff = 0;
	for (int i = 1; i <= 100; ++i) {
		if (s.Get(i) != first_val && !first_diff) {
			first_diff = i;
		}
		REQUIRE_GE(s.Get(i), -999);
		REQUIRE_LE(s.Get(i), 999);
	}

	// Verifies the RNG was called for each value.
	REQUIRE_NE(first_diff, 0);
}

TEST_CASE("Overflow/Underflow") {
	lcf::Data::variables.resize(max_vars);

	auto _min = std::numeric_limits<Game_Variables::Var_t>::min();
	auto _max = std::numeric_limits<Game_Variables::Var_t>::max();

	Game_Variables v(_min, _max);
	v.SetWarning(0);

	v.Set(1, _max);
	v.Add(1, 1);
	REQUIRE(v.Get(1) == _max);

	v.Set(1, _min);
	v.Add(1, -1);
	REQUIRE(v.Get(1) == _min);

	v.Set(1, _max);
	v.Sub(1, -1);
	REQUIRE(v.Get(1) == _max);

	v.Set(1, _min);
	v.Sub(1, 1);
	REQUIRE(v.Get(1) == _min);

	v.Set(1, _max);
	v.Mult(1, 2);
	REQUIRE(v.Get(1) == _max);

	v.Set(1, _min);
	v.Mult(1, -2);
	REQUIRE(v.Get(1) == _max);

	v.Set(1, _max);
	v.Mult(1, -2);
	REQUIRE(v.Get(1) == _min);

	v.Set(1, _min);
	v.Mult(1, 2);
	REQUIRE(v.Get(1) == _min);
}

TEST_CASE("Enumerate") {
	auto s = make();

	s.EnumerateRange(2, 5, 3);

	REQUIRE_EQ(s.Get(2), 3);
	REQUIRE_EQ(s.Get(3), 4);
	REQUIRE_EQ(s.Get(4), 5);
	REQUIRE_EQ(s.Get(5), 6);
}

TEST_CASE("Swap") {
	auto s = make();

	s.EnumerateRange(1, 9, 1);

	s.SwapArray(2, 6, 4);

	REQUIRE_EQ(s.Get(2), 8);
	REQUIRE_EQ(s.Get(3), 7);
	REQUIRE_EQ(s.Get(4), 2);
	REQUIRE_EQ(s.Get(5), 3);
	REQUIRE_EQ(s.Get(6), 4);
	REQUIRE_EQ(s.Get(7), 5);
	REQUIRE_EQ(s.Get(8), 6);
	REQUIRE_EQ(s.Get(9), 9);
}

TEST_CASE("Sort") {
	auto s = make();

	s.Set(2, 4);
	s.Set(3, 6);
	s.Set(4, 5);

	s.SortRange(2, 4, true);

	REQUIRE_EQ(s.Get(2), 4);
	REQUIRE_EQ(s.Get(3), 5);
	REQUIRE_EQ(s.Get(4), 6);

	s.Set(2, 4);
	s.Set(3, 6);
	s.Set(4, 5);

	s.SortRange(2, 4, false);

	REQUIRE_EQ(s.Get(2), 6);
	REQUIRE_EQ(s.Get(3), 5);
	REQUIRE_EQ(s.Get(4), 4);
}

TEST_SUITE_END();
