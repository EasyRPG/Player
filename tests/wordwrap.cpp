#include "game_message.h"
#include "options.h"
#include <iostream> // needed on macOS (see onqtam/doctest#126)
#include "doctest.h"

TEST_SUITE_BEGIN("Word Wrap");

constexpr int limit_2k = SCREEN_TARGET_WIDTH - 20;

static std::vector<std::string> WordWrap(StringView line, int limit = limit_2k) {
	std::vector<std::string> lines;
	Game_Message::WordWrap(line, limit, [&](StringView l) { lines.push_back(std::string(l)); });
	return lines;
}


TEST_CASE("Empty") {
	auto lines = WordWrap("");
	REQUIRE_EQ(lines.size(), 0);

	lines = WordWrap("", 0);
	REQUIRE_EQ(lines.size(), 0);

	lines = WordWrap("a", 0);
	REQUIRE_EQ(lines.size(), 1);
	REQUIRE_EQ(lines[0], "a");
}

TEST_CASE("1char") {
	auto lines = WordWrap("a");
	REQUIRE_EQ(lines.size(), 1);
	REQUIRE_EQ(lines[0], "a");
}

TEST_CASE("normal") {
	std::string line = "Skeleton Attacks!";
	auto lines = WordWrap(line);
	REQUIRE_EQ(lines.size(), 1);
	REQUIRE_EQ(lines[0], line);

	line = "Alex takes 300 damage!";
	lines = WordWrap(line);
	REQUIRE_EQ(lines.size(), 1);
	REQUIRE_EQ(lines[0], line);
}

TEST_CASE("toolong") {
	std::string base;
	for (int i = 0; i < 6; ++i) {
		base += "Skeleton";
	}
	auto lines = WordWrap(base + "XYZ");
	REQUIRE_EQ(lines.size(), 1);
	REQUIRE_EQ(lines[0], base + "XYZ");

	lines = WordWrap(base + " XYZ");
	REQUIRE_EQ(lines.size(), 2);
	REQUIRE_EQ(lines[0], base);
	REQUIRE_EQ(lines[1], "XYZ");
}

TEST_CASE("limits") {
	const std::string nstr = "0123456789";
	const std::string limit = nstr + nstr + nstr + nstr + nstr;

	auto lines = WordWrap(limit);
	REQUIRE_EQ(lines.size(), 1);
	REQUIRE_EQ(lines[0], limit);

	auto test = limit + "X";
	lines = WordWrap(test);
	REQUIRE_EQ(lines.size(), 1);
	REQUIRE_EQ(lines[0], test);

	test = limit + limit;
	lines = WordWrap(test);
	REQUIRE_EQ(lines.size(), 1);
	REQUIRE_EQ(lines[0], test);

	test = limit + " " + limit;
	lines = WordWrap(test);
	REQUIRE_EQ(lines.size(), 2);
	REQUIRE_EQ(lines[0], limit);
	REQUIRE_EQ(lines[1], limit);

	test = limit + "  " + limit;
	lines = WordWrap(test);
	REQUIRE_EQ(lines.size(), 2);
	REQUIRE_EQ(lines[0], limit);
	REQUIRE_EQ(lines[1], limit);

	test = limit.substr(0, limit.size()-1) + " " + limit;
	lines = WordWrap(test);
	REQUIRE_EQ(lines.size(), 2);
	REQUIRE_EQ(lines[0], limit.substr(0, limit.size()-1));
	REQUIRE_EQ(lines[1], limit);

}

TEST_SUITE_END();
