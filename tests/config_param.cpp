#include <ostream>
#include "config_param.h"
#include "doctest.h"
#include <climits>

namespace {
enum class TestColor { Red, Blue, Green };
}

template class ConfigParam<int>;
template class ConfigParam<std::string>;

template class LockedConfigParam<int>;
template class LockedConfigParam<std::string>;

template class RangeConfigParam<int>;
template class RangeConfigParam<float>;

template class EnumConfigParam<TestColor, 3>;

TEST_SUITE_BEGIN("ConfigParam");

TEST_CASE("Bool") {
	BoolConfigParam p("Unit Test", "", "", "", false);

	REQUIRE(p.IsOptionVisible());
	REQUIRE(!p.IsLocked());
	REQUIRE_EQ(p.Get(), false);
	REQUIRE(p.IsValid(false));
	REQUIRE(p.IsValid(true));

	p = BoolConfigParam("Unit Test", "", "", "", true);

	REQUIRE(p.IsOptionVisible());
	REQUIRE(!p.IsLocked());
	REQUIRE_EQ(p.Get(), true);
	REQUIRE(p.IsValid(false));
	REQUIRE(p.IsValid(true));

	REQUIRE(p.Set(false));
	REQUIRE_EQ(p.Get(), false);

	REQUIRE(p.Set(true));
	REQUIRE_EQ(p.Get(), true);

	p.Lock(true);

	REQUIRE(p.IsOptionVisible());
	REQUIRE(p.IsLocked());
	REQUIRE_EQ(p.Get(), true);
	REQUIRE(!p.IsValid(false));
	REQUIRE(p.IsValid(true));

	REQUIRE(!p.Set(false));
	REQUIRE_EQ(p.Get(), true);

	p.Lock(false);

	REQUIRE(p.IsOptionVisible());
	REQUIRE(p.IsLocked());
	REQUIRE_EQ(p.Get(), false);
	REQUIRE(p.IsValid(false));
	REQUIRE(!p.IsValid(true));

	REQUIRE(!p.Set(true));
	REQUIRE_EQ(p.Get(), false);

	p.SetOptionVisible(false);
	REQUIRE(!p.IsOptionVisible());
	REQUIRE(p.IsLocked());
	REQUIRE(!p.IsValid(false));
	REQUIRE(!p.IsValid(true));
}

TEST_CASE("Int") {
	ConfigParam<int> p("Unit Test", "", "", "");

	REQUIRE(p.IsOptionVisible());
	REQUIRE(!p.IsLocked());
	REQUIRE(p.IsValid(0));
	REQUIRE_EQ(p.Get(), 0);
	REQUIRE(p.IsValid(INT_MIN));
	REQUIRE(p.IsValid(INT_MAX));

	p.Lock(5);
	REQUIRE(p.IsOptionVisible());
	REQUIRE(p.IsLocked());
	REQUIRE_EQ(p.Get(), 5);
	REQUIRE(!p.IsValid(4));
	REQUIRE(p.IsValid(5));
	REQUIRE(!p.IsValid(6));
	REQUIRE(!p.IsValid(INT_MIN));
	REQUIRE(!p.IsValid(INT_MAX));

	REQUIRE(!p.Set(0));
	REQUIRE_EQ(p.Get(), 5);

	p.SetOptionVisible(false);
	REQUIRE(!p.IsOptionVisible());
	REQUIRE(p.IsLocked());
	REQUIRE(!p.IsValid(0));
	REQUIRE(!p.IsValid(5));
	REQUIRE(!p.IsValid(INT_MIN));
	REQUIRE(!p.IsValid(INT_MAX));

	REQUIRE(!p.Set(0));
}

TEST_CASE("String") {
	StringConfigParam p("Unit Test", "", "", "", "Hello World");

	REQUIRE(p.IsOptionVisible());
	REQUIRE(!p.IsLocked());
	REQUIRE(p.IsValid(""));
	REQUIRE_EQ(p.Get(), "Hello World");

	REQUIRE(p.Set("Alex"));
	REQUIRE_EQ(p.Get(), "Alex");
}

