#include "flat_map.h"
#include "doctest.h"
#include <climits>

using IntUMMap = FlatUniqueMultiMap<int,int>;

TEST_SUITE_BEGIN("FlatUniqueMultiMap");

TEST_CASE("DefaultConstruct") {
	IntUMMap mp;
	const auto& cmp = mp;

	REQUIRE_EQ(mp.size(), 0);
	REQUIRE(mp.empty());
	REQUIRE_EQ(mp.begin(), mp.end());
	REQUIRE_EQ(cmp.begin(), cmp.end());
}

TEST_CASE("InitListSorted") {
	IntUMMap mp = {
		{ 0, 0 },
		{ 0, 1 },
		{ 1, -1 },
		{ 1, 0, },
		{ 1, 1, }
	};
	const auto& cmp = mp;

	REQUIRE_EQ(mp.size(), 5);
	REQUIRE_FALSE(mp.empty());
	REQUIRE_NE(mp.begin(), mp.end());
	REQUIRE_NE(cmp.begin(), cmp.end());

	REQUIRE(std::is_sorted(mp.begin(), mp.end()));
	REQUIRE(std::is_sorted(cmp.begin(), cmp.end()));

	REQUIRE(mp.Has(0));
	REQUIRE(mp.Has(1));

	REQUIRE(mp.Has({0, 0}));
	REQUIRE(mp.Has({ 0, 1 }));
	REQUIRE(mp.Has({ 1, -1 }));
	REQUIRE(mp.Has({ 1, 0, }));
	REQUIRE(mp.Has({ 1, 1, }));
}

TEST_CASE("InitListUnSorted") {
	IntUMMap mp = {
		{ 1, 1, },
		{ 0, 1 },
		{ 1, 0, },
		{ 0, 0 },
		{ 1, -1 }
	};
	const auto& cmp = mp;

	REQUIRE_EQ(mp.size(), 5);
	REQUIRE_FALSE(mp.empty());
	REQUIRE_NE(mp.begin(), mp.end());
	REQUIRE_NE(cmp.begin(), cmp.end());

	REQUIRE(std::is_sorted(mp.begin(), mp.end()));
	REQUIRE(std::is_sorted(cmp.begin(), cmp.end()));

	REQUIRE(mp.Has(0));
	REQUIRE(mp.Has(1));

	REQUIRE(mp.Has({0, 0}));
	REQUIRE(mp.Has({ 0, 1 }));
	REQUIRE(mp.Has({ 1, -1 }));
	REQUIRE(mp.Has({ 1, 0, }));
	REQUIRE(mp.Has({ 1, 1, }));
}

TEST_CASE("InitListDupes") {
	IntUMMap mp = {
		{ 1, 1, },
		{ 1, 1 },
		{ 2, 2, },
		{ 2, 2 },
	};

	REQUIRE_EQ(mp.size(), 2);
	REQUIRE_FALSE(mp.empty());
	REQUIRE_NE(mp.begin(), mp.end());

	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE(mp.Has(1));
	REQUIRE(mp.Has(2));

	REQUIRE(mp.Has({ 1, 1, }));
	REQUIRE(mp.Has({ 2, 2, }));
}



