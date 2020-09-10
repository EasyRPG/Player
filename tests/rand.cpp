#include "rand.h"
#include "doctest.h"

static void testGetRandomNumber(int32_t a, int32_t b) {
	for (int i = 0; i < 1000; ++i) {
		auto x = Rand::GetRandomNumber(a, b);
		REQUIRE_GE(x, a);
		REQUIRE_LE(x, b);
	}
}

TEST_SUITE_BEGIN("Rand");

TEST_CASE("GetRandomNumber") {
	testGetRandomNumber(0, 43);
	testGetRandomNumber(-21, 31);
	testGetRandomNumber(0, 0);
	testGetRandomNumber(5, 5);
	testGetRandomNumber(-5, -2);
}

TEST_CASE("Lock") {
	REQUIRE_FALSE(Rand::GetRandomLocked().first);

	Rand::LockRandom(55);

	REQUIRE(Rand::GetRandomLocked().first);
	REQUIRE_EQ(Rand::GetRandomLocked().second, 55);

	Rand::UnlockRandom();

	REQUIRE_FALSE(Rand::GetRandomLocked().first);
}

TEST_CASE("LockGuardNest") {
	REQUIRE_FALSE(Rand::GetRandomLocked().first);
	{
		Rand::LockGuard fg(32);
		REQUIRE(fg.Enabled());

		REQUIRE(Rand::GetRandomLocked().first);
		REQUIRE_EQ(Rand::GetRandomLocked().second, 32);

		{
			Rand::LockGuard fg(0, false);
			REQUIRE(fg.Enabled());

			REQUIRE_FALSE(Rand::GetRandomLocked().first);
		}

		REQUIRE(Rand::GetRandomLocked().first);
		REQUIRE_EQ(Rand::GetRandomLocked().second, 32);
	}
	REQUIRE_FALSE(Rand::GetRandomLocked().first);
}

TEST_CASE("LockGuardRelease") {
	REQUIRE_FALSE(Rand::GetRandomLocked().first);

	Rand::LockGuard fg(-16);
	REQUIRE(fg.Enabled());

	REQUIRE(Rand::GetRandomLocked().first);
	REQUIRE_EQ(Rand::GetRandomLocked().second, -16);

	fg.Release();

	REQUIRE_FALSE(Rand::GetRandomLocked().first);
	REQUIRE_FALSE(fg.Enabled());
}

TEST_CASE("LockGuardDismiss") {
	Rand::LockGuard fg(INT32_MAX);
	REQUIRE(fg.Enabled());

	REQUIRE(Rand::GetRandomLocked().first);
	REQUIRE_EQ(Rand::GetRandomLocked().second, INT32_MAX);

	fg.Dismiss();

	REQUIRE(Rand::GetRandomLocked().first);
	REQUIRE_EQ(Rand::GetRandomLocked().second, INT32_MAX);

	Rand::UnlockRandom();
}

static void testGetRandomNumberFixed(int32_t a, int32_t b, int32_t fix, int32_t result) {
	Rand::LockGuard fg(fix);
	for (int i = 0; i < 10; ++i) {
		auto x = Rand::GetRandomNumber(a, b);
		REQUIRE_EQ(x, result);
	}
}

TEST_CASE("GetRandomNumberFixed") {
	testGetRandomNumberFixed(-10, 12, 5, 5);
	testGetRandomNumberFixed(-10, 12, 12, 12);
	testGetRandomNumberFixed(-10, 12, 55, 12);
	testGetRandomNumberFixed(-10, 12, INT32_MIN, -10);
	testGetRandomNumberFixed(-10, 12, INT32_MAX, 12);
}

TEST_SUITE_END();
