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

#ifndef EP_FLAT_MAP_H
#define EP_FLAT_MAP_H

// Headers
#include <vector>
#include <array>
#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <ostream>
#include <utility>

/** An ordered associative container which maps keys to values.
 * Each key may appear multiple times, but each (key, value) pair
 * must be unique. The elements are ordered by (key, value) lexicographically.
 * The data is stored in a flat array, which means read operations are fast
 * and write operations are slower.
 * This data structure is optimized for write rarely, read often workloads.
 */
template <typename K, typename V>
class FlatUniqueMultiMap {
	public:
		using pair_type = std::pair<K,V>;
		using first_type = K;
		using second_type = V;
		using container_type = std::vector<pair_type>;

		using iterator = typename container_type::const_iterator;

		/** Construct an empty map */
		FlatUniqueMultiMap() = default;

		/**
		 * Construct a map with the given (key, value) pairs.
		 * Any duplicates in the list will be dropped. Which of the
		 * duplicate elements get removed is implementation defined.
		 *
		 * @param ilist the list of initial (key, value) pairs.
		 */
		FlatUniqueMultiMap(std::initializer_list<pair_type> ilist);

		/**
		 * Return lower bound of (key, value) pair
		 *
		 * @param pt the pair to lookup.
		 * @return iterator to the first element greater than or equal to pt.
		 */
		iterator LowerBound(const pair_type& pt) const;

		/**
		 * Return lower bound of key
		 *
		 * @param pt the pair to lookup.
		 * @return iterator to the first element whose key is greater than or equal to key.
		 */
		iterator LowerBound(const first_type& key) const;

		/**
		 * Check whether the given (key, value) pair is in the map.
		 * @param pt the pair to check.
		 * @return true if pt is in the map.
		 */
		bool Has(const pair_type& pt) const;

		/**
		 * Check whether the given key has any value in the map.
		 * @param key the key to check.
		 * @return true if key is in the map.
		 */
		bool Has(const first_type& key) const;

		/**
		 * Counts how many items with a given key are in the map.
		 * @param key the key to check.
		 * @return size_t how often the key appears
		 */
		size_t Count(const first_type& key) const;

		/**
		 * Adds the (key, value) pair to the map if it doesn't already exist.
		 * @param pt the pair to add.
		 * @return true if was added.
		 */
		bool Add(pair_type pt);

		/**
		 * Removes the (key, value) pair from the map if it exists.
		 * @param pt the pair to remove.
		 * @return true if pt was removed.
		 */
		bool Remove(const pair_type& pt);

		/**
		 * Removes all (key, value) pairs for the given key.
		 * @param key the key to remove.
		 * @return the number of pairs removed.
		 */
		int RemoveAll(const first_type& key);

		/**
		 * Replace all (key, value) pairs for the given key with the given values in the iterator range.
		 * @param key the key to replace.
		 * @param values_begin an iterator pointing to the beginning of the values range to replace with.
		 * @param values_end an iterator pointing to the end of the values range to replace with.
		 */
		template <typename ValueIter>
		void ReplaceAll(const first_type& key, ValueIter values_begin, ValueIter values_end);

		/** @return iterator to beginning */
		iterator begin() const { return mappings.begin(); }
		/** @return iterator to end */
		iterator end() const { return mappings.end(); }

		/** @return the number of (key, value) pairs in the map. */
		size_t size() const { return mappings.size(); }

		/** @return true if the map is empty. */
		bool empty() const { return mappings.empty(); }
	private:
		container_type mappings;
};

template <typename K, typename V>
FlatUniqueMultiMap<K,V>::FlatUniqueMultiMap(std::initializer_list<pair_type> ilist) : mappings{ilist}
{
	std::sort(mappings.begin(), mappings.end());
	auto iter = std::unique(mappings.begin(), mappings.end());
	mappings.erase(iter, mappings.end());
}