TEST_CASE("Add") {
	IntUMMap mp;

	REQUIRE_EQ(mp.size(), 0);

	REQUIRE(mp.Add(std::make_pair(1, 3)));
	REQUIRE_EQ(mp.size(), 1);
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE(!mp.Has(0));
	REQUIRE(mp.Has(1));
	REQUIRE(!mp.Has(2));

	REQUIRE(!mp.Has({1, 2}));
	REQUIRE(mp.Has({1, 3}));
	REQUIRE(!mp.Has({1, 4}));

	REQUIRE_FALSE(mp.Add(std::make_pair(1, 3)));
	REQUIRE_EQ(mp.size(), 1);
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE(!mp.Has(0));
	REQUIRE(mp.Has(1));
	REQUIRE(!mp.Has(2));

	REQUIRE(!mp.Has({1, 2}));
	REQUIRE(mp.Has({1, 3}));
	REQUIRE(!mp.Has({1, 4}));

	REQUIRE(mp.Add(std::make_pair(2, 2)));
	REQUIRE_EQ(mp.size(), 2);
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE(!mp.Has(0));
	REQUIRE(mp.Has(1));
	REQUIRE(mp.Has(2));
	REQUIRE(!mp.Has(3));

	REQUIRE(!mp.Has({1, 2}));
	REQUIRE(mp.Has({1, 3}));
	REQUIRE(!mp.Has({1, 4}));

	REQUIRE(!mp.Has({2, 1}));
	REQUIRE(mp.Has({2, 2}));
	REQUIRE(!mp.Has({2, 3}));

	REQUIRE_FALSE(mp.Add(std::make_pair(2, 2)));
	REQUIRE_EQ(mp.size(), 2);
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE(!mp.Has(0));
	REQUIRE(mp.Has(1));
	REQUIRE(mp.Has(2));
	REQUIRE(!mp.Has(3));

	REQUIRE(!mp.Has({1, 2}));
	REQUIRE(mp.Has({1, 3}));
	REQUIRE(!mp.Has({1, 4}));

	REQUIRE(!mp.Has({2, 1}));
	REQUIRE(mp.Has({2, 2}));
	REQUIRE(!mp.Has({2, 3}));
}

TEST_CASE("Remove") {
	IntUMMap mp = {
		{ 1, 1 },
		{ 1, 2 },
		{ 2, 1 },
		{ 2, 2 }
	};

	REQUIRE_EQ(mp.size(), 4);

	REQUIRE_FALSE(mp.Remove({1, 0}));
	REQUIRE_EQ(mp.size(), 4);
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE(mp.Has({1, 1}));
	REQUIRE(mp.Has({1, 2}));
	REQUIRE(mp.Has({2, 1}));
	REQUIRE(mp.Has({2, 2}));

	REQUIRE(mp.Remove({1, 2}));
	REQUIRE_EQ(mp.size(), 3);
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE(mp.Has({1, 1}));
	REQUIRE(!mp.Has({1, 2}));
	REQUIRE(mp.Has({2, 1}));
	REQUIRE(mp.Has({2, 2}));

	REQUIRE_FALSE(mp.Remove({1, 2}));
	REQUIRE_EQ(mp.size(), 3);
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE(mp.Has({1, 1}));
	REQUIRE(!mp.Has({1, 2}));
	REQUIRE(mp.Has({2, 1}));
	REQUIRE(mp.Has({2, 2}));

	REQUIRE(mp.Remove({2, 2}));
	REQUIRE_EQ(mp.size(), 2);
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE(mp.Has({1, 1}));
	REQUIRE(!mp.Has({1, 2}));
	REQUIRE(mp.Has({2, 1}));
	REQUIRE(!mp.Has({2, 2}));

	REQUIRE(mp.Remove({1, 1}));
	REQUIRE_EQ(mp.size(), 1);
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE(!mp.Has({1, 1}));
	REQUIRE(!mp.Has({1, 2}));
	REQUIRE(mp.Has({2, 1}));
	REQUIRE(!mp.Has({2, 2}));

	REQUIRE(mp.Remove({2, 1}));
	REQUIRE_EQ(mp.size(), 0);
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE(!mp.Has({1, 1}));
	REQUIRE(!mp.Has({1, 2}));
	REQUIRE(!mp.Has({2, 1}));
	REQUIRE(!mp.Has({2, 2}));
}

