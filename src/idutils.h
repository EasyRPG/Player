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

#ifndef EP_GET_H
#define EP_GET_H

#include "span.h"
#include "string_view.h"

#include <memory>
#include <cassert>

namespace IdUtils {

/**
 * Check with a 1 based id is valid.
 *
 * @param array the array to check
 * @param the id to check against array
 * @return true if the id valid
 */
template <typename T>
bool IsValid(Span<T> array, int id);

/**
 * Retrieve an element from the array using it's 1 based index.
 *
 * @param array the array to look into.
 * @param id the 1 based id.
 * @return reference to the requested object
 * @pre If id is not valid id into the dataset the result is undefined.
 */
template <typename T>
T& Get(Span<T> array, int id);

/**
 * Retrieve an element from the array using it's 1 based index.
 *
 * @param array the array to look into.
 * @param id the 1 based id.
 * @return pointer to the requested object, or null if the id is invalid.
 */
template <typename T>
T* GetPtr(Span<T> array, int id);

/**
 * Retrieve an element from the array using it's 1 based index and log a warning if the id is invalid.
 *
 * @param array the array to look into.
 * @param id the 1 based id.
 * @param objname The name of the object type
 * @param log_context A string describing the content with which this was called.
 * @return pointer to the requested object, or null if the id is invalid.
 */
template <typename T>
T* GetPtr(Span<T> array, int id, StringView objname, StringView log_context);

/**
 * Log a warning about an invalid id.
 *
 * @param id the 1 based id.
 * @param objname The name of the object type
 * @param log_context A string describing the content with which this was called.
 */
void Warning(int id, StringView objname, StringView log_context);

template <typename T>
inline bool IsValid(Span<T> array, int id) {
	return id >= 1 && id <= static_cast<int>(array.size());
}

template <typename T>
inline T& Get(Span<T> array, int id) {
	assert(IsValid(array, id));
	return array[id - 1];
}


template <typename T>
inline T* GetPtr(Span<T> array, int id) {
	return IsValid(array, id)
		? array.data() + id - 1
		: nullptr;
}

template <typename T>
inline T* GetPtr(Span<T> array, int id, StringView objname, StringView log_context) {
	auto* p = GetPtr(array, id);
	if (p == nullptr) {
		Warning(id, objname, log_context);
	}
	return p;
}

} // namespace IdUtils
#endif
