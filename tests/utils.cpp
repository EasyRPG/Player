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

TEST_CASE("StrICmp") {
	REQUIRE_EQ(Utils::StrICmp("easyrpg", "easyrpg"), 0);
	REQUIRE_EQ(Utils::StrICmp("easyrpg", "EASYRPG"), 0);
	REQUIRE_EQ(Utils::StrICmp("EASYRPG", "easyrpg"), 0);

	REQUIRE_LT(Utils::StrICmp("A", "B"), 0);
	REQUIRE_LT(Utils::StrICmp("a", "B"), 0);
	REQUIRE_LT(Utils::StrICmp("A", "b"), 0);

	REQUIRE_GT(Utils::StrICmp("B", "A"), 0);
	REQUIRE_GT(Utils::StrICmp("b", "A"), 0);
	REQUIRE_GT(Utils::StrICmp("B", "a"), 0);

	REQUIRE_GT(Utils::StrICmp("AA", "A"), 0);
	REQUIRE_GT(Utils::StrICmp("aa", "A"), 0);
	REQUIRE_GT(Utils::StrICmp("AA", "a"), 0);

	REQUIRE_LT(Utils::StrICmp("A", "AA"), 0);
	REQUIRE_LT(Utils::StrICmp("a", "AA"), 0);
	REQUIRE_LT(Utils::StrICmp("A", "aa"), 0);
}

TEST_SUITE_END();