TEST_CASE("RemoveAll") {
	IntUMMap mp = {
		{ 1, 0 },
		{ 1, 2 },
		{ 2, 1 },
		{ 2, 3 }
	};

	REQUIRE_EQ(mp.size(), 4);

	REQUIRE_EQ(mp.RemoveAll(0), 0);
	REQUIRE_EQ(mp.size(), 4);
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE(mp.Has({1, 0}));
	REQUIRE(mp.Has({1, 2}));
	REQUIRE(mp.Has({2, 1}));
	REQUIRE(mp.Has({2, 3}));

	REQUIRE_EQ(mp.RemoveAll(3), 0);
	REQUIRE_EQ(mp.size(), 4);
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE(mp.Has({1, 0}));
	REQUIRE(mp.Has({1, 2}));
	REQUIRE(mp.Has({2, 1}));
	REQUIRE(mp.Has({2, 3}));

	REQUIRE_EQ(mp.RemoveAll(2), 2);
	REQUIRE_EQ(mp.size(), 2);
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE(mp.Has({1, 0}));
	REQUIRE(mp.Has({1, 2}));
	REQUIRE(!mp.Has({2, 1}));
	REQUIRE(!mp.Has({2, 3}));

	REQUIRE_EQ(mp.RemoveAll(2), 0);
	REQUIRE_EQ(mp.size(), 2);
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE(mp.Has({1, 0}));
	REQUIRE(mp.Has({1, 2}));
	REQUIRE(!mp.Has({2, 1}));
	REQUIRE(!mp.Has({2, 3}));

	REQUIRE_EQ(mp.RemoveAll(1), 2);
	REQUIRE_EQ(mp.size(), 0);
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE(!mp.Has({1, 1}));
	REQUIRE(!mp.Has({1, 2}));
	REQUIRE(!mp.Has({2, 1}));
	REQUIRE(!mp.Has({2, 2}));
}

TEST_CASE("ReplaceAllWithSameSize") {
	IntUMMap mp = {
		{ 1, 0 },
		{ 1, 2 },
		{ 2, 1 },
		{ 2, 3 },
		{ 3, 1 },
		{ 3, 4 }
	};

	REQUIRE_EQ(mp.size(), 6);

	std::array<int,2> replace = {{0, 2}};

	mp.ReplaceAll(2, replace.begin(), replace.end());

	REQUIRE_EQ(mp.size(), 6);
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE(mp.Has({ 1, 0 }));
	REQUIRE(mp.Has({ 1, 2 }));
	REQUIRE(!mp.Has({ 2, 1 }));
	REQUIRE(!mp.Has({ 2, 3 }));
	REQUIRE(mp.Has({ 3, 1 }));
	REQUIRE(mp.Has({ 3, 4 }));

	REQUIRE(mp.Has({ 2, 0 }));
	REQUIRE(mp.Has({ 2, 2 }));

	mp.ReplaceAll(3, replace.begin(), replace.end());

	REQUIRE_EQ(mp.size(), 6);
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE(mp.Has({ 1, 0 }));
	REQUIRE(mp.Has({ 1, 2 }));
	REQUIRE(!mp.Has({ 2, 1 }));
	REQUIRE(!mp.Has({ 2, 3 }));
	REQUIRE(!mp.Has({ 3, 1 }));
	REQUIRE(!mp.Has({ 3, 4 }));

	REQUIRE(mp.Has({ 2, 0 }));
	REQUIRE(mp.Has({ 2, 2 }));
	REQUIRE(mp.Has({ 3, 0 }));
	REQUIRE(mp.Has({ 3, 2 }));

	mp.ReplaceAll(1, replace.begin(), replace.end());

	REQUIRE_EQ(mp.size(), 6);
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE(mp.Has({ 1, 0 }));
	REQUIRE(mp.Has({ 1, 2 }));
	REQUIRE(!mp.Has({ 2, 1 }));
	REQUIRE(!mp.Has({ 2, 3 }));
	REQUIRE(!mp.Has({ 3, 1 }));
	REQUIRE(!mp.Has({ 3, 4 }));

	REQUIRE(mp.Has({ 2, 0 }));
	REQUIRE(mp.Has({ 2, 2 }));
	REQUIRE(mp.Has({ 3, 0 }));
	REQUIRE(mp.Has({ 3, 2 }));
}

