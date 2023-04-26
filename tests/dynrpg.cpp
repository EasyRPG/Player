#include <lcf/data.h>
#include "doctest.h"
#include "dynrpg.h"
#include "game_variables.h"
#include "test_mock_actor.h"

TEST_SUITE_BEGIN("DynRPG");

TEST_CASE("Basic parsing") {
	std::vector<std::string> args;
	CHECK(DynRpg::ParseCommand(R"(@FunC abc , "Hello World", 42, 1.5, 1.5a, "Str"",ing")", args) == "func");
	CHECK(args.size() == 6);
	CHECK(args[0] == "abc");
	CHECK(args[1] == "Hello World");
	CHECK(args[2] == "42");
	CHECK(args[3] == "1.5");
	CHECK(args[4] == "1.5a"); // DynRPG returns 1.5, not critical, we extract float later
	CHECK(args[5] == "Str\",ing");
}

TEST_CASE("Unterminated literal") {
	std::vector<std::string> args;
	CHECK(DynRpg::ParseCommand("@FunC abc , \"Hello ", args) == "func");
	CHECK(args.size() == 2);
	CHECK(args[0] == "abc");
	CHECK(args[1] == "Hello ");
}

TEST_CASE("Comma") {
	std::vector<std::string> args;
	CHECK(DynRpg::ParseCommand("@FunC , abc,,,xyz,,", args) == "func");
	CHECK(args.size() == 7);
	CHECK(args[0] == "");
	CHECK(args[1] == "abc");
	CHECK(args[2] == "");
	CHECK(args[3] == "");
	CHECK(args[4] == "xyz");
	CHECK(args[5] == "");
	CHECK(args[6] == "");
}

TEST_CASE("Tokens") {
	std::vector<std::string> args;
	CHECK(DynRpg::ParseCommand("@FunC ab cd,Bla  ,   Bla Blub, Ab\"C \"d e", args) == "func");
	CHECK(args.size() == 4);
	CHECK(args[0] == "abcd");
	CHECK(args[1] == "bla");
	CHECK(args[2] == "blablub");
	CHECK(args[3] == "ab\"c\"de");
}

TEST_CASE("Variable names") {
	const MockActor m;

	std::vector<int32_t> vars = {100, 4, 2, 1};
	Main_Data::game_variables->SetData(vars);
	Main_Data::game_variables->SetWarning(0);

	std::vector<std::string> args;
	CHECK(DynRpg::ParseCommand("@FunC V2,VV3,VVVV3", args) == "func");
	CHECK(args.size() == 3);
	CHECK(args[0] == "4");
	CHECK(args[1] == "4");
	CHECK(args[2] == "100");
}

TEST_CASE("Variable names with junk") {
	const MockActor m;

	std::vector<int32_t> vars = {0, 4, 2, 1};
	Main_Data::game_variables->SetData(vars);
	Main_Data::game_variables->SetWarning(0);

	std::vector<std::string> args;
	CHECK(DynRpg::ParseCommand("@FunC V2junk,VV3 junk,VVVjunk3", args) == "func");
	CHECK(args.size() == 3);
	CHECK(args[0] == "4");
	CHECK(args[1] == "4");
	CHECK(args[2] == "vvvjunk3");
}

TEST_CASE("Variable names with junk") {
	const MockActor m;

	std::vector<int32_t> vars = {0, 4, 2};
	Main_Data::game_variables->SetData(vars);
	Main_Data::game_variables->SetWarning(0);

	MakeDBActor(2);
	MakeDBActor(4);
	Main_Data::game_actors->GetActor(2)->SetName("Actor 2");
	Main_Data::game_actors->GetActor(4)->SetName("Actor 4");

	std::vector<std::string> args;
	CHECK(DynRpg::ParseCommand("@FunC N2junk,NV3 junk,NVV3", args) == "func");
	CHECK(args.size() == 3);
	CHECK(args[0] == "Actor 2");
	CHECK(args[1] == "Actor 2");
	CHECK(args[2] == "Actor 4");
}

