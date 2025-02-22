#include "doctest.h"
#include "json_helper.h"
#include <fstream>
#include <nlohmann/json_fwd.hpp>

#ifdef HAVE_NLOHMANN_JSON

TEST_SUITE_BEGIN("JSON");

json load(std::string_view file) {
	std::ifstream is(EP_TEST_PATH "/json/" + std::string(file));
	return json::parse(is, nullptr, false);
}

TEST_CASE("Get Value") {
	auto obj = load("obj.json");
	auto get = [&](std::string_view path) {
		return Json_Helper::GetValue(obj, path);
	};

	CHECK_EQ(get("/name"), "Aina");
	CHECK_EQ(get("/level"), "5");
	CHECK_EQ(get("/active"), "true");
	CHECK_EQ(get("/atk"), "[1,5,10,30,20]");
	CHECK_EQ(get("/skills/0"), R"({"name":"Sword","atk":10})");

	CHECK_EQ(get("/missing"), "");
	CHECK_EQ(get("/atk/missing"), "");
	CHECK_EQ(get("/atk/42"), "");

	obj = load("list.json");
	CHECK_EQ(get("/0"), "1");
	CHECK_EQ(get("/2/name"), "Aina");
}

TEST_CASE("Set Value") {
	auto obj = load("obj.json");

	auto get = [&](std::string_view path) {
		return Json_Helper::GetValue(obj, path);
	};

	auto set = [&](std::string_view path, std::string_view value) {
		Json_Helper::SetValue(obj, path, value);
		CHECK_EQ(get(path), value);
	};

	set("/name", "Easy");
	set("/level", "A");
	set("/active", "false");

	set("/atk/0", "3");
	CHECK_EQ(get("/atk"), "[3,5,10,30,20]");

	set("/missing", "1");
	set("/atk/6", "50");
	CHECK_EQ(get("/atk"), "[3,5,10,30,20,null,50]");

	set("/obj", R"({"key":"value"})");
	CHECK_EQ(get("/obj/key"), "value");
}

TEST_CASE("Get Length") {
	auto obj = load("obj.json");

	auto len = [&](std::string_view path) {
		return Json_Helper::GetLength(obj, path);
	};

	CHECK_EQ(len("/name"), 0);
	CHECK_EQ(len("/atk"), 5);
	CHECK_EQ(len("/skills"), 2);
	CHECK_EQ(len("/missing"), 0);
}

TEST_CASE("Get Keys") {
	auto obj = load("obj.json");

	auto keys = [&](std::string_view path) {
		return Json_Helper::GetKeys(obj, path);
	};

	CHECK_EQ(keys("/name"), std::vector<std::string>());
	CHECK_EQ(keys("/atk"), std::vector<std::string>{"0", "1", "2", "3", "4"});
	CHECK_EQ(keys("/skills/0"), std::vector<std::string>{"name", "atk"});
	CHECK_EQ(keys("/missing"), std::vector<std::string>());
}

TEST_CASE("Get Type") {
	auto obj = load("obj.json");

	auto check = [&](std::string_view path) {
		return Json_Helper::GetType(obj, path);
	};

	CHECK_EQ(check("/name"), "string");
	CHECK_EQ(check("/atk"), "array");
	CHECK_EQ(check("/active"), "boolean");
	CHECK_EQ(check("/level"), "number");
	CHECK_EQ(check("/skills/0"), "object");
	CHECK_EQ(check("/class"), "null");
	CHECK_EQ(check("/missing"), "");
}

TEST_CASE("Get Path") {
	auto obj = load("obj.json");

	auto path = [&](std::string_view path) {
		return Json_Helper::GetPath(obj, path);
	};

	// Function isn't fully implemented yet and not used by the Player
	CHECK_EQ(path("Aina"), "/name");
	//CHECK_EQ(path("5"), "/level");
	//CHECK_EQ(path("10"), "/atk/1");
	CHECK_EQ(path("Sword"), "/skills/0/name");

	CHECK_EQ(Json_Helper::GetPath(obj, "/missing"), "");
}

TEST_CASE("Remove Value") {
	auto obj = load("obj.json");

	auto remove = [&](std::string_view path) {
		Json_Helper::RemoveValue(obj, path);
	};

	{
	auto orig = obj;
	remove("/");
	CHECK_EQ(obj.dump(), orig.dump());
	}

	{
	auto orig = obj;
	remove("/name");
	orig.erase("name");
	CHECK_EQ(obj.dump(), orig.dump());
	}

	{
	auto orig = obj;
	remove("/atk/1");
	orig["atk"].erase(1);
	CHECK_EQ(obj.dump(), orig.dump());
	}

	{
	auto orig = obj;
	remove("/skills/0/name");
	orig["skills"][0].erase("name");
	CHECK_EQ(obj.dump(), orig.dump());
	}
}

TEST_CASE("Push Value") {
	auto obj = load("obj.json");

	auto push = [&](std::string_view path, std::string_view value) {
		Json_Helper::PushValue(obj, path, value);
	};

	auto orig = obj.dump();
	push("/name", "Easy");
	CHECK_EQ(orig, obj.dump());

	push("/atk", "3");
	CHECK_EQ(obj["atk"], std::vector<int>{1,5,10,30,20,3});
}

TEST_CASE("Pop Value") {
	auto obj = load("obj.json");

	auto pop = [&](std::string_view path, bool success = true) {
		auto [json_obj, element] = Json_Helper::PopValue(obj, path);

		CHECK_EQ(!json_obj.empty(), success);

		return element;
	};

	CHECK_EQ(pop("/atk"), "20");
	CHECK_EQ(pop("/atk"), "30");
	CHECK_EQ(pop("/atk"), "10");
	CHECK_EQ(pop("/atk"), "5");
	CHECK_EQ(pop("/atk"), "1");
	CHECK_EQ(pop("/atk", false), "");

	CHECK_EQ(pop("/skills"), R"({"name":"Fireball","atk":20})");

	CHECK_EQ(pop("/name", false), "");
	CHECK_EQ(pop("/missing", false), "");
}

TEST_CASE("Contains") {
	auto obj = load("obj.json");

	auto check = [&](std::string_view path) {
		return Json_Helper::Contains(obj, path);
	};

	CHECK_EQ(check("/name"), true);
	CHECK_EQ(check("/atk"), true);
	CHECK_EQ(check("/skills/0"), true);
	CHECK_EQ(check("/skills/2"), false);
	CHECK_EQ(check("/missing"), false);
}

TEST_SUITE_END();

#endif