TEST_CASE("ReplaceAllWithMore") {
	IntUMMap mp = {
		{ 1, 0 },
		{ 1, 2 },
		{ 2, 1 },
		{ 2, 3 },
		{ 3, 1 },
		{ 3, 4 }
	};

	REQUIRE_EQ(mp.size(), 6);

	std::array<int,3> replace = {{0, 2, 5}};

	mp.ReplaceAll(2, replace.begin(), replace.end());

	REQUIRE_EQ(mp.size(), 7);
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE(mp.Has({ 1, 0 }));
	REQUIRE(mp.Has({ 1, 2 }));
	REQUIRE(!mp.Has({ 2, 1 }));
	REQUIRE(!mp.Has({ 2, 3 }));
	REQUIRE(mp.Has({ 3, 1 }));
	REQUIRE(mp.Has({ 3, 4 }));

	REQUIRE(mp.Has({ 2, 0 }));
	REQUIRE(mp.Has({ 2, 2 }));
	REQUIRE(mp.Has({ 2, 5 }));

	mp.ReplaceAll(3, replace.begin(), replace.end());

	REQUIRE_EQ(mp.size(), 8);
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE(mp.Has({ 1, 0 }));
	REQUIRE(mp.Has({ 1, 2 }));
	REQUIRE(!mp.Has({ 2, 1 }));
	REQUIRE(!mp.Has({ 2, 3 }));
	REQUIRE(!mp.Has({ 3, 1 }));
	REQUIRE(!mp.Has({ 3, 4 }));

	REQUIRE(mp.Has({ 2, 0 }));
	REQUIRE(mp.Has({ 2, 2 }));
	REQUIRE(mp.Has({ 2, 5 }));
	REQUIRE(mp.Has({ 3, 0 }));
	REQUIRE(mp.Has({ 3, 2 }));
	REQUIRE(mp.Has({ 3, 5 }));

	mp.ReplaceAll(1, replace.begin(), replace.end());

	REQUIRE_EQ(mp.size(), 9);
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE(mp.Has({ 1, 0 }));
	REQUIRE(mp.Has({ 1, 2 }));
	REQUIRE(!mp.Has({ 2, 1 }));
	REQUIRE(!mp.Has({ 2, 3 }));
	REQUIRE(!mp.Has({ 3, 1 }));
	REQUIRE(!mp.Has({ 3, 4 }));

	REQUIRE(mp.Has({ 2, 0 }));
	REQUIRE(mp.Has({ 2, 2 }));
	REQUIRE(mp.Has({ 2, 5 }));
	REQUIRE(mp.Has({ 3, 0 }));
	REQUIRE(mp.Has({ 3, 2 }));
	REQUIRE(mp.Has({ 3, 5 }));
	REQUIRE(mp.Has({ 1, 5 }));
}

TEST_CASE("ReplaceAllWithLess") {
	IntUMMap mp = {
		{ 1, 0 },
		{ 1, 2 },
		{ 2, 1 },
		{ 2, 3 },
		{ 3, 1 },
		{ 3, 4 }
	};

	REQUIRE_EQ(mp.size(), 6);

	std::array<int,1> replace = {{ 0 }};

	mp.ReplaceAll(2, replace.begin(), replace.end());
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE_EQ(mp.size(), 5);

	REQUIRE(mp.Has({ 1, 0 }));
	REQUIRE(mp.Has({ 1, 2 }));
	REQUIRE(!mp.Has({ 2, 1 }));
	REQUIRE(!mp.Has({ 2, 3 }));
	REQUIRE(mp.Has({ 3, 1 }));
	REQUIRE(mp.Has({ 3, 4 }));

	REQUIRE(mp.Has({ 2, 0 }));

	mp.ReplaceAll(3, replace.begin(), replace.end());
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE_EQ(mp.size(), 4);

	REQUIRE(mp.Has({ 1, 0 }));
	REQUIRE(mp.Has({ 1, 2 }));
	REQUIRE(!mp.Has({ 2, 1 }));
	REQUIRE(!mp.Has({ 2, 3 }));
	REQUIRE(!mp.Has({ 3, 1 }));
	REQUIRE(!mp.Has({ 3, 4 }));

	REQUIRE(mp.Has({ 2, 0 }));
	REQUIRE(mp.Has({ 3, 0 }));

	mp.ReplaceAll(1, replace.begin(), replace.end());
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE_EQ(mp.size(), 3);

	REQUIRE(mp.Has({ 1, 0 }));
	REQUIRE(!mp.Has({ 1, 2 }));
	REQUIRE(!mp.Has({ 2, 1 }));
	REQUIRE(!mp.Has({ 2, 3 }));
	REQUIRE(!mp.Has({ 3, 1 }));
	REQUIRE(!mp.Has({ 3, 4 }));

	REQUIRE(mp.Has({ 2, 0 }));
	REQUIRE(mp.Has({ 3, 0 }));
}

