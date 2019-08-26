#include <cassert>
#include <cstdlib>
#include "utils.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"


TEST_CASE("Lower") {
	REQUIRE_EQ(Utils::LowerCase("EasyRPG"), "easyrpg");
	REQUIRE_EQ(Utils::LowerCase("player"), "player");
}

