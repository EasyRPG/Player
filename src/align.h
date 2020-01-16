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

#ifndef EP_ALIGN_H
#define EP_ALIGN_H

#include <type_traits>
#include <cstdint>
#include <cstddef>

/**
 * @return true if t is aligned to a
 * @pre If a is not a power of 2, the result is undefined.
 */
template <typename T,
         typename = std::enable_if_t<std::is_integral<T>::value>>
constexpr bool IsAligned(T t, size_t a) noexcept {
  return ((t & (a-1)) == 0);
}

/**
 * @return true if p is aligned to a
 * @pre If a is not a power of 2, the result is undefined.
 */
inline bool IsAligned(void* p, size_t a) noexcept {
  return IsAligned(uintptr_t(p), a);
}

/*
 * @return the smallest number n when n >= val && is_aligned(n, align).
 * @pre If a is not a power of 2, the result is undefined.
 */
template <typename T,
         typename = std::enable_if_t<std::is_integral<T>::value>>
constexpr T AlignUp(T val, size_t a) noexcept {
  return ((val + (a -1)) & -a);
}

/**
 * @return the closest pointer p' where p' >= p && is_aligned(p, align).
 * @pre If a is not a power of 2, the result is undefined.
 */
inline void* AlignUp(void* val, size_t a) noexcept {
  return (void*)AlignUp(uintptr_t(val), a);
}

/**
 * @return the largest number n when n <= val && is_aligned(n, align).
 * @pre If a is not a power of 2, the result is undefined.
 */
template <typename T,
         typename = std::enable_if_t<std::is_integral<T>::value>>
constexpr T AlignDown(T val, size_t a) noexcept {
  return val & -a;
}

/**
 * @return the closest pointer p' where p' <= val && is_aligned(p, align).
 * @pre If a is not a power of 2, the result is undefined.
 */
inline void* AlignDown(void* val, size_t a) noexcept {
  return (void*)AlignDown(uintptr_t(val), a);
}

/** @return true if val is a power of 2 */
template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
constexpr bool IsPow2(T val) noexcept {
	return (val & (val - 1)) == 0;
}

#endif
