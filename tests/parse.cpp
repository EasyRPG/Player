#include "game_actors.h"
#include "game_message.h"
#include "game_party.h"
#include "options.h"
#include <lcf/data.h>
#include "game_variables.h"
#include "main_data.h"
#include <iostream>
#include "doctest.h"

TEST_SUITE_BEGIN("Parse");

constexpr char escape = '\\';

struct DataInit {
	DataInit() {
		for (int i = 0; i < 4; ++i) {
			lcf::Data::actors.push_back({});
			lcf::Data::actors.back().ID = i + 1;
			lcf::Data::actors.back().Setup(true);
		}
		lcf::rpg::SaveInventory inventory;
		inventory.party.push_back(3);

		Main_Data::game_actors = std::make_unique<Game_Actors>();
		Main_Data::game_party = std::make_unique<Game_Party>();
		Main_Data::game_party->SetupFromSave(std::move(inventory));
		Main_Data::game_variables = std::make_unique<Game_Variables>(Game_Variables::min_2k3, Game_Variables::max_2k3);
		Main_Data::game_variables->SetWarning(0);
	}
	~DataInit() {
		lcf::Data::actors.clear();
		Main_Data::game_party.reset();
		Main_Data::game_variables.reset();
	}
};

TEST_CASE("Actors") {
	DataInit init;

	std::string msg;
	Game_Message::ParseParamResult ret;

	msg = u8"\\n[0]";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 3);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	msg = u8"\\n[1]";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 1);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	msg = u8"\\n[2]";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 2);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	msg = u8"\\n[3]HelloWorld";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 3);
	REQUIRE_EQ(ret.next, msg.data() + 5);

	msg = u8"\\n[55]HelloWorld";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 55);
	REQUIRE_EQ(ret.next, msg.data() + 6);

	msg = u8"\\N[55]HelloWorld";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 55);
	REQUIRE_EQ(ret.next, msg.data() + 6);

	msg = u8"\\C[55]HelloWorld";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.next, msg.data());
}

TEST_CASE("BadActors") {
	DataInit init;

	std::string msg;
	Game_Message::ParseParamResult ret;

	msg = u8"\\n[A]";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	msg = u8"\\n[2A]";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 2);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	msg = u8"\\n[A2]";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	msg = u8"\\n[2A2]";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 2);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	msg = u8"\\n[]";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	msg = u8"\\n[";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	msg = u8"\\n[2";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 2);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	msg = u8"\\n";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	msg = u8"\\n";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	msg = u8"\\n[01]";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 1);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	msg = u8"\\n[02]";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 2);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	msg = u8"\\n[000]";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 3);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));
}

TEST_CASE("ActorVars") {
	DataInit init;

	std::string msg;
	Game_Message::ParseParamResult ret;

	msg = u8"\\n[\\v[0]]";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 3);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	Main_Data::game_variables->Set(1, 0);
	msg = u8"\\n[\\v[1]]";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 3);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	Main_Data::game_variables->Set(1, 1);
	msg = u8"\\n[\\v[1]]";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 1);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	Main_Data::game_variables->Set(1, 5);
	msg = u8"\\n[\\v[1]]";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 5);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	Main_Data::game_variables->Set(1, 2);
	Main_Data::game_variables->Set(2, 3);
	msg = u8"\\n[\\v[2]\\v[1]]";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 32);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	Main_Data::game_variables->Set(1, 2);
	Main_Data::game_variables->Set(2, 3);
	msg = u8"\\n[\\v[1]\\v[0]]";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 20);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	msg = u8"\\n[\\v[0]\\v[0]]";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 3);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));
}

TEST_CASE("ActorVarsRecurse") {
	DataInit init;

	std::string msg;
	Game_Message::ParseParamResult ret;

	Main_Data::game_variables->Set(1, 2);
	Main_Data::game_variables->Set(2, 50);
	msg = u8"\\n[\\v[\\v[1]]]";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape, false, Game_Message::easyrpg_default_max_recursion);
	REQUIRE_EQ(ret.value, 50);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	Main_Data::game_variables->Set(1, 2);
	Main_Data::game_variables->Set(2, 50);
	msg = u8"\\n[\\v[\\v[1]]]";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape, false, Game_Message::rpg_rt_default_max_recursion);
	REQUIRE_EQ(ret.value, 3);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()) - 1);
}

TEST_CASE("ActorsUnicode") {
	DataInit init;

	// Hack for MSVC. Complains with error C2015 when we try to use U'σ'
	const uint32_t esc = U"σ"[0];
	std::string msg;
	Game_Message::ParseParamResult ret;

	msg = u8"σn[1]";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), esc);
	REQUIRE_EQ(ret.value, 1);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	Main_Data::game_variables->Set(1, 25);
	msg = u8"σn[σv[1]]";
	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), esc);
	REQUIRE_EQ(ret.value, 25);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));
}

