/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

// Headers
#include "rand.h"
#include "utils.h"
#include "output.h"
#include <cassert>
#include <cstdint>
#include <algorithm>
#include <random>

namespace Rand
{
	AbstractRNGWrapper::result_type SequencedRNGWrapper::operator()()
	{
		return pickNext();
	}

	AbstractRNGWrapper::result_type SequencedRNGWrapper::distribute(std::uint32_t max)
	{
		return Utils::Clamp(pickNext(), 0u, max);
	}

	std::int32_t SequencedRNGWrapper::distribute(std::int32_t from, std::int32_t to)
	{
		return Utils::Clamp(static_cast<std::int32_t>(pickNext()), from, to);
	}

	void SequencedRNGWrapper::seed(Seed_t seed)
	{
	}

	AbstractRNGWrapper::result_type SequencedRNGWrapper::pickNext()
	{
		auto value = m_Sequence[m_NextIndex];
		m_NextIndex = ++m_NextIndex % std::size(m_Sequence);
		return value;
	}
}

template <class TGenerator>
class RNGWrapper :
	public Rand::AbstractRNGWrapper
{
public:
	explicit RNGWrapper(TGenerator generator = TGenerator{}) :
		AbstractRNGWrapper{},
		m_Generator{ std::move(generator) }
	{
	}

	void seed(Rand::Seed_t seed) override
	{
		m_Generator.seed(seed);
	}

	/** Gets a random number uniformly distributed in [0, U32_MAX] */
	result_type operator()() override
	{
		return m_Generator();
	}

	/** Generate a random number in the range [0,max] */
	result_type distribute(std::uint32_t max) override
	{
		if (max == 0xffffffffull)
		{
			return m_Generator();
		}

		// Rejection sampling:
		// 1. Divide the range of uint32 into blocks of max+1
		//    numbers each, with rem numbers left over.
		// 2. Generate a random u32. If it belongs to a block,
		//    mod it into the range [0,max] and accept it.
		// 3. If it fell into the range of rem leftover numbers,
		//    reject it and go back to step 2.
		std::uint32_t m = max + 1;
		std::uint32_t rem = -m % m; // = 2^32 mod m
		while (true) {
			auto n =  m_Generator();
			if (n >= rem)
				return n % m;
		}
	}
	
	std::int32_t distribute(std::int32_t from, std::int32_t to) override
	{
		// Don't use uniform_int_distribution--the algorithm used isn't
		// portable between stdlibs.
		// We do from + (rand int in [0, to-from]). The miracle of two's
		// complement let's us do this all in unsigned and then just cast
		// back.
		auto ufrom =  static_cast<std::uint32_t>(from);
		auto uto =  static_cast<std::uint32_t>(to);
		auto urange = uto - ufrom;
		auto ures = ufrom + distribute(urange);
		return static_cast<std::int32_t>(ures);
	}

private:
	TGenerator m_Generator;
};

namespace {
	Rand::RngPtr rng = std::make_unique<RNGWrapper<std::mt19937>>();
}

Rand::RngPtr Rand::ExchangeRNG(RngPtr newRng)
{
	assert(newRng && "rng must not be nullptr");
	return std::exchange(rng, std::move(newRng));
}

std::int32_t Rand::GetRandomNumber(std::int32_t from, std::int32_t to) {
	assert(from <= to && "from must be less-equal than to");
	return GetRNG().distribute(from, to);
}

Rand::RNG& Rand::GetRNG() {
	assert(rng && "rng is empty");
	return *rng;
}

bool Rand::ChanceOf(std::int32_t n, std::int32_t m) {
	assert(n >= 0 && m > 0);
	return GetRandomNumber(1, m) <= n;
}

bool Rand::PercentChance(float rate) {
	constexpr auto scale = 0x1000000;
	return GetRandomNumber(0, scale-1) < static_cast<std::int32_t>(rate * scale);
}

bool Rand::PercentChance(int rate) {
	return GetRandomNumber(0, 99) < rate;
}

void Rand::SeedRandomNumberGenerator(Seed_t seed) {
	GetRNG().seed(seed);
	Output::Debug("Seeded the RNG with {}.", seed);
}
