#include <cassert>
#include <cstdlib>
#include "utils.h"
#include "doctest.h"

TEST_SUITE_BEGIN("Utils");

TEST_CASE("Lower") {
	REQUIRE_EQ(Utils::LowerCase("EasyRPG"), "easyrpg");
	REQUIRE_EQ(Utils::LowerCase("player"), "player");
	REQUIRE_EQ(Utils::LowerCase("!A/b"), "!a/b");
}

TEST_CASE("Upper") {
	REQUIRE_EQ(Utils::UpperCase("EasyRPG"), "EASYRPG");
	REQUIRE_EQ(Utils::UpperCase("player"), "PLAYER");
	REQUIRE_EQ(Utils::UpperCase("!A/b"), "!A/B");
}

template <typename T>
static void testStrICmp() {
	REQUIRE_EQ(Utils::StrICmp(T("easyrpg"), T("easyrpg")), 0);
	REQUIRE_EQ(Utils::StrICmp(T("easyrpg"), T("EASYRPG")), 0);
	REQUIRE_EQ(Utils::StrICmp(T("EASYRPG"), T("easyrpg")), 0);

	REQUIRE_LT(Utils::StrICmp(T("A"), T("B")), 0);
	REQUIRE_LT(Utils::StrICmp(T("a"), T("B")), 0);
	REQUIRE_LT(Utils::StrICmp(T("A"), T("b")), 0);

	REQUIRE_GT(Utils::StrICmp(T("B"), T("A")), 0);
	REQUIRE_GT(Utils::StrICmp(T("b"), T("A")), 0);
	REQUIRE_GT(Utils::StrICmp(T("B"), T("a")), 0);

	REQUIRE_GT(Utils::StrICmp(T("AA"), T("A")), 0);
	REQUIRE_GT(Utils::StrICmp(T("aa"), T("A")), 0);
	REQUIRE_GT(Utils::StrICmp(T("AA"), T("a")), 0);

	REQUIRE_LT(Utils::StrICmp(T("A"), T("AA")), 0);
	REQUIRE_LT(Utils::StrICmp(T("a"), T("AA")), 0);
	REQUIRE_LT(Utils::StrICmp(T("A"), T("aa")), 0);
}

TEST_CASE("StrICmp") {

	SUBCASE("cstr") {
		testStrICmp<const char*>();
	}

	SUBCASE("sv") {
		testStrICmp<StringView>();
	}
}

TEST_CASE("ReplaceAll") {
	SUBCASE("one") {
		REQUIRE(Utils::ReplaceAll("abc", "b", "xyz") == "axyzc");
		REQUIRE(Utils::ReplaceAll("abc", "abc", "uvwxyz") == "uvwxyz");
		REQUIRE(Utils::ReplaceAll("abc", "c", "xyz") == "abxyz");
	}

	SUBCASE("url encode") {
		REQUIRE(Utils::ReplaceAll("ab+cd", "+", "%2B") == "ab%2Bcd");
		REQUIRE(Utils::ReplaceAll("ab%cd", "%", "%25") == "ab%25cd");
	}

	SUBCASE("many") {
		REQUIRE(Utils::ReplaceAll("abba", "b", "xyz") == "axyzxyza");
		REQUIRE(Utils::ReplaceAll("abba", "a", "ba") == "babbba");
	}
}

TEST_CASE("TrimWhitespace") {
	SUBCASE("left") {
		REQUIRE(Utils::TrimWhitespace(" le ft") == "le ft");
		REQUIRE(Utils::TrimWhitespace("   le ft") == "le ft");
	}

	SUBCASE("right") {
		REQUIRE(Utils::TrimWhitespace("ri ght ") == "ri ght");
		REQUIRE(Utils::TrimWhitespace("ri ght  ") == "ri ght");
	}

	SUBCASE("both") {
		REQUIRE(Utils::TrimWhitespace(" bot h ") == "bot h");
		REQUIRE(Utils::TrimWhitespace("    bot h  ") == "bot h");
		REQUIRE(Utils::TrimWhitespace(" bot h    ") == "bot h");
	}

	SUBCASE("other") {
		REQUIRE(Utils::TrimWhitespace("") == "");
		REQUIRE(Utils::TrimWhitespace("    ") == "");
		REQUIRE(Utils::TrimWhitespace("T ex t") == "T ex t");
	}
}

TEST_SUITE_END();
