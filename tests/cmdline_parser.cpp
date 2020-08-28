#include <ostream>
#include "cmdline_parser.h"
#include "doctest.h"

TEST_SUITE_BEGIN("CmdlineParser");

TEST_CASE("SkipAll") {
	const int argc = 4;
	const char* argv[] = { "testapp", "--bool", "--value", "val" };

	CmdlineParser cp(argc, const_cast<char**>(argv));

	for (int i = 0; i < argc - 1; ++ i) {
		REQUIRE(!cp.Done());
		cp.SkipNext();
	}
	REQUIRE(cp.Done());
}

TEST_CASE("ParseFound") {
	const int argc = 4;
	const char* argv[] = { "testapp", "--bool", "--value", "val" };

	CmdlineParser cp(argc, const_cast<char**>(argv));


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
	const int argc = 4;
	const char* argv[] = { "testapp", "--bool", "--value", "v" };

	CmdlineParser cp(argc, const_cast<char**>(argv));

	while (!cp.Done()) {
		CmdlineArg arg;
		REQUIRE(!cp.ParseNext(arg, 0, "--missing-bool", 'b'));
		REQUIRE(!cp.ParseNext(arg, 1, "--missing-value", 'v'));

		cp.SkipNext();
	}
}

TEST_CASE("ParseMulti") {
	const int argc = 5;
	const char* argv[] = { "testapp", "battletest", "0", "1", "2" };

	CmdlineParser cp(argc, const_cast<char**>(argv));

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
	CmdlineParser cp(0, (char**)nullptr);
	REQUIRE(cp.Done());
}
