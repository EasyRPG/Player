#include <cassert>
#include <cstdlib>
#include "utils.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

// Correct Tests

struct TestSet {
	std::string u8;
	std::u16string u16;
	std::u32string u32;
};

#define CONCAT2(x, y) x ## y
#define CONCAT(x, y) CONCAT2(x, y)
#define STRINGIZE(x) #x
//#define TS(X) { CONCAT(u8, STRINGIZE(X)), CONCAT(u, STRINGIZE(X)), CONCAT(U, STRINGIZE(X)) }
#define TS(X) { CONCAT(u8, X), CONCAT(u, X), CONCAT(U, X) }

TestSet tests[] = {
	//Valid Strings
	TS("κόσμε"),
	//First Char tests
	TS("\U00000000"),
	TS("\U00000080"),
	TS("\U00000800"),
	TS("\U00010000"),
//	TS("\U00200000"),
//	TS("\U04000000"),
//	//Last char tests
	TS("\U0000007F"),
	TS("\U000007FF"),
	TS("\U0000FFFF"),
//	TS("\U001FFFFF"),
//	TS("\U03FFFFFF"),
//	TS("\U7FFFFFFF"),
//	//Other boundary tests
	TS("\U0000D7FF"),
	TS("\U0000E000"),
	TS("\U0000FFFD"),
	TS("\U0010FFFF"),
//	TS("\U00110000"),
};

TEST_CASE("8to16") {
	for (auto& ts: tests) {
		auto u16 = Utils::DecodeUTF16(ts.u8);
		REQUIRE_EQ(u16, ts.u16);
	}
}

TEST_CASE("8to32") {
	for (auto& ts: tests) {
		auto u32 = Utils::DecodeUTF32(ts.u8);
		REQUIRE_EQ(u32, ts.u32);
	}
}

TEST_CASE("16to8") {
	for (auto& ts: tests) {
		auto u8 = Utils::EncodeUTF(ts.u16);
		REQUIRE_EQ(u8, ts.u8);
	}
}

TEST_CASE("32to8") {
	for (auto& ts: tests) {
		auto u8 = Utils::EncodeUTF(ts.u32);
		REQUIRE_EQ(u8, ts.u8);
	}
}

TEST_CASE("next") {
	for (auto& ts: tests) {
		const char* iter = ts.u8.data();
		const char* const e = ts.u8.data() + ts.u8.size();
		int i = 0;
		while (iter < e) {
			auto ret = Utils::UTF8Next(iter, e);
			REQUIRE_EQ(ret.ch, ts.u32[i]);
			iter = ret.iter;
			++i;
		}
	}
}
