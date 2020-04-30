#include "graphics.h"
#include "output.h"
#include "main_data.h"
#include "doctest.h"

TEST_SUITE_BEGIN("Output");

TEST_CASE("Message Output") {
	Graphics::Init();
	Main_Data::Init();
	Output::Debug("Test {}", "debg");
	Output::Warning("Test {}", "test");
	Output::Info("Test {}", "info");
	Main_Data::Cleanup();
	Graphics::Quit();
}

TEST_SUITE_END();