TEST_CASE("ReplaceAllWithNothing") {
	IntUMMap mp = {
		{ 1, 0 },
		{ 1, 2 },
		{ 2, 1 },
		{ 2, 3 },
		{ 3, 1 },
		{ 3, 4 }
	};

	REQUIRE_EQ(mp.size(), 6);

	std::vector<int> replace = {};

	mp.ReplaceAll(2, replace.begin(), replace.end());

	REQUIRE_EQ(mp.size(), 4);
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE(mp.Has({ 1, 0 }));
	REQUIRE(mp.Has({ 1, 2 }));
	REQUIRE(!mp.Has({ 2, 1 }));
	REQUIRE(!mp.Has({ 2, 3 }));
	REQUIRE(mp.Has({ 3, 1 }));
	REQUIRE(mp.Has({ 3, 4 }));

	mp.ReplaceAll(3, replace.begin(), replace.end());
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE_EQ(mp.size(), 2);

	REQUIRE(mp.Has({ 1, 0 }));
	REQUIRE(mp.Has({ 1, 2 }));
	REQUIRE(!mp.Has({ 2, 1 }));
	REQUIRE(!mp.Has({ 2, 3 }));
	REQUIRE(!mp.Has({ 3, 1 }));
	REQUIRE(!mp.Has({ 3, 4 }));

	mp.ReplaceAll(1, replace.begin(), replace.end());
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE_EQ(mp.size(), 0);

	REQUIRE(!mp.Has({ 1, 0 }));
	REQUIRE(!mp.Has({ 1, 2 }));
	REQUIRE(!mp.Has({ 2, 1 }));
	REQUIRE(!mp.Has({ 2, 3 }));
	REQUIRE(!mp.Has({ 3, 1 }));
	REQUIRE(!mp.Has({ 3, 4 }));
}

TEST_CASE("ReplaceAllWithDupes") {
	IntUMMap mp = {
		{ 1, 0 },
		{ 1, 2 },
		{ 2, 1 },
		{ 2, 3 },
		{ 3, 1 },
		{ 3, 4 }
	};

	REQUIRE_EQ(mp.size(), 6);

	std::array<int,2> replace = {{ 0, 0 }};

	mp.ReplaceAll(2, replace.begin(), replace.end());
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE_EQ(mp.size(), 5);

	REQUIRE(mp.Has({ 1, 0 }));
	REQUIRE(mp.Has({ 1, 2 }));
	REQUIRE(!mp.Has({ 2, 1 }));
	REQUIRE(!mp.Has({ 2, 3 }));
	REQUIRE(mp.Has({ 3, 1 }));
	REQUIRE(mp.Has({ 3, 4 }));

	REQUIRE(mp.Has({ 2, 0 }));

	mp.ReplaceAll(3, replace.begin(), replace.end());
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE_EQ(mp.size(), 4);

	REQUIRE(mp.Has({ 1, 0 }));
	REQUIRE(mp.Has({ 1, 2 }));
	REQUIRE(!mp.Has({ 2, 1 }));
	REQUIRE(!mp.Has({ 2, 3 }));
	REQUIRE(!mp.Has({ 3, 1 }));
	REQUIRE(!mp.Has({ 3, 4 }));

	REQUIRE(mp.Has({ 2, 0 }));
	REQUIRE(mp.Has({ 3, 0 }));

	mp.ReplaceAll(1, replace.begin(), replace.end());
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE_EQ(mp.size(), 3);

	REQUIRE(mp.Has({ 1, 0 }));
	REQUIRE(!mp.Has({ 1, 2 }));
	REQUIRE(!mp.Has({ 2, 1 }));
	REQUIRE(!mp.Has({ 2, 3 }));
	REQUIRE(!mp.Has({ 3, 1 }));
	REQUIRE(!mp.Has({ 3, 4 }));

	REQUIRE(mp.Has({ 2, 0 }));
	REQUIRE(mp.Has({ 3, 0 }));
}


