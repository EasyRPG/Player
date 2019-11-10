#include <cassert>
#include <cstdlib>
#include "utils.h"
#include "doctest.h"

TEST_SUITE_BEGIN("Utils");

TEST_CASE("Lower") {
	REQUIRE_EQ(Utils::LowerCase("EasyRPG"), "easyrpg");
	REQUIRE_EQ(Utils::LowerCase("player"), "player");
}

TEST_SUITE_END();
