#include "graphics.h"
#include "output.h"
#include "main_data.h"
#include "doctest.h"

TEST_SUITE_BEGIN("Output");

TEST_CASE("Message Output") {
	Graphics::Init();
	Main_Data::Init();
	Output::Debug("Test %s", "debg");
	Output::Warning("Test %s", "test");
	Output::Post("Test %s", "post");
	Main_Data::Cleanup();
	Graphics::Quit();
}

TEST_SUITE_END();
