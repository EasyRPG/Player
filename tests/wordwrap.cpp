#include <gtest/gtest.h>
#include "game_message.h"
#include "options.h"

constexpr int limit_2k = SCREEN_TARGET_WIDTH - 20;

static std::vector<std::string> WordWrap(const std::string& line, int limit = limit_2k) {
	std::vector<std::string> lines;
	Game_Message::WordWrap(line, limit, [&](const std::string& l) { lines.push_back(l); });
	return lines;
}


TEST(WordWrapTest, Empty) {
	auto lines = WordWrap("");
	ASSERT_EQ(lines.size(), 0);

	lines = WordWrap("", 0);
	ASSERT_EQ(lines.size(), 0);

	lines = WordWrap("a", 0);
	ASSERT_EQ(lines.size(), 1);
	ASSERT_EQ(lines[0], "a");
}

TEST(WordWrapTest, 1char) {
	auto lines = WordWrap("a");
	ASSERT_EQ(lines.size(), 1);
	ASSERT_EQ(lines[0], "a");
}

TEST(WordWrapTest, normal) {
	std::string line = "Skeleton Attacks!";
	auto lines = WordWrap(line);
	ASSERT_EQ(lines.size(), 1);
	ASSERT_EQ(lines[0], line);

	line = "Alex takes 300 damage!";
	lines = WordWrap(line);
	ASSERT_EQ(lines.size(), 1);
	ASSERT_EQ(lines[0], line);
}

TEST(WordWrapTest, toolong) {
	std::string base;
	for (int i = 0; i < 6; ++i) {
		base += "Skeleton";
	}
	auto lines = WordWrap(base + "XYZ");
	ASSERT_EQ(lines.size(), 1);
	ASSERT_EQ(lines[0], base + "XYZ");

	lines = WordWrap(base + " XYZ");
	ASSERT_EQ(lines.size(), 2);
	ASSERT_EQ(lines[0], base);
	ASSERT_EQ(lines[1], "XYZ");
}

TEST(WordWrapTest, limits) {
	const std::string nstr = "0123456789";
	const std::string limit = nstr + nstr + nstr + nstr + nstr;

	auto lines = WordWrap(limit);
	ASSERT_EQ(lines.size(), 1);
	ASSERT_EQ(lines[0], limit);

	auto test = limit + "X";
	lines = WordWrap(test);
	ASSERT_EQ(lines.size(), 1);
	ASSERT_EQ(lines[0], test);

	test = limit + limit;
	lines = WordWrap(test);
	ASSERT_EQ(lines.size(), 1);
	ASSERT_EQ(lines[0], test);

	test = limit + " " + limit;
	lines = WordWrap(test);
	ASSERT_EQ(lines.size(), 2);
	ASSERT_EQ(lines[0], limit);
	ASSERT_EQ(lines[1], limit);

	test = limit + "  " + limit;
	lines = WordWrap(test);
	ASSERT_EQ(lines.size(), 2);
	ASSERT_EQ(lines[0], limit);
	ASSERT_EQ(lines[1], limit);

	test = limit.substr(0, limit.size()-1) + " " + limit;
	lines = WordWrap(test);
	ASSERT_EQ(lines.size(), 2);
	ASSERT_EQ(lines[0], limit.substr(0, limit.size()-1));
	ASSERT_EQ(lines[1], limit);

}
