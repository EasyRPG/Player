#include <cassert>
#include <cstdlib>
#include "utils.h"

static void LowerCase() {
	assert(Utils::LowerCase("EasyRPG") == "easyrpg");
	assert(Utils::LowerCase("player") == "player");
}

extern "C" int main(int, char**) {
	LowerCase();

	return EXIT_SUCCESS;
}
