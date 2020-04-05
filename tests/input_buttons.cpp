#include "input_buttons.h"
#include "doctest.h"
#include <climits>

using namespace Input;

TEST_SUITE_BEGIN("InputButtons");

TEST_CASE("ButtonMappingArrayDefault") {
	ButtonMappingArray bma;
	const auto& cbma = bma;

	REQUIRE_EQ(bma.size(), 0);
	REQUIRE_EQ(bma.begin(), bma.end());
	REQUIRE_EQ(cbma.begin(), cbma.end());

}

TEST_CASE("ButtonMappingArrayInitList") {
	ButtonMappingArray bma = {
		{ N0, Keys::N0 },
		{ DOWN, Keys::DOWN },
		{ RIGHT, Keys::RIGHT },
		{ RIGHT, Keys::A },
		{ UP, Keys::UP }
	};
	const auto& cbma = bma;

	REQUIRE_EQ(bma.size(), 5);
	REQUIRE_NE(bma.begin(), bma.end());
	REQUIRE_NE(cbma.begin(), cbma.end());
	REQUIRE_EQ(bma.begin(), cbma.begin());
	REQUIRE_EQ(bma.end(), cbma.end());

	REQUIRE(std::is_sorted(bma.begin(), bma.end()));
	REQUIRE(std::is_sorted(cbma.begin(), cbma.end()));
}

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
