#include "cmdline_parser.h"
#include "doctest.h"

TEST_SUITE_BEGIN("CmdlineParser");

TEST_CASE("CheckNothing") {
	int argc = 4;
	const char* argv[] = { "testapp", "--bool", "--value", "v" };

	CmdlineParser cp(&argc, const_cast<char**>(argv));

	REQUIRE(cp.Next());
	REQUIRE(cp.Next());
	REQUIRE(cp.Next());
	REQUIRE(cp.Next());

	REQUIRE(!cp.Next());

	REQUIRE_EQ(argc, 4);
	REQUIRE(!std::strcmp(argv[0], "testapp"));
	REQUIRE(!std::strcmp(argv[1], "--bool"));
	REQUIRE(!std::strcmp(argv[2], "--value"));
	REQUIRE(!std::strcmp(argv[3], "v"));
}

TEST_CASE("CheckMissing") {
	int argc = 4;
	const char* argv[] = { "testapp", "--bool", "--value", "v" };

	CmdlineParser cp(&argc, const_cast<char**>(argv));

	while (cp.Next()) {
		REQUIRE(!cp.Check("--missing-bool", 'b'));
		REQUIRE_EQ(cp.CheckValue("--missing-value", 'v'), nullptr);
	}

	REQUIRE_EQ(argc, 4);
	REQUIRE(!std::strcmp(argv[0], "testapp"));
	REQUIRE(!std::strcmp(argv[1], "--bool"));
	REQUIRE(!std::strcmp(argv[2], "--value"));
	REQUIRE(!std::strcmp(argv[3], "v"));
}

#include <iostream>

TEST_CASE("CheckConsume") {
	int argc = 10;
	const char* argv[] = { "testapp", "--bool", "--skip-me", "--value", "v", "-x", "X", "--skip-me-too", "-u", "--end" };
	const char* val = "";

	CmdlineParser cp(&argc, const_cast<char**>(argv));

	REQUIRE(cp.Next());

	REQUIRE(cp.Next());
	REQUIRE(cp.Check("--bool"));

	REQUIRE(cp.Next());

	REQUIRE(cp.Next());
	val = cp.CheckValue("--value");
	std::cout << "VAL" << val << std::endl;
	REQUIRE_NE(val, nullptr);
	REQUIRE(!std::strcmp(val, "v"));

	REQUIRE(cp.Next());
	val = cp.CheckValue("--xxx", 'x');
	REQUIRE_NE(val, nullptr);
	REQUIRE(!std::strcmp(val, "X"));

	REQUIRE(cp.Next());

	REQUIRE(cp.Next());
	REQUIRE(cp.Check("--uuu", 'u'));

	REQUIRE(cp.Next());

	REQUIRE(!cp.Next());

	REQUIRE_EQ(argc, 4);
	REQUIRE(!std::strcmp(argv[0], "testapp"));
	REQUIRE(!std::strcmp(argv[1], "--skip-me"));
	REQUIRE(!std::strcmp(argv[2], "--skip-me-too"));
	REQUIRE(!std::strcmp(argv[3], "--end"));
}

TEST_CASE("CheckConsumeLast") {
	int argc = 2;
	const char* argv[] = { "testapp", "--bool" };

	CmdlineParser cp(&argc, const_cast<char**>(argv));

	REQUIRE(cp.Next());
	REQUIRE(!cp.Check("--bool"));

	REQUIRE(cp.Next());
	REQUIRE(cp.Check("--bool"));

	REQUIRE(!cp.Next());

	REQUIRE_EQ(argc, 1);
	REQUIRE(!std::strcmp(argv[0], "testapp"));
}

TEST_CASE("CheckConsumeLastValue") {
	int argc = 3;
	const char* argv[] = { "testapp", "--value", "v" };

	CmdlineParser cp(&argc, const_cast<char**>(argv));

	REQUIRE(cp.Next());
	REQUIRE(!cp.CheckValue("--value"));

	REQUIRE(cp.Next());
	auto val = cp.CheckValue("--value");
	REQUIRE_NE(val, nullptr);
	REQUIRE(!std::strcmp(val, "v"));

	REQUIRE(!cp.Next());

	REQUIRE_EQ(argc, 1);
	REQUIRE(!std::strcmp(argv[0], "testapp"));
}

TEST_CASE("CheckConsumeLastNoValue") {
	int argc = 2;
	const char* argv[] = { "testapp", "--value" };

	CmdlineParser cp(&argc, const_cast<char**>(argv));

	REQUIRE(cp.Next());
	REQUIRE(!cp.CheckValue("--value"));

	REQUIRE(cp.Next());
	REQUIRE(!cp.CheckValue("--value"));

	REQUIRE(!cp.Next());

	REQUIRE_EQ(argc, 2);
	REQUIRE(!std::strcmp(argv[0], "testapp"));
	REQUIRE(!std::strcmp(argv[1], "--value"));
}
