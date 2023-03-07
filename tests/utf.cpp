#include <cassert>
#include <cstdlib>
#include "utils.h"
#include "doctest.h"

constexpr char32_t escape = '\\';
// Correct Tests
TEST_SUITE_BEGIN("UTF");

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
			iter = ret.next;
			++i;
		}
	}
}

TEST_CASE("TextNext") {
	std::string text = u8"H $A$B\\\\\\^\\n\nぽ";
	const auto* iter = text.data();
	const auto* end = text.data() + text.size();

	Utils::TextRet ret;
	ret = Utils::TextNext(iter, end, escape);
	REQUIRE_EQ(ret.ch, 'H');
	REQUIRE_NE(ret.next, end);
	REQUIRE_FALSE(ret.is_exfont);
	REQUIRE_FALSE(ret.is_escape);
	iter = ret.next;

	ret = Utils::TextNext(iter, end, escape);
	REQUIRE_EQ(ret.ch, ' ');
	REQUIRE_NE(ret.next, end);
	REQUIRE_FALSE(ret.is_exfont);
	REQUIRE_FALSE(ret.is_escape);
	iter = ret.next;

	ret = Utils::TextNext(iter, end, escape);
	REQUIRE_EQ(ret.ch, 'A');
	REQUIRE_NE(ret.next, end);
	REQUIRE(ret.is_exfont);
	REQUIRE_FALSE(ret.is_escape);
	iter = ret.next;

	ret = Utils::TextNext(iter, end, escape);
	REQUIRE_EQ(ret.ch, 'B');
	REQUIRE_NE(ret.next, end);
	REQUIRE(ret.is_exfont);
	REQUIRE_FALSE(ret.is_escape);
	iter = ret.next;

	ret = Utils::TextNext(iter, end, escape);
	REQUIRE_EQ(ret.ch, '\\');
	REQUIRE_NE(ret.next, end);
	REQUIRE_FALSE(ret.is_exfont);
	REQUIRE(ret.is_escape);
	iter = ret.next;

	ret = Utils::TextNext(iter, end, escape);
	REQUIRE_EQ(ret.ch, '^');
	REQUIRE_NE(ret.next, end);
	REQUIRE_FALSE(ret.is_exfont);
	REQUIRE(ret.is_escape);
	iter = ret.next;

	ret = Utils::TextNext(iter, end, escape);
	REQUIRE_EQ(ret.ch, 'n');
	REQUIRE_NE(ret.next, end);
	REQUIRE_FALSE(ret.is_exfont);
	REQUIRE(ret.is_escape);
	iter = ret.next;

	ret = Utils::TextNext(iter, end, escape);
	REQUIRE_EQ(ret.ch, '\n');
	REQUIRE_NE(ret.next, end);
	REQUIRE_FALSE(ret.is_exfont);
	REQUIRE_FALSE(ret.is_escape);
	iter = ret.next;

	ret = Utils::TextNext(iter, end, escape);
	REQUIRE_EQ(ret.ch, U'ぽ');
	REQUIRE_EQ(ret.next, end);
	REQUIRE_FALSE(ret.is_exfont);
	REQUIRE_FALSE(ret.is_escape);
	iter = ret.next;
}

TEST_CASE("TextNextNoEscape") {
	std::string text = u8"\\$";
	const auto* iter = text.data();
	const auto* end = text.data() + text.size();

	Utils::TextRet ret;

	ret = Utils::TextNext(iter, end, 0);
	REQUIRE_EQ(ret.ch, '\\');
	REQUIRE_NE(ret.next, end);
	REQUIRE_FALSE(ret.is_exfont);
	REQUIRE_FALSE(ret.is_escape);
	iter = ret.next;

	ret = Utils::TextNext(iter, end, 0);
	REQUIRE_EQ(ret.ch, '$');
	REQUIRE_EQ(ret.next, end);
	REQUIRE_FALSE(ret.is_exfont);
	REQUIRE_FALSE(ret.is_escape);
	iter = ret.next;
}

TEST_SUITE_END();
