#include "filefinder.h"
#include "player.h"
#include "rtp.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

static std::shared_ptr<FileFinder::DirectoryTree> make_tree() {
	Player::escape_symbol = "\\";

	auto tree = std::make_shared<FileFinder::DirectoryTree>();
	tree->directories["faceset"] = "FaceSet";
	tree->directories["music"] = "Music";
	tree->directories["sound"] = "Sound";

	tree->sub_members["faceset"] = {
			{"主人公1.png", "主人公1.png"}, // Official Japanese
			{"actor2.png", "actor2.png"} // Official English
	};

	tree->sub_members["music"] = {
			{"ダンジョン1.wav", "ダンジョン1.wav"},
			{"ダンジョン2.ogg", "ダンジョン2.ogg"} // Not detected, wrong ending
	};

	tree->sub_members["sound"] = {
			{"カーソル1.wav", "カーソル1.wav"},
			{"キャンセル1.wav", "キャンセル2.wav"}
	};

	return tree;
}

TEST_CASE("RTP 2000: lookup table is correct") {
	for (int i = 0; RTP::rtp_table_2k_categories[i] != nullptr; ++i) {
		const char* category = RTP::rtp_table_2k_categories[i];
		std::pair<int, int> range = {RTP::rtp_table_2k_categories_idx[i], RTP::rtp_table_2k_categories_idx[i+1]};

		for (int j = range.first; j < range.second; ++j) {
			REQUIRE_EQ(strcmp(RTP::rtp_table_2k[j][0], category), 0);
		}
	}
}

TEST_CASE("RTP 2003: lookup table is correct") {
	for (int i = 0; RTP::rtp_table_2k3_categories[i] != nullptr; ++i) {
		const char* category = RTP::rtp_table_2k3_categories[i];
		std::pair<int, int> range = {RTP::rtp_table_2k3_categories_idx[i], RTP::rtp_table_2k3_categories_idx[i+1]};

		for (int j = range.first; j < range.second; ++j) {
			REQUIRE_EQ(strcmp(RTP::rtp_table_2k3[j][0], category), 0);
		}
	}
}

TEST_CASE("RTP 2000: Detection") {
	auto tree = make_tree();
	std::vector<RTP::RtpHitInfo> hits = RTP::Detect(tree, 2000);

	REQUIRE(hits.size() == 2);

	REQUIRE(hits[0].type == RTP::Type::RPG2000_OfficialJapanese);
	REQUIRE(hits[0].hits == 4);
	REQUIRE(hits[0].tree == tree);

	REQUIRE(hits[1].type == RTP::Type::RPG2000_OfficialEnglish);
	REQUIRE(hits[1].hits == 1);
}

TEST_CASE("RTP 2003: Detection") {
	std::vector<RTP::RtpHitInfo> hits = RTP::Detect(make_tree(), 2003);

	REQUIRE(hits.size() == 2);

	REQUIRE(hits[0].type == RTP::Type::RPG2003_OfficialJapanese);
	REQUIRE(hits[0].hits == 4);

	REQUIRE(hits[1].type == RTP::Type::RPG2003_OfficialEnglish);
	REQUIRE(hits[1].hits == 1);
}

TEST_CASE("RTP 2000: Lookup Any to RTP with 1 hit") {
	auto types = RTP::LookupAnyToRtp("faceset", "actor1", 2000);

	REQUIRE(types.size() == 1);
	REQUIRE(types[0] == RTP::Type::RPG2000_OfficialEnglish);
}

TEST_CASE("RTP 2000: Lookup Any to RTP with 2 hits") {
	auto types = RTP::LookupAnyToRtp("music", "dungeon1", 2000);

	REQUIRE(types.size() == 2);
	REQUIRE(types[0] == RTP::Type::RPG2000_OfficialEnglish);
	REQUIRE(types[1] == RTP::Type::RPG2000_DonMiguelEnglish);
}

TEST_CASE("RTP 2003: Lookup Any to RTP with 1 hit") {
	auto types = RTP::LookupAnyToRtp("faceset", "actor1", 2003);

	REQUIRE(types.size() == 1);
	REQUIRE(types[0] == RTP::Type::RPG2003_OfficialEnglish);
}

TEST_CASE("RTP 2000: Lookup RTP to RTP (Found)") {
	bool is_rtp_asset;

	std::string name = RTP::LookupRtpToRtp("faceset", "主人公2", RTP::Type::RPG2000_OfficialJapanese, RTP::Type::RPG2000_OfficialEnglish, &is_rtp_asset);
	REQUIRE(name == "actor2");
	REQUIRE(is_rtp_asset);
}

TEST_CASE("RTP 2003: Lookup RTP to RTP (Found)") {
	bool is_rtp_asset;

	std::string name = RTP::LookupRtpToRtp("faceset", "主人公2", RTP::Type::RPG2003_OfficialJapanese, RTP::Type::RPG2003_OfficialEnglish, &is_rtp_asset);
	REQUIRE(name == "actor2");
	REQUIRE(is_rtp_asset);
}

TEST_CASE("RTP 2000: Lookup RTP to RTP (Not found)") {
	bool is_rtp_asset;

	std::string name = RTP::LookupRtpToRtp("faceset", "NotFound", RTP::Type::RPG2000_OfficialJapanese, RTP::Type::RPG2000_OfficialEnglish, &is_rtp_asset);
	REQUIRE(name.empty());
	REQUIRE(!is_rtp_asset);
}

TEST_CASE("RTP 2000: Lookup RTP to RTP (Same RTP)") {
	// For performance reasons same to same does no table scan, update this test when the behaviour changes
	bool is_rtp_asset;

	std::string name = RTP::LookupRtpToRtp("faceset", "actor2", RTP::Type::RPG2000_OfficialEnglish, RTP::Type::RPG2000_OfficialEnglish, &is_rtp_asset);

	REQUIRE(name == "actor2");
	REQUIRE(!is_rtp_asset);

	name = RTP::LookupRtpToRtp("faceset", "NotFound", RTP::Type::RPG2000_OfficialEnglish, RTP::Type::RPG2000_OfficialEnglish, &is_rtp_asset);
	REQUIRE(name == "NotFound");
	REQUIRE(!is_rtp_asset);
}
