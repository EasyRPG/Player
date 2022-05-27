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

#ifndef EP_COMPILER_H
#define EP_COMPILER_H

#ifdef __GNUC__

#define EP_LIKELY(x) __builtin_expect(!!(x), 1)
#define EP_UNLIKELY(x) __builtin_expect(!!(x), 0)

#define EP_ALWAYS_INLINE __attribute__((always_inline)) inline

#elif _MSC_VER

#define EP_LIKELY(x) x
#define EP_UNLIKELY(x) x

#define EP_ALWAYS_INLINE __forceinline

#else

#define EP_LIKELY(x) x
#define EP_UNLIKELY(x) x

#define EP_ALWAYS_INLINE inline

#endif

#endif
