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

#ifndef EP_RANDOM_H
#define EP_RANDOM_H

#include <functional>
#include <vector>
#include <random>
#include "string_view.h"
#include "span.h"

namespace Rand {

using Seed_t = std::uint32_t;

class AbstractRNGWrapper
{
public:
	using result_type = std::uint32_t;
	
	virtual ~AbstractRNGWrapper() noexcept = default;

	AbstractRNGWrapper(const AbstractRNGWrapper&) = delete;
	AbstractRNGWrapper& operator =(const AbstractRNGWrapper&) = delete;
	AbstractRNGWrapper(AbstractRNGWrapper&&) = delete;
	AbstractRNGWrapper& operator =(AbstractRNGWrapper&&) = delete;

	virtual result_type operator()() = 0;
	virtual result_type distribute(std::uint32_t max) = 0;
	virtual std::int32_t distribute(std::int32_t from, std::int32_t to) = 0;
	virtual void seed(Seed_t seed) = 0;
	
	static result_type min() { return std::numeric_limits<result_type>::min(); }
	static result_type max() { return std::numeric_limits<result_type>::max(); }

protected:
	AbstractRNGWrapper() noexcept = default;
};

class SequencedRNGWrapper :
	public AbstractRNGWrapper
{
public:
	explicit SequencedRNGWrapper(std::vector<result_type> seq) noexcept :
		AbstractRNGWrapper{},
		m_Sequence{ std::move(seq) }
	{
		assert(!std::empty(m_Sequence) && "Empty sequence is not allowed.");
	}
	
	template <class... TArgs, typename = std::enable_if_t<std::is_convertible_v<std::common_type_t<TArgs...>, result_type>>>
	explicit SequencedRNGWrapper(TArgs ... args) noexcept :
		SequencedRNGWrapper{ std::vector<result_type>{ static_cast<result_type>(args)... } }
	{
	}

	result_type operator()() override;

	result_type distribute(std::uint32_t max) override;
	std::int32_t distribute(std::int32_t from, std::int32_t to) override;

	void seed(Seed_t seed) override;

private:
	std::size_t m_NextIndex = 0;
	std::vector<result_type> m_Sequence;

	result_type pickNext();
};

 /**
 * The random number generator object to use
 */
using RNG = AbstractRNGWrapper;
using RngPtr = std::unique_ptr<RNG>;

RngPtr ExchangeRNG(RngPtr newRng);

/**
 * Gets a random number in the inclusive range from - to.
 *
 * @param from Interval start
 * @param to Interval end
 * @return Random number in inclusive interval
 */
std::int32_t GetRandomNumber(std::int32_t from, std::int32_t to);

/**
 * Gets the seeded Random Number Generator (RNG).
 *
 * @return the random number generator
 */
RNG& GetRNG();

/**
 * Has an n/m chance of returning true. If n>m, always returns true.
 *
 * @param n number of times out of m to return true (non-negative)
 * @param m denominator of the probability (positive)
 * @return true with probability n/m, false with probability 1-n/m
 */
bool ChanceOf(std::int32_t n, std::int32_t m);

/**
 * Rolls a random number in [0.0f, 1.0f) returns true if it's less than rate.
 *
 * @param rate a value in [0.0f, 1.0f]. Values out of this range are clamped.
 * @return true with probability rate.
 */
bool PercentChance(float rate);

/**
 * Rolls a random number in [0, 99] and returns true if it's less than rate.
 *
 * @param rate a value in [0, 100]. Values out of this range are clamped.
 * @return true with probability rate.
 */
bool PercentChance(int rate);
bool PercentChance(long rate);

/**
 * Seeds the RNG used by GetRandomNumber and ChanceOf.
 *
 * @param seed Seed to use
 */
void SeedRandomNumberGenerator(Seed_t seed);

namespace test
{
	template <class TRNGWrapper, typename = std::enable_if_t<std::is_base_of_v<AbstractRNGWrapper, TRNGWrapper>>>
	class ScopedRNGExchange
	{
	public:
		template <class... TArgs, typename = std::enable_if_t<std::is_constructible_v<TRNGWrapper, TArgs...>>>
		explicit ScopedRNGExchange(TArgs&&... args) :
			m_PreviousRNG{ Rand::ExchangeRNG(std::make_unique<TRNGWrapper>(std::forward<TArgs>(args)...)) }
		{
		}

		~ScopedRNGExchange() noexcept
		{
			if (m_PreviousRNG)
			{
				ExchangeRNG(std::move(m_PreviousRNG));
			}
		}

		ScopedRNGExchange(const ScopedRNGExchange&) = delete;
		ScopedRNGExchange& operator =(const ScopedRNGExchange&) = delete;

		ScopedRNGExchange(ScopedRNGExchange&&) noexcept = default;
		ScopedRNGExchange& operator =(ScopedRNGExchange&&) noexcept = default;
	
	private:
		RngPtr m_PreviousRNG;
	};

	template <class TRNGWrapper, class... TArgs>
	std::enable_if_t<
		std::is_base_of_v<AbstractRNGWrapper, TRNGWrapper> && std::is_constructible_v<TRNGWrapper, TArgs...>,
		ScopedRNGExchange<TRNGWrapper>	// actual return type
	>
	makeScopedRNGExchange(TArgs&&... args)
	{
		return ScopedRNGExchange<TRNGWrapper>{ std::forward<TArgs>(args)... };
	}
}
} // namespace Rand

#endif
