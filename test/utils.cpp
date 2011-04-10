#include <gtest/gtest.h>

#include "utils.h"


TEST(Utils, LowerCase) {
	ASSERT_EQ(Utils::LowerCase("EasyRPG"), "easyrpg");
	ASSERT_EQ(Utils::LowerCase("player"), "player");
}