TEST_CASE("ReplaceAllWithNew") {
	IntUMMap mp = {
		{ 1, 0 },
		{ 1, 2 },
		{ 2, 1 },
		{ 2, 3 },
		{ 3, 1 },
		{ 3, 4 }
	};

	REQUIRE_EQ(mp.size(), 6);

	std::array<int,2> replace = {{ 7, 8 }};

	mp.ReplaceAll(9, replace.begin(), replace.end());
	REQUIRE(std::is_sorted(mp.begin(), mp.end()));

	REQUIRE_EQ(mp.size(), 8);

	REQUIRE(mp.Has({ 1, 0 }));
	REQUIRE(mp.Has({ 1, 2 }));
	REQUIRE(mp.Has({ 2, 1 }));
	REQUIRE(mp.Has({ 2, 3 }));
	REQUIRE(mp.Has({ 3, 1 }));
	REQUIRE(mp.Has({ 3, 4 }));

	REQUIRE(mp.Has({ 9, 7 }));
	REQUIRE(mp.Has({ 9, 8 }));
}

TEST_CASE("Count") {
	IntUMMap mp = {
		{ 1, 0 },
		{ 1, 2 },
		{ 2, 1 },
		{ 2, 3 },
		{ 2, 4 },
		{ 3, 1 }
	};

	REQUIRE(mp.Count(1) == 2);
	REQUIRE(mp.Count(2) == 3);
	REQUIRE(mp.Count(3) == 1);
	REQUIRE(mp.Count(4) == 0);
}

#if 0

TEST_CASE("ButtonMappingArrayAddRemove") {
	ButtonMappingArray bma;
	ButtonMapping bm;

	bm = {N0, Keys::N0};
	REQUIRE(bma.Add(bm));
	REQUIRE(bma.Has(bm));
	REQUIRE_EQ(bma.size(), 1);
	REQUIRE(std::is_sorted(bma.begin(), bma.end()));

	bm = {N0, Keys::N0};
	REQUIRE(!bma.Add(bm));
	REQUIRE(bma.Has(bm));
	REQUIRE_EQ(bma.size(), 1);
	REQUIRE(std::is_sorted(bma.begin(), bma.end()));

	bm = {DOWN, Keys::DOWN};
	REQUIRE(bma.Add(bm));
	REQUIRE(bma.Has(bm));
	REQUIRE_EQ(bma.size(), 2);
	REQUIRE(std::is_sorted(bma.begin(), bma.end()));

	bm = {UP, Keys::UP};
	REQUIRE(bma.Add(bm));
	REQUIRE(bma.Has(bm));
	REQUIRE_EQ(bma.size(), 3);
	REQUIRE(std::is_sorted(bma.begin(), bma.end()));

	bm = {UP, Keys::UP};
	REQUIRE(bma.Remove(bm));
	REQUIRE(!bma.Has(bm));
	REQUIRE_EQ(bma.size(), 2);
	REQUIRE(std::is_sorted(bma.begin(), bma.end()));

	bm = {UP, Keys::UP};
	REQUIRE(!bma.Remove(bm));
	REQUIRE(!bma.Has(bm));
	REQUIRE_EQ(bma.size(), 2);
	REQUIRE(std::is_sorted(bma.begin(), bma.end()));
}

