#include "filesystem.h"
#include "filefinder.h"
#include "main_data.h"
#include "doctest.h"
#include "player.h"

TEST_SUITE_BEGIN("Filesystem");

TEST_CASE("Create") {
	CHECK(FileFinder::Root().Exists(EP_TEST_PATH "/game"));
	CHECK(!FileFinder::Root().Exists(EP_TEST_PATH "/!!!invalidpath!!!"));
}

TEST_CASE("ListDirectory") {
	auto fs = FileFinder::Root().Subtree(EP_TEST_PATH "/game");
	auto root = fs.ListDirectory();
	CHECK(root->size() == 4);

	for (const auto& it : *root) {
		if (it.first == "charset") {
			CHECK(it.second.name == "Charset");
			CHECK(it.second.type == DirectoryTree::FileType::Directory);
		} else if (it.first == "rpg_rt.ldb") {
			CHECK(it.second.name == "RPG_RT.ldb");
			CHECK(it.second.type == DirectoryTree::FileType::Regular);
		}
	}

	auto charset = fs.ListDirectory("Charset");
	CHECK(charset->size() == 1);
	CHECK((*charset)[0].first == "chara1.png");

	charset = fs.ListDirectory("cHaRsEt");
	CHECK(charset->size() == 1);
	CHECK((*charset)[0].first == "chara1.png");

	CHECK(!fs.ListDirectory("!!!invaliddir!!!"));
}

TEST_CASE("ListDirectorySubtree") {
	auto fs = FileFinder::Root().Subtree(EP_TEST_PATH);
	CHECK(fs.Subtree("gAmE").ListDirectory()->size() == 4);

	auto subtree = fs.Subtree("game");
	auto game = subtree.ListDirectory();
	CHECK(game->size() == 4);

	for (const auto& it : *game) {
		if (it.first == "charset") {
			CHECK(it.second.name == "Charset");
			CHECK(it.second.type == DirectoryTree::FileType::Directory);
		} else if (it.first == "rpg_rt.ldb") {
			CHECK(it.second.name == "RPG_RT.ldb");
			CHECK(it.second.type == DirectoryTree::FileType::Regular);
		}
	}

	CHECK(!subtree.Subtree("!!!invalidpath!!!"));

	Player::escape_symbol = "\\";
	auto charset = subtree.Subtree("charset").ListDirectory();
	CHECK(charset->size() == 1);
	CHECK((*charset)[0].first == "chara1.png");
	Player::escape_symbol = "";
}

TEST_CASE("FindFile") {
	// Only checking the filenames here because FindFile returns absolute paths
	auto fs = FileFinder::Root().Subtree(EP_TEST_PATH "/game");

	auto name = [](const std::string& file) {
		return std::get<1>(FileFinder::GetPathAndFilename(file));
	};

	Player::escape_symbol = "\\";

	CHECK(name(fs.FindFile("rpg_RT.LdB")) == "RPG_RT.ldb");
	CHECK(name(fs.FindFile("charSET/CharA1.png")) == "chara1.png");
	CHECK(fs.FindFile("charSET").empty()); // only files are found
	CHECK(fs.FindFile("!!!nonexistant!!!").empty());
	CHECK(name(fs.FindFile("charSET", "CharA1.png")) == "chara1.png");

	auto IMG_TYPES = Utils::MakeSvArray(".bmp",  ".png", ".xyz");
	CHECK(name(fs.FindFile({ "charSET/charA1", IMG_TYPES })) == "chara1.png");
	CHECK(name(fs.FindFile({ "folder/../charSET/charA1", IMG_TYPES, 1 })) == "chara1.png");
	CHECK(name(fs.FindFile({ "picTures/../exfont", IMG_TYPES, 1 })) == "ExFont.png");

	Player::escape_symbol = "";
}

TEST_SUITE_END();
