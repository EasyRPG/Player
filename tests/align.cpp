#include "align.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

static_assert(IsPow2(1), "IsPow2 broken");
static_assert(IsPow2(2), "IsPow2 broken");
static_assert(!IsPow2(3), "IsPow2 broken");
static_assert(IsPow2(4), "IsPow2 broken");
static_assert(!IsPow2(5), "IsPow2 broken");
static_assert(!IsPow2(6), "IsPow2 broken");
static_assert(!IsPow2(7), "IsPow2 broken");
static_assert(IsPow2(8), "IsPow2 broken");

TEST_CASE("IsPow2") {
	REQUIRE(IsPow2(0));
	int next = 1;
	for (int i = 1; i < 1000; ++i) {
		INFO("val=" << i << " next=" << next);
		if (i == next) {
			REQUIRE(IsPow2(i));
			next *= 2;
		} else {
			REQUIRE_FALSE(IsPow2(i));
		}
	}
}

TEST_CASE("AlignUp") {
	for(int a = 1; a < 64; a *= 2) {
		INFO("val=0 align=" << a);
		REQUIRE_EQ(AlignUp(0, a), 0);
	}
	for (int i = 1; i < 64; ++i) {
		for(int a = 1; a < 64; a *= 2) {
			INFO("val=" << i << " align=" << a);
			REQUIRE_EQ(AlignUp(i, a), (i % a) == 0 ? i : (i - (i % a) + a));
		}
	}
}

TEST_CASE("AlignDown") {
	for(int a = 1; a < 64; a *= 2) {
		INFO("val=0 align=" << a);
		REQUIRE_EQ(AlignDown(0, a), 0);
	}
	for (int i = 1; i < 64; ++i) {
		for(int a = 1; a < 64; a *= 2) {
			INFO("val=" << i << " align=" << a);
			REQUIRE_EQ(AlignDown(i, a), (i - (i % a)));
		}
	}
}

TEST_CASE("IsAligned") {
	for(int a = 1; a < 64; a *= 2) {
		INFO("val=0 align=" << a);
		REQUIRE(IsAligned(0, a));
	}
	for (int i = 1; i < 64; ++i) {
		for(int a = 1; a < 64; a *= 2) {
			INFO("val=" << i << " align=" << a);
			if ((i % a) == 0) {
				REQUIRE(IsAligned(i, a));
			} else {
				REQUIRE(!IsAligned(i, a));
			}
		}
	}
}