TEST_CASE("Variables") {
	DataInit init;

	std::string msg;
	Game_Message::ParseParamResult ret;

	msg = u8"\\v[0]";
	ret = Game_Message::ParseVariable(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	msg = u8"\\v[1]";
	ret = Game_Message::ParseVariable(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 1);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	msg = u8"\\v[A]";
	ret = Game_Message::ParseVariable(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	msg = u8"\\v[999]";
	ret = Game_Message::ParseVariable(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 999);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	msg = u8"\\v[1]HelloWorld";
	ret = Game_Message::ParseVariable(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 1);
	REQUIRE_EQ(ret.next, &*msg.data() + 5);
}

TEST_CASE("VarsRecurse") {
	DataInit init;

	std::string msg;
	Game_Message::ParseParamResult ret;

	Main_Data::game_variables->Set(1, 2);
	msg = u8"\\v[\\v[1]]";
	ret = Game_Message::ParseVariable(msg.data(), (msg.data() + msg.size()), escape, false, Game_Message::easyrpg_default_max_recursion);
	REQUIRE_EQ(ret.value, 2);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	Main_Data::game_variables->Set(1, 2);
	msg = u8"\\v[\\v[1]]";
	ret = Game_Message::ParseVariable(msg.data(), (msg.data() + msg.size()), escape, false, Game_Message::rpg_rt_default_max_recursion);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()) - 1);
}

TEST_CASE("Colors") {
	DataInit init;

	std::string msg;
	Game_Message::ParseParamResult ret;

	msg = u8"\\c[0]";
	ret = Game_Message::ParseColor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	msg = u8"\\c[1]";
	ret = Game_Message::ParseColor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 1);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	msg = u8"\\c[A]";
	ret = Game_Message::ParseColor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	msg = u8"\\c[999]";
	ret = Game_Message::ParseColor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 999);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	msg = u8"\\c[1]HelloWorld";
	ret = Game_Message::ParseColor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 1);
	REQUIRE_EQ(ret.next, &*msg.data() + 5);
}

TEST_CASE("ColorVarsRecurse") {
	DataInit init;

	std::string msg;
	Game_Message::ParseParamResult ret;

	Main_Data::game_variables->Set(1, 2);
	Main_Data::game_variables->Set(2, 50);
	msg = u8"\\c[\\v[\\v[1]]]";
	ret = Game_Message::ParseColor(msg.data(), (msg.data() + msg.size()), escape, false, Game_Message::easyrpg_default_max_recursion);
	REQUIRE_EQ(ret.value, 50);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	Main_Data::game_variables->Set(1, 2);
	Main_Data::game_variables->Set(2, 50);
	msg = u8"\\c[\\v[\\v[1]]]";
	ret = Game_Message::ParseColor(msg.data(), (msg.data() + msg.size()), escape, false, Game_Message::rpg_rt_default_max_recursion);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()) - 1);
}

TEST_CASE("Speed") {
	DataInit init;

	std::string msg;
	Game_Message::ParseParamResult ret;

	msg = u8"\\s[0]";
	ret = Game_Message::ParseSpeed(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	msg = u8"\\s[1]";
	ret = Game_Message::ParseSpeed(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 1);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	msg = u8"\\s[A]";
	ret = Game_Message::ParseSpeed(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	msg = u8"\\s[999]";
	ret = Game_Message::ParseSpeed(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 999);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	msg = u8"\\s[1]HelloWorld";
	ret = Game_Message::ParseSpeed(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 1);
	REQUIRE_EQ(ret.next, &*msg.data() + 5);
}

TEST_CASE("SpeedVarsRecurse") {
	DataInit init;

	std::string msg;
	Game_Message::ParseParamResult ret;

	Main_Data::game_variables->Set(1, 2);
	Main_Data::game_variables->Set(2, 50);
	msg = u8"\\s[\\v[\\v[1]]]";
	ret = Game_Message::ParseSpeed(msg.data(), (msg.data() + msg.size()), escape, false, Game_Message::easyrpg_default_max_recursion);
	REQUIRE_EQ(ret.value, 50);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()));

	Main_Data::game_variables->Set(1, 2);
	Main_Data::game_variables->Set(2, 50);
	msg = u8"\\s[\\v[\\v[1]]]";
	ret = Game_Message::ParseSpeed(msg.data(), (msg.data() + msg.size()), escape, false, Game_Message::rpg_rt_default_max_recursion);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.next, (msg.data() + msg.size()) - 1);
}

TEST_CASE("BadActor") {
	DataInit init;

	std::string msg;
	Game_Message::ParseParamResult ret;

	msg = u8"\\n[3]";

	ret = Game_Message::ParseVariable(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.next, msg.data());

	ret = Game_Message::ParseColor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.next, msg.data());

	ret = Game_Message::ParseSpeed(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.next, msg.data());
}

TEST_CASE("BadVariable") {
	DataInit init;

	std::string msg;
	Game_Message::ParseParamResult ret;

	msg = u8"\\v[3]";

	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.next, msg.data());

	ret = Game_Message::ParseColor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.next, msg.data());

	ret = Game_Message::ParseSpeed(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.next, msg.data());
}

TEST_CASE("BadColor") {
	DataInit init;

	std::string msg;
	Game_Message::ParseParamResult ret;

	msg = u8"\\c[3]";

	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.next, msg.data());

	ret = Game_Message::ParseVariable(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.next, msg.data());

	ret = Game_Message::ParseSpeed(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.next, msg.data());
}

TEST_CASE("BadSpeed") {
	DataInit init;

	std::string msg;
	Game_Message::ParseParamResult ret;

	msg = u8"\\s[3]";

	ret = Game_Message::ParseActor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.next, msg.data());

	ret = Game_Message::ParseVariable(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.next, msg.data());

	ret = Game_Message::ParseColor(msg.data(), (msg.data() + msg.size()), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.next, msg.data());
}

TEST_SUITE_END();