template <typename K, typename V>
typename FlatUniqueMultiMap<K,V>::iterator FlatUniqueMultiMap<K, V>::LowerBound(const pair_type& pt) const {
	return std::lower_bound(mappings.begin(), mappings.end(), pt);
}

template <typename K, typename V>
typename FlatUniqueMultiMap<K,V>::iterator FlatUniqueMultiMap<K, V>::LowerBound(const first_type& key) const {
	return std::lower_bound(mappings.begin(), mappings.end(), key, [](auto& l, auto& r) { return l.first < r; });
}

template <typename K, typename V>
bool FlatUniqueMultiMap<K, V>::Has(const pair_type& pt) const {
	auto iter = LowerBound(pt);
	return iter != mappings.end() && *iter == pt;
}

template <typename K, typename V>
bool FlatUniqueMultiMap<K, V>::Has(const first_type& key) const {
	auto iter = LowerBound(key);
	return iter != mappings.end() && iter->first == key;
}

template <typename K, typename V>
size_t FlatUniqueMultiMap<K, V>::Count(const first_type& key) const {
	auto iter = LowerBound(key);

	auto start_iter = iter;

	if (iter != mappings.end() && iter->first == key) {
		while (iter != mappings.end() && iter->first == key) {
			++iter;
		}
	}

	return std::distance(start_iter, iter);
}

template <typename K, typename V>
bool FlatUniqueMultiMap<K,V>::Add(pair_type pt) {
	auto iter = LowerBound(pt);
	if (iter != mappings.end() && *iter == pt) {
		return false;
	}
	mappings.insert(iter, std::move(pt));
	return true;
}

template <typename K, typename V>
bool FlatUniqueMultiMap<K,V>::Remove(const pair_type& pt) {
	auto iter = LowerBound(pt);
	if (iter != mappings.end() && *iter == pt) {
		mappings.erase(iter);
		return true;
	}
	return false;
}

template <typename K, typename V>
int FlatUniqueMultiMap<K,V>::RemoveAll(const first_type& key) {
	const auto pos = std::distance(mappings.cbegin(), LowerBound(key));
	const auto end = mappings.end();
	const auto iter = std::remove_if(mappings.begin() + pos, end, [&key](auto& pt) { return pt.first == key; });
	const auto ret = std::distance(iter, end);
	mappings.erase(iter, end);
	return ret;
}

template <typename K, typename V>
template <typename ValueIter>
void FlatUniqueMultiMap<K,V>::ReplaceAll(const first_type& key, const ValueIter values_begin, const ValueIter values_end) {
	auto end = mappings.end();
	const auto key_start = std::distance(mappings.cbegin(), LowerBound(key));

	auto iter = mappings.begin() + key_start;
	auto values_iter = values_begin;

	// Replace existing mappings with new ones in-place
	while (iter != end && values_iter != values_end && iter->first == key) {
		(iter++)->second = *(values_iter++);
	}
	if (values_iter != values_end) {
		// We ran out of space and there are still more values to add.
		const auto num_additional_values = std::distance(values_iter, values_end);
		const auto iter_pos = std::distance(mappings.begin(), iter);
		const auto end_pos = mappings.size();

		mappings.resize(mappings.size() + num_additional_values);
		iter = mappings.begin() + iter_pos;
		const auto prev_end = mappings.begin() + end_pos;
		end = mappings.end();

		// Shift all array contents to make room, then replace the new values.
		std::move_backward(iter, prev_end, end);
		while (values_iter != values_end) {
			*(iter++) = { key, *(values_iter++) };
		}
	} else {
		// The new keyset was smaller (or exact same size), remove the extra ones
		iter = std::remove_if(iter, end, [&](auto& pt) { return pt.first == key; });
		mappings.erase(iter, end);
	}

	// Sort the new keyset
	iter = mappings.begin() + key_start;
	const auto key_end = iter + std::distance(values_begin, values_end);
	std::sort(iter, key_end);

	// Rare case - if the user passed in duplicates we need to remove them.
	iter = std::unique(iter, key_end);
	mappings.erase(iter, key_end);
}

#endif

