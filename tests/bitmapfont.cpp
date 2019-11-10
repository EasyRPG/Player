#include "bitmapfont.h"
#include <algorithm>
#include <iterator>
#include "doctest.h"

TEST_SUITE_BEGIN("BitmapFont");

template <typename T>
bool IsSorted(const T& glyphs) {
	return std::is_sorted(std::begin(glyphs), std::end(glyphs));
}

TEST_CASE("SortedGothic") {
	REQUIRE(IsSorted(SHINONOME_GOTHIC));
}

TEST_CASE("SortedMincho") {
	REQUIRE(IsSorted(SHINONOME_MINCHO));
}

TEST_CASE("SortedWQY") {
	REQUIRE(IsSorted(BITMAPFONT_WQY));
}

TEST_CASE("SortedRMG2000") {
	REQUIRE(IsSorted(BITMAPFONT_RMG2000));
}

TEST_CASE("SortedTTYP0") {
	REQUIRE(IsSorted(BITMAPFONT_TTYP0));
}

TEST_SUITE_END();
