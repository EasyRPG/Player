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

TEST_SUITE_END();
