#include "directory_tree.h"
#include "filefinder.h"
#include "main_data.h"
#include "doctest.h"
#include "player.h"

static bool skip_tests() {
#ifdef EMSCRIPTEN
	return true;
#else
	return false;
#endif
}

TEST_SUITE_BEGIN("DirectoryTree" * doctest::skip(skip_tests()));

TEST_CASE("Create") {
	CHECK(DirectoryTree::Create(EP_TEST_PATH "/game"));
	CHECK(!DirectoryTree::Create(EP_TEST_PATH "/!!!invalidpath!!!"));
}

TEST_CASE("Subtree") {
	auto tree = DirectoryTree::Create(EP_TEST_PATH "/game");
	CHECK(tree->Subtree("CharSet"));
	CHECK(!tree->Subtree("!!!invalidpath!!!"));
}

TEST_CASE("ListDirectory") {
	auto tree = DirectoryTree::Create(EP_TEST_PATH "/game");
	auto root = tree->ListDirectory();
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

	auto charset = tree->ListDirectory("Charset");
	CHECK(charset->size() == 1);
	CHECK(charset->find("chara1.png") != charset->end());

	charset = tree->ListDirectory("cHaRsEt");
	CHECK(charset->size() == 1);
	CHECK(charset->find("chara1.png") != charset->end());

	CHECK(!tree->ListDirectory("!!!invaliddir!!!"));
}

TEST_CASE("ListDirectorySubtree") {
	auto tree = DirectoryTree::Create(EP_TEST_PATH);
	CHECK(tree->Subtree("gAmE").ListDirectory()->size() == 4);

	auto subtree = tree->Subtree("game");
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
	CHECK(charset->find("chara1.png") != charset->end());
	Player::escape_symbol = "";
}

TEST_CASE("FindFile") {
	// Only checking the filenames here because FindFile returns absolute paths
	auto tree = DirectoryTree::Create(EP_TEST_PATH "/game");

	auto name = [](const std::string& file) {
		return std::get<1>(FileFinder::GetPathAndFilename(file));
	};

	Player::escape_symbol = "\\";

	CHECK(name(tree->FindFile("rpg_RT.LdB")) == "RPG_RT.ldb");
	CHECK(name(tree->FindFile("charSET/CharA1.png")) == "chara1.png");
	CHECK(tree->FindFile("charSET").empty()); // only files are found
	CHECK(tree->FindFile("!!!nonexistant!!!").empty());
	CHECK(name(tree->FindFile("charSET", "CharA1.png")) == "chara1.png");

	auto IMG_TYPES = Utils::MakeSvArray(".bmp",  ".png", ".xyz");
	CHECK(name(tree->FindFile({ "charSET/charA1", IMG_TYPES })) == "chara1.png");
	CHECK(name(tree->FindFile({ "folder/../charSET/charA1", IMG_TYPES, 1 })) == "chara1.png");
	CHECK(name(tree->FindFile({ "picTures/../exfont", IMG_TYPES, 1 })) == "ExFont.png");

	Player::escape_symbol = "";
}

TEST_SUITE_END();