TEST_CASE("ButtonMappingArrayRemoveAll") {
	const ButtonMappingArray cbma = {
		{ UP, Keys::UP }, { UP, Keys::A },
		{ DOWN, Keys::DOWN }, { DOWN, Keys::B }, { DOWN, Keys::C },
		{ LEFT, Keys::LEFT },
		{ RIGHT, Keys::RIGHT }
	};

	auto bma = cbma;

	REQUIRE_EQ(bma.size(), 7);
	REQUIRE(std::is_sorted(bma.begin(), bma.end()));
	for (auto& bm: cbma) {
		REQUIRE(bma.Has(bm));
	}

	REQUIRE_EQ(bma.RemoveAll(DOWN), 3);
	REQUIRE_EQ(bma.size(), 4);
	REQUIRE(std::is_sorted(bma.begin(), bma.end()));
	for (auto& bm: cbma) {
		if (bm.button == DOWN) {
			REQUIRE(!bma.Has(bm));
		} else {
			REQUIRE(bma.Has(bm));
		}
	}

	REQUIRE_EQ(bma.RemoveAll(RIGHT), 1);
	REQUIRE_EQ(bma.size(), 3);
	REQUIRE(std::is_sorted(bma.begin(), bma.end()));
	for (auto& bm: cbma) {
		if (bm.button == DOWN || bm.button == RIGHT) {
			REQUIRE(!bma.Has(bm));
		} else {
			REQUIRE(bma.Has(bm));
		}
	}

	REQUIRE_EQ(bma.RemoveAll(LEFT), 1);
	REQUIRE_EQ(bma.size(), 2);
	REQUIRE(std::is_sorted(bma.begin(), bma.end()));
	for (auto& bm: cbma) {
		if (bm.button == DOWN || bm.button == RIGHT || bm.button == LEFT) {
			REQUIRE(!bma.Has(bm));
		} else {
			REQUIRE(bma.Has(bm));
		}
	}

	REQUIRE_EQ(bma.RemoveAll(UP), 2);
	REQUIRE_EQ(bma.size(), 0);
	for (auto& bm: cbma) {
		REQUIRE(!bma.Has(bm));
	}
}

TEST_CASE("ButtonMappingArrayReplaceAll") {
	const ButtonMappingArray cbma = {
		{ UP, Keys::UP }, { UP, Keys::A },
		{ DOWN, Keys::DOWN }, { DOWN, Keys::B }, { DOWN, Keys::C },
		{ LEFT, Keys::LEFT },
		{ RIGHT, Keys::RIGHT }
	};
	auto bma = cbma;
	std::vector<Keys::InputKey> keys;

	REQUIRE_EQ(bma.size(), 7);
	REQUIRE(std::is_sorted(bma.begin(), bma.end()));


	keys = { Keys::W, Keys::X, Keys::U, Keys::Z, Keys::G };
	bma.ReplaceAll(DOWN, keys);
	REQUIRE_EQ(bma.size(), 9);
	REQUIRE(std::is_sorted(bma.begin(), bma.end()));
	for (auto& k: keys) {
		REQUIRE(bma.Has(ButtonMapping{DOWN, k}));
	}
	for (auto& bm: cbma) {
		if (bm.button == DOWN) {
			REQUIRE(!bma.Has(bm));
		} else {
			REQUIRE(bma.Has(bm));
		}
	}

	keys = { Keys::N0, Keys::N2, Keys::N3, Keys::N5, Keys::N4 };
	bma.ReplaceAll(DOWN, keys);
	REQUIRE_EQ(bma.size(), 9);
	REQUIRE(std::is_sorted(bma.begin(), bma.end()));
	for (auto& k: keys) {
		REQUIRE(bma.Has(ButtonMapping{DOWN, k}));
	}
	for (auto& bm: cbma) {
		if (bm.button == DOWN) {
			REQUIRE(!bma.Has(bm));
		} else {
			REQUIRE(bma.Has(bm));
		}
	}

	keys = { Keys::U, Keys::V };
	bma.ReplaceAll(DOWN, keys);
	REQUIRE_EQ(bma.size(), 6);
	REQUIRE(std::is_sorted(bma.begin(), bma.end()));
	for (auto& k: keys) {
		REQUIRE(bma.Has(ButtonMapping{DOWN, k}));
	}
	for (auto& bm: cbma) {
		if (bm.button == DOWN) {
			REQUIRE(!bma.Has(bm));
		} else {
			REQUIRE(bma.Has(bm));
		}
	}

	keys = {};
	bma.ReplaceAll(DOWN, keys);
	REQUIRE_EQ(bma.size(), 4);
	REQUIRE(std::is_sorted(bma.begin(), bma.end()));
	for (auto& k: keys) {
		REQUIRE(bma.Has(ButtonMapping{DOWN, k}));
	}
	for (auto& bm: cbma) {
		if (bm.button == DOWN) {
			REQUIRE(!bma.Has(bm));
		} else {
			REQUIRE(bma.Has(bm));
		}
	}

	keys = { Keys::Z };
	bma.ReplaceAll(UP, keys);
	REQUIRE_EQ(bma.size(), 3);
	REQUIRE(std::is_sorted(bma.begin(), bma.end()));
	for (auto& k: keys) {
		REQUIRE(bma.Has(ButtonMapping{UP, k}));
	}
	for (auto& bm: cbma) {
		if (bm.button == DOWN || bm.button == UP) {
			REQUIRE(!bma.Has(bm));
		} else {
			REQUIRE(bma.Has(bm));
		}
	}

	keys = {};
	bma.ReplaceAll(RIGHT, keys);
	REQUIRE_EQ(bma.size(), 2);
	REQUIRE(std::is_sorted(bma.begin(), bma.end()));
	for (auto& k: keys) {
		REQUIRE(bma.Has(ButtonMapping{RIGHT, k}));
	}
	for (auto& bm: cbma) {
		if (bm.button == DOWN || bm.button == UP || bm.button == RIGHT) {
			REQUIRE(!bma.Has(bm));
		} else {
			REQUIRE(bma.Has(bm));
		}
	}
}

