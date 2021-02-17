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

static void testSequenceGenerator(const std::vector<std::uint32_t>& seq, Rand::SequencedRNGWrapper& rng, std::size_t iterations = 1)
{
	for (std::size_t i = 0; i < iterations; ++i)
	{
		for (auto value : seq)
		{
			REQUIRE(rng() == value);
		}
	}
}

TEST_CASE("Single-Element Sequence") {

	std::vector<std::uint32_t> seq{ 42 };
	Rand::SequencedRNGWrapper seqRNG(seq);

	testSequenceGenerator(seq, seqRNG, 3);
}

TEST_CASE("Multi-Element Sequence") {
	
	std::vector<std::uint32_t> seq{ 42, 1337, 42, 3, 1, 3 };
	Rand::SequencedRNGWrapper seqRNG(seq);

	testSequenceGenerator(seq, seqRNG, 3);
}

TEST_CASE("GetRNG") {

	REQUIRE(&Rand::GetRNG() != nullptr);
}

TEST_CASE("ExchangeRNG") {

	auto* preRNGPtr = &Rand::GetRNG();
	std::vector<std::uint32_t> seq{ 42, 1337, 42, 3, 1, 3 };
	auto seqRNG = std::make_unique<Rand::SequencedRNGWrapper>(seq);
	auto* seqRNGPtr = seqRNG.get();
	auto prevRNG = Rand::ExchangeRNG(std::move(seqRNG));
	
	REQUIRE(preRNGPtr == prevRNG.get());

	REQUIRE(seqRNGPtr == &Rand::GetRNG());
}

TEST_CASE("ScopedRNGExchange") {

	auto* preRNGPtr = &Rand::GetRNG();
	
	{
		auto scoped = Rand::test::makeScopedRNGExchange<Rand::SequencedRNGWrapper>(1);

		REQUIRE(preRNGPtr != &Rand::GetRNG());
	}

	REQUIRE(preRNGPtr == &Rand::GetRNG());
}

static void testGetRandomNumberFixed(int32_t a, int32_t b, int32_t fix, int32_t result) {
	auto scoped = Rand::test::makeScopedRNGExchange<Rand::SequencedRNGWrapper>(fix);
	for (int i = 0; i < 10; ++i) {
		auto x = Rand::GetRandomNumber(a, b);
		REQUIRE_EQ(x, result);
	}
}

TEST_CASE("GetRandomNumberFixed") {
	testGetRandomNumberFixed(-10, 12, 5, 5);
	testGetRandomNumberFixed(-10, 12, 12, 12);
	testGetRandomNumberFixed(-10, 12, 55, 12);
	testGetRandomNumberFixed(-10, 12, std::numeric_limits<std::int32_t>::min(), -10);
	testGetRandomNumberFixed(-10, 12, std::numeric_limits<std::int32_t>::max(), 12);
}

TEST_SUITE_END();
