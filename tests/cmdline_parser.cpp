#include <ostream>
#include "cmdline_parser.h"
#include "doctest.h"

TEST_SUITE_BEGIN("CmdlineParser");

TEST_CASE("SkipAll") {
	std::vector<std::string> args = { "testapp", "--bool", "--value", "val" };

	CmdlineParser cp(args);

	for (size_t i = 0; i < args.size() - 1; ++ i) {
		REQUIRE(!cp.Done());
		cp.SkipNext();
	}
	REQUIRE(cp.Done());
}

TEST_CASE("ParseFound") {
	std::vector<std::string> args = { "testapp", "--bool", "--value", "val" };

	CmdlineParser cp(args);

	CmdlineArg arg;
	REQUIRE(!cp.Done());
	REQUIRE(cp.ParseNext(arg, 0, "--bool", 'b'));
	REQUIRE(!cp.Done());
	REQUIRE(cp.ParseNext(arg, 1, "--value", 'v'));
	REQUIRE_EQ(arg.NumValues(), 1);
	REQUIRE_EQ(arg.Value(0), "val");
	REQUIRE(cp.Done());
}

TEST_CASE("ParseMissing") {
	std::vector<std::string> args = { "testapp", "--bool", "--value", "v" };

	CmdlineParser cp(args);

	while (!cp.Done()) {
		CmdlineArg arg;
		REQUIRE(!cp.ParseNext(arg, 0, "--missing-bool", 'b'));
		REQUIRE(!cp.ParseNext(arg, 1, "--missing-value", 'v'));

		cp.SkipNext();
	}
}

TEST_CASE("ParseMulti") {
	std::vector<std::string> args = { "testapp", "battletest", "0", "1", "2" };

	CmdlineParser cp(args);

	CmdlineArg arg;
	REQUIRE(!cp.Done());
	REQUIRE(cp.ParseNext(arg, 3, {"--battle-test", "battletest"}, 'b'));
	REQUIRE(cp.Done());

	REQUIRE_EQ(arg.NumValues(), 3);
	REQUIRE_EQ(arg.Value(0), "0");
	REQUIRE_EQ(arg.Value(1), "1");
	REQUIRE_EQ(arg.Value(2), "2");

	long li = -1;
	REQUIRE(arg.ParseValue(0, li));
	REQUIRE_EQ(li, 0);
	REQUIRE(arg.ParseValue(1, li));
	REQUIRE_EQ(li, 1);
	REQUIRE(arg.ParseValue(2, li));
	REQUIRE_EQ(li, 2);
}

TEST_CASE("ParseNull") {
	std::vector<std::string> args;
	CmdlineParser cp(args);
	REQUIRE(cp.Done());
}
