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

#include <cstdint>
#include <functional>
#include <string>
#include <sstream>
#include <vector>
#include <random>
#include "system.h"
#include "string_view.h"
#include "span.h"

namespace Rand {
/**
 * The random number generator object to use
 */
using RNG = std::mt19937;

/**
 * Gets a random number in the inclusive range from - to.
 *
 * @param from Interval start
 * @param to Interval end
 * @return Random number in inclusive interval
 */
int32_t GetRandomNumber(int32_t from, int32_t to);

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
bool ChanceOf(int32_t n, int32_t m);

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
void SeedRandomNumberGenerator(int32_t seed);

/**
 * Forces GetRandomNumber() and all dervative functions to return a fixed value.
 * Useful for testing.
 *
 * @param lock_value the value to set. A calls to GetRandomNumber(a, b) will return clamp(lock_value, a, b)
 * @post All calls to GetRandomNumber(a, b) will return clamp(lock_value, a, b)
 */
void LockRandom(int32_t lock_value);

/**
 * Disables locked random number and returns RNG to original state.
 * @post All calls to GetRandomNumber(a, b) will return random values.
 */
void UnlockRandom();

/**
 * Retrive whether random numbers are locked and if so, which value they are locked to.
 * @return whether or not random numbers are locked and if so, to what value.
 */
std::pair<bool,int32_t> GetRandomLocked();

/** An RAII guard which fixes the rng while active and resets on destruction */
class LockGuard {
public:
	/**
	 * Store current state and set locked state
	 * @param lock_value The rng value to fix to
	 * @param locked Whether to fix or reset
	 */
	LockGuard(int32_t lock_value, bool locked = true);

	LockGuard(const LockGuard&) = delete;
	LockGuard& operator=(const LockGuard&) = delete;

	/** Move other LockGuard to this */
	LockGuard(LockGuard&& o) noexcept;
	LockGuard& operator=(LockGuard&&) = delete;

	/** Calls Release() */
	~LockGuard();

	/** If Enabled(), returns the rng locked state to what it was */
	void Release() noexcept;

	/** Disables the LockGuard leaving the rng state as is */
	void Dismiss();

	/** @return whether the guard is enabled and will release on destruction */
	bool Enabled() const;
private:
	int32_t _prev_lock_value = 0;
	bool _prev_locked = false;
	bool _active = false;
};

inline bool PercentChance(long rate) {
	return PercentChance(static_cast<int>(rate));
}

inline LockGuard::LockGuard(LockGuard&& o) noexcept {
	std::swap(_prev_lock_value, o._prev_lock_value);
	std::swap(_prev_locked, o._prev_locked);
	std::swap(_active, o._active);
}

inline LockGuard::~LockGuard() {
	Release();
}

inline void LockGuard::Dismiss() {
	_active = false;
}

inline bool LockGuard::Enabled() const {
	return _active;
}

} // namespace Rand


#endif
