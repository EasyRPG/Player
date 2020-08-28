#include <ostream>
#include "config_param.h"
#include "doctest.h"
#include <climits>

namespace {
enum class Color { Red, Blue, Green };
}

template class ConfigParam<int>;
template class ConfigParam<std::string>;

template class RangeConfigParam<int>;
template class RangeConfigParam<float>;

template class SetConfigParam<int>;
template class SetConfigParam<float>;
template class SetConfigParam<std::string>;

template class EnumConfigParam<Color>;

static_assert(IsConfigParamT<ConfigParam<int>>::value, "ConfigParam Broken");
static_assert(IsConfigParamT<RangeConfigParam<int>>::value, "ConfigParam Broken");
static_assert(IsConfigParamT<BoolConfigParam>::value, "ConfigParam Broken");
static_assert(IsConfigParamT<SetConfigParam<int>>::value, "ConfigParam Broken");
static_assert(IsConfigParamT<EnumConfigParam<Color>>::value, "ConfigParam Broken");

TEST_SUITE_BEGIN("ConfigParam");

TEST_CASE("Bool") {
	BoolConfigParam p;

	REQUIRE(p.Enabled());
	REQUIRE(!p.Locked());
	REQUIRE_EQ(p.Get(), false);
	REQUIRE(p.IsValid(false));
	REQUIRE(p.IsValid(true));

	p = BoolConfigParam(true);

	REQUIRE(p.Enabled());
	REQUIRE(!p.Locked());
	REQUIRE_EQ(p.Get(), true);
	REQUIRE(p.IsValid(false));
	REQUIRE(p.IsValid(true));

	REQUIRE(p.Set(false));
	REQUIRE_EQ(p.Get(), false);

	REQUIRE(p.Set(true));
	REQUIRE_EQ(p.Get(), true);

	p.Lock(true);

	REQUIRE(p.Enabled());
	REQUIRE(p.Locked());
	REQUIRE_EQ(p.Get(), true);
	REQUIRE(!p.IsValid(false));
	REQUIRE(p.IsValid(true));

	REQUIRE(!p.Set(false));
	REQUIRE_EQ(p.Get(), true);

	p.Lock(false);

	REQUIRE(p.Enabled());
	REQUIRE(p.Locked());
	REQUIRE_EQ(p.Get(), false);
	REQUIRE(p.IsValid(false));
	REQUIRE(!p.IsValid(true));

	REQUIRE(!p.Set(true));
	REQUIRE_EQ(p.Get(), false);

	p.Disable();
	REQUIRE(!p.Enabled());
	REQUIRE(p.Locked());
	REQUIRE(!p.IsValid(false));
	REQUIRE(!p.IsValid(true));
}

TEST_CASE("Int") {
	IntConfigParam p;

	REQUIRE(p.Enabled());
	REQUIRE(!p.Locked());
	REQUIRE(p.IsValid(0));
	REQUIRE_EQ(p.Get(), 0);
	REQUIRE(p.IsValid(INT_MIN));
	REQUIRE(p.IsValid(INT_MAX));

	p.Lock(5);
	REQUIRE(p.Enabled());
	REQUIRE(p.Locked());
	REQUIRE_EQ(p.Get(), 5);
	REQUIRE(!p.IsValid(4));
	REQUIRE(p.IsValid(5));
	REQUIRE(!p.IsValid(6));
	REQUIRE(!p.IsValid(INT_MIN));
	REQUIRE(!p.IsValid(INT_MAX));

	REQUIRE(!p.Set(0));
	REQUIRE_EQ(p.Get(), 5);

	p.Disable();
	REQUIRE(!p.Enabled());
	REQUIRE(p.Locked());
	REQUIRE(!p.IsValid(0));
	REQUIRE(!p.IsValid(5));
	REQUIRE(!p.IsValid(INT_MIN));
	REQUIRE(!p.IsValid(INT_MAX));

	REQUIRE(!p.Set(0));
}

TEST_CASE("String") {
	StringConfigParam p("Hello World");

	REQUIRE(p.Enabled());
	REQUIRE(!p.Locked());
	REQUIRE(p.IsValid(""));
	REQUIRE_EQ(p.Get(), "Hello World");

	REQUIRE(p.Set("Alex"));
	REQUIRE_EQ(p.Get(), "Alex");
}