TEST_CASE("ButtonMappingArrayReplaceAllSame") {
	const ButtonMappingArray cbma = {
		{ UP, Keys::UP }, { UP, Keys::A },
		{ DOWN, Keys::DOWN }, { DOWN, Keys::B }, { DOWN, Keys::C },
		{ LEFT, Keys::LEFT },
		{ RIGHT, Keys::RIGHT }
	};
	auto bma = cbma;
	std::vector<Keys::InputKey> keys;

	REQUIRE_EQ(bma.size(), 7);
	REQUIRE(std::is_sorted(bma.begin(), bma.end()));

	keys = { Keys::DOWN, Keys::B, Keys::C };
	bma.ReplaceAll(DOWN, keys);
	REQUIRE_EQ(bma.size(), 7);
	REQUIRE(std::is_sorted(bma.begin(), bma.end()));

	auto iter = bma.begin();
	auto citer = cbma.begin();

	while (iter != bma.end()) {
		REQUIRE_EQ(*iter, *citer);
		++iter;
		++citer;
	}
}

TEST_CASE("ButtonMappingArrayReplaceAllDupes") {
	const ButtonMappingArray cbma = {
		{ UP, Keys::UP }, { UP, Keys::A },
		{ DOWN, Keys::DOWN }, { DOWN, Keys::B }, { DOWN, Keys::C },
		{ LEFT, Keys::LEFT },
		{ RIGHT, Keys::RIGHT }
	};
	auto bma = cbma;
	std::vector<Keys::InputKey> keys;

	REQUIRE_EQ(bma.size(), 7);
	REQUIRE(std::is_sorted(bma.begin(), bma.end()));

	keys = { Keys::X, Keys::B, Keys::Y, Keys::B };
	bma.ReplaceAll(DOWN, keys);
	REQUIRE_EQ(bma.size(), 7);
	REQUIRE(std::is_sorted(bma.begin(), bma.end()));

	keys = { Keys::X, Keys::X };
	bma.ReplaceAll(DOWN, keys);
	REQUIRE_EQ(bma.size(), 5);
	REQUIRE(std::is_sorted(bma.begin(), bma.end()));

}

END_TEST_SUITE();

#endif