TEST_CASE("Arg parse") {
	const MockActor m; // disable log

	// 2.5x not tested here because the result differs between different C++ libraries
	// See: https://bugs.llvm.org/show_bug.cgi?id=17782
	std::vector<std::string> args = {"A", "1y", "2.5 x"};
	bool okay = false;

	std::string s, s2;
	int i, i2;
	float f;

	std::tie(s, i, f) = DynRpg::ParseArgs<std::string, int, float>("func", args, &okay);
	CHECK(okay);
	CHECK(s == "A");
	CHECK(i == 1);
	CHECK(f == 2.5);

	std::tie(i, i2, f) = DynRpg::ParseArgs<int, int, float>("func", args, &okay);
	CHECK(!okay);
	CHECK(i == 0);
	CHECK(i2 == 0);
	CHECK(f == 0.0);

	std::tie(f, i,s) = DynRpg::ParseArgs<float, int, std::string>("func", args, &okay);
	CHECK(!okay);
	CHECK(f == 0.0f);
	CHECK(i == 0);
	CHECK(s == "");

	std::tie(s, s2, i) = DynRpg::ParseArgs<std::string, std::string, int>("func", args, &okay);
	CHECK(okay);
	CHECK(s == "A");
	CHECK(s2 == "1y");
	CHECK(i == 2);

	std::tie(std::ignore, std::ignore, s) = DynRpg::ParseArgs<std::string, std::string, std::string>("func", args, &okay);
	CHECK(okay);
	CHECK(s == "2.5 x");

	// DynRPG reports string here but for convenience empty strings are 0
	args = {"", ""};
	std::tie(f, i) = DynRpg::ParseArgs<float, int>("func", args, &okay);
	CHECK(okay);
	CHECK(f == 0.0f);
	CHECK(i == 0);
}

TEST_CASE("easyrpg dynrpg invoke") {
	const MockActor m;

	std::vector<int32_t> vars = {-1};
	Main_Data::game_variables->SetData(vars);
	Main_Data::game_variables->SetWarning(0);

	DynRpg::Invoke("@easyrpg_add 1, 2, 4");
	CHECK(Main_Data::game_variables->Get(1) == 6);

	// Invalid, stays 6
	DynRpg::Invoke("@easyrpg_add 1, 2, a");
	CHECK(Main_Data::game_variables->Get(1) == 6);

	// Not enough args, stays 6
	DynRpg::Invoke("@easyrpg_add 1");
	CHECK(Main_Data::game_variables->Get(1) == 6);

	DynRpg::Invoke("@easyrpg_add 1, 2");
	CHECK(Main_Data::game_variables->Get(1) == 2);

	DynRpg::Invoke("@call easyrpg_add, 1, 4.3, 7.7");
	CHECK(Main_Data::game_variables->Get(1) == 11);

	// Extra args ignored
	DynRpg::Invoke("@call easyrpg_add, 1, 4, 8, -14.0");
	CHECK(Main_Data::game_variables->Get(1) == -2);

	// Invalid func, stays -2
	DynRpg::Invoke("@call easyrpg_xxx, 1, 4, 7");
	CHECK(Main_Data::game_variables->Get(1) == -2);

	// does not crash
	DynRpg::Invoke("@unknownfunc 1, 2, 3");
}

TEST_CASE("Incompatible changes") {
	const MockActor m; // disable log

	std::vector<std::string> args;

	// Empty function name (DynRPG accepts this, but makes no sense)
	CHECK(DynRpg::ParseCommand("@ aaa", args).empty());

	// Text after literal (DynRPG accepts this, extra data is removed)
	CHECK(DynRpg::ParseCommand(R"(@FunC "Arg"junk)", args).empty());

	// DynRpg interprets as float and reads V3
	Main_Data::game_variables = std::make_unique<Game_Variables>(-99999, 99999);
	std::vector<int32_t> vars = {0, 4, 5};
	Main_Data::game_variables->SetData(vars);
	args.clear();
	CHECK(DynRpg::ParseCommand("@FunC V2.5", args) == "func");
	CHECK(args.size() == 1);
	CHECK(args[0] == "4");
}

TEST_SUITE_END();
