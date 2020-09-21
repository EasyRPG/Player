#include "game_event.h"
#include "doctest.h"
#include "options.h"
#include "game_map.h"
#include "main_data.h"
#include <climits>

TEST_SUITE_BEGIN("Game_Event");

TEST_CASE("IdName") {
	{
		lcf::rpg::Event event;
		Game_Event ch(0, &event);

		REQUIRE_EQ(ch.GetId(), event.ID);
		REQUIRE_EQ(ch.GetName(), event.name);
	}

	{
		lcf::rpg::Event event;
		event.ID = 5;
		event.name = "X";

		Game_Event ch(0, &event);

		REQUIRE_EQ(ch.GetId(), event.ID);
		REQUIRE_EQ(ch.GetName(), event.name);
	}
}

TEST_SUITE_END();
