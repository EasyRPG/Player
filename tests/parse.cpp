#include "game_message.h"
#include "options.h"
#include "data.h"
#include "game_variables.h"
#include <iostream>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

constexpr char escape = '\\';

struct Initializer {
	Initializer() {
		Data::actors.push_back({});
		Data::actors.back().name = "Alex";
		Data::actors.push_back({});
		Data::actors.back().name = "Brian";
		Data::actors.push_back({});
		Data::actors.back().name = "Carol";
		Data::actors.push_back({});
		Data::actors.back().name = "Daisy";
		Data::actors.push_back({});
	}
};

Initializer init;

TEST_CASE("Actors") {
	std::string msg;
	Game_Message::ParseParamResult ret;

	msg = u8"\\n[0]";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.iter, &*msg.end());

	msg = u8"\\n[1]";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), escape);
	REQUIRE_EQ(ret.value, 1);
	REQUIRE_EQ(ret.iter, &*msg.end());

	msg = u8"\\n[2]";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), escape);
	REQUIRE_EQ(ret.value, 2);
	REQUIRE_EQ(ret.iter, &*msg.end());

	msg = u8"\\n[3]HelloWorld";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), escape);
	REQUIRE_EQ(ret.value, 3);
	REQUIRE_EQ(ret.iter, msg.data() + 5);

	msg = u8"\\n[55]HelloWorld";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), escape);
	REQUIRE_EQ(ret.value, 55);
	REQUIRE_EQ(ret.iter, msg.data() + 6);

	msg = u8"\\N[55]HelloWorld";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), escape);
	REQUIRE_EQ(ret.value, 55);
	REQUIRE_EQ(ret.iter, msg.data() + 6);

	msg = u8"\\C[55]HelloWorld";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.iter, msg.data());
}

TEST_CASE("BadActors") {
	std::string msg;
	Game_Message::ParseParamResult ret;

	msg = u8"\\n[A]";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.iter, &*msg.end());

	msg = u8"\\n[2A]";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), escape);
	REQUIRE_EQ(ret.value, 2);
	REQUIRE_EQ(ret.iter, &*msg.end());

	msg = u8"\\n[A2]";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.iter, &*msg.end());

	msg = u8"\\n[2A2]";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), escape);
	REQUIRE_EQ(ret.value, 2);
	REQUIRE_EQ(ret.iter, &*msg.end());

	msg = u8"\\n[]";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.iter, &*msg.end());

	msg = u8"\\n[";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.iter, &*msg.end());

	msg = u8"\\n[2";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), escape);
	REQUIRE_EQ(ret.value, 2);
	REQUIRE_EQ(ret.iter, &*msg.end());

	msg = u8"\\n";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.iter, &*msg.end());

	msg = u8"\\n";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.iter, &*msg.end());

	msg = u8"\\n[01]";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), escape);
	REQUIRE_EQ(ret.value, 1);
	REQUIRE_EQ(ret.iter, &*msg.end());

	msg = u8"\\n[000]";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), escape);
	REQUIRE_EQ(ret.value, 0);
	REQUIRE_EQ(ret.iter, &*msg.end());
}

TEST_CASE("ActorVars") {
	std::string msg;
	Game_Message::ParseParamResult ret;

	msg = u8"\\n[\\v[0]]";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), escape);
	REQUIRE_EQ(ret.value, 1);
	REQUIRE_EQ(ret.iter, &*msg.end());

	Game_Variables.Set(1, 0);
	msg = u8"\\n[\\v[1]]";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), escape);
	REQUIRE_EQ(ret.value, 1);
	REQUIRE_EQ(ret.iter, &*msg.end());

	Game_Variables.Set(1, 1);
	msg = u8"\\n[\\v[1]]";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), escape);
	REQUIRE_EQ(ret.value, 1);
	REQUIRE_EQ(ret.iter, &*msg.end());

	Game_Variables.Set(1, 5);
	msg = u8"\\n[\\v[1]]";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), escape);
	REQUIRE_EQ(ret.value, 5);
	REQUIRE_EQ(ret.iter, &*msg.end());

	Game_Variables.Set(1, 2);
	Game_Variables.Set(2, 3);
	msg = u8"\\n[\\v[2]\\v[1]]";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), escape);
	REQUIRE_EQ(ret.value, 32);
	REQUIRE_EQ(ret.iter, &*msg.end());

	Game_Variables.Set(1, 2);
	Game_Variables.Set(2, 3);
	msg = u8"\\n[\\v[1]\\v[0]]";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), escape);
	REQUIRE_EQ(ret.value, 20);
	REQUIRE_EQ(ret.iter, &*msg.end());

	msg = u8"\\n[\\v[0]\\v[0]]";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), escape);
	REQUIRE_EQ(ret.value, 1);
	REQUIRE_EQ(ret.iter, &*msg.end());
}

TEST_CASE("ActorVarsRecurse") {
	std::string msg;
	Game_Message::ParseParamResult ret;

	Game_Variables.Set(1, 2);
	Game_Variables.Set(2, 50);
	msg = u8"\\n[\\v[\\v[1]]]";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), escape, false, Game_Message::easyrpg_default_max_recursion);
	REQUIRE_EQ(ret.value, 50);
	REQUIRE_EQ(ret.iter, &*msg.end());

	Game_Variables.Set(1, 2);
	Game_Variables.Set(2, 50);
	msg = u8"\\n[\\v[\\v[1]]]";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), escape, false, Game_Message::rpg_rt_default_max_recursion);
	REQUIRE_EQ(ret.value, 1);
	REQUIRE_EQ(ret.iter, &*msg.end() - 1);
}

TEST_CASE("ActorsUnicode") {
	// Hack for MSVC. Complains with error C2015 when we try to use U'σ'
	const uint32_t esc = U"σ"[0];
	std::string msg;
	Game_Message::ParseParamResult ret;

	msg = u8"σn[1]";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), esc);
	REQUIRE_EQ(ret.value, 1);
	REQUIRE_EQ(ret.iter, &*msg.end());

	Game_Variables.Set(1, 25);
	msg = u8"σn[σv[1]]";
	ret = Game_Message::ParseActor(&*msg.begin(), &*msg.end(), esc);
	REQUIRE_EQ(ret.value, 25);
	REQUIRE_EQ(ret.iter, &*msg.end());
}


