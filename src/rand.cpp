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
#include "compiler.h"
#include <cassert>
#include <cstdint>
#include <cinttypes>
#include <algorithm>
#include <random>

namespace {
Rand::RNG rng;

/** Gets a random number uniformly distributed in [0, U32_MAX] */
uint32_t GetRandomU32() { return rng(); }

int32_t rng_lock_value = 0;
bool rng_locked= false;
}

/** Generate a random number in the range [0,max] */
static uint32_t GetRandomUnsigned(uint32_t max)
{
	if (max == 0xffffffffull) return GetRandomU32();

	// Rejection sampling:
	// 1. Divide the range of uint32 into blocks of max+1
	//    numbers each, with rem numbers left over.
	// 2. Generate a random u32. If it belongs to a block,
	//    mod it into the range [0,max] and accept it.
	// 3. If it fell into the range of rem leftover numbers,
	//    reject it and go back to step 2.
	uint32_t m = max + 1;
	uint32_t rem = -m % m; // = 2^32 mod m
	while (true) {
		uint32_t n = GetRandomU32();
		if (n >= rem)
			return n % m;
	}
}

int32_t Rand::GetRandomNumber(int32_t from, int32_t to) {
	assert(from <= to);
	if (rng_locked) {
		return Utils::Clamp(rng_lock_value, from, to);
	}
	// Don't use uniform_int_distribution--the algorithm used isn't
	// portable between stdlibs.
	// We do from + (rand int in [0, to-from]). The miracle of two's
	// complement let's us do this all in unsigned and then just cast
	// back.
	uint32_t ufrom = uint32_t(from);
	uint32_t uto = uint32_t(to);
	uint32_t urange = uto - ufrom;
	uint32_t ures = ufrom + GetRandomUnsigned(urange);
	return int32_t(ures);
}

Rand::RNG& Rand::GetRNG() {
	return rng;
}

bool Rand::ChanceOf(int32_t n, int32_t m) {
	assert(n >= 0 && m > 0);
	return GetRandomNumber(1, m) <= n;
}

bool Rand::PercentChance(float rate) {
	constexpr auto scale = 0x1000000;
	return GetRandomNumber(0, scale-1) < int32_t(rate * scale);
}

bool Rand::PercentChance(int rate) {
	return GetRandomNumber(0, 99) < rate;
}

void Rand::SeedRandomNumberGenerator(int32_t seed) {
	rng.seed(seed);
	Output::Debug("Seeded the RNG with {}.", seed);
}

void Rand::LockRandom(int32_t value) {
	rng_locked = true;
	rng_lock_value = value;
}

void Rand::UnlockRandom() {
	rng_locked = false;
}

std::pair<bool,int32_t> Rand::GetRandomLocked() {
	return { rng_locked, rng_lock_value };
}

Rand::LockGuard::LockGuard(int32_t lock_value, bool locked) {
	auto p = GetRandomLocked();
	_prev_locked = p.first;
	_prev_lock_value = p.second;
	_active = true;

	if (locked) {
		LockRandom(lock_value);
	} else {
		UnlockRandom();
	}
}

void Rand::LockGuard::Release() noexcept {
	if (Enabled()) {
		if (_prev_locked) {
			LockRandom(_prev_lock_value);
		} else {
			UnlockRandom();
		}
		Dismiss();
	}
}
