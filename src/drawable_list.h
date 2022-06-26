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

#ifndef EP_DRAWABLE_LIST_H
#define EP_DRAWABLE_LIST_H

#include "drawable.h"
#include <memory>
#include <vector>
#include <limits>

/** A list of Drawable objects. These are used by the graphics engine store and
 * to render all drawable objects.
 */
class DrawableList {
	public:
		/** Default Constructor */
		DrawableList() = default;
		DrawableList(const DrawableList&) = delete;
		DrawableList& operator=(const DrawableList&) = delete;
		~DrawableList();

		/** Iterator type */
		using iterator = std::vector<Drawable*>::const_iterator;

		/** Sorts the drawables and clears the dirty flag.  */
		void Sort();

		/** Return true if drawables are sorted */
		bool IsSorted() const;

		/**
		 * Add a drawable to the list.
		 *
		 * @param drawable the Drawable to add
		 */
		void Append(Drawable* drawable);

		/** Removes all drawables */
		void Clear();

		/**
		 * Searches for the given drawable, if found removes from the list and returns it.
		 *
		 * @param drawable the Drawable to remove.
		 * @return drawable if drawable was in the list and removed.
		 */
		Drawable* Take(Drawable* drawable);

		/**
		 * Searches for the given drawable, if found removes from the list and returns it.
		 *
		 * @param drawable the Drawable to remove.
		 * @return drawable if drawable was in the list and removed.
		 */
		template <typename T>
		T* Take(std::enable_if_t<IsDrawable<T>,T*> drawable);

		/**
		 * Remove all drawables from other and append them to this.
		 *
		 * @param other other list to take from
		 */
		void TakeFrom(DrawableList& other) noexcept;

		/**
		 * Remove all drawables from other which satify preedicate and append them to this.
		 *
		 * @param other other list to take from
		 * @param predicate a function taking a Drawable*. If it returns true, the drawable is taken.
		 */
		template <typename F>
		void TakeFrom(DrawableList& other, F&& predicate) noexcept;

		/** @return true if the list is dirty and needs to be sorted */
		bool IsDirty() const;

		/** Mark the list as dirty. It will be sorted the next time Draw() is called */
		void SetDirty();

		/** @return an iterator to the beginning */
		iterator begin() const { return _list.begin(); }

		/** @return an iterator to the end */
		iterator end() const { return _list.end(); }

		/**
		 * Return drawable at i'th index
		 *
		 * @param i index of drawable
		 * @pre if i < 0 and i >= size(), the result is undefined.
		 * @return the drawable at i
		 */
		Drawable* operator[](size_t i) const {
			return _list[i];
		}

		/** @return the number of drawables in the list */
		size_t size() const { return _list.size(); }

		/** @return if the list is empty */
		bool empty() const { return _list.empty(); }

		/**
		 * Sort the list if it's dirty, then call Draw() on every drawable in order.
		 *
		 * @param dst The bitmap to draw onto
		 */
		void Draw(Bitmap& dst);

		/**
		 * Sort the list if it's dirty, then call Draw() on every drawable in order.
		 *
		 * @param dst The bitmap to draw onto
		 * @param min_z Skip any drawables with z < min_z
		 * @param max_z Skip any drawables with z > max_z
		 */
		void Draw(Bitmap& dst, Drawable::Z_t min_z, Drawable::Z_t max_z);

	private:
		std::vector<Drawable*> _list;
		bool _dirty = false;

		void SetClean();
};

template <typename T>
T* DrawableList::Take(std::enable_if_t<IsDrawable<T>,T*> drawable) {
	auto ptr = Take(static_cast<Drawable*>(drawable));
	return static_cast<T*>(ptr);
}

template <typename F>
void DrawableList::TakeFrom(DrawableList& other, F&& cond) noexcept {
	if (&other == this) { return; }

	auto& olist = other._list;

	int shift = 0;
	const auto end = olist.end();
	for (auto iter = olist.begin(); iter != end - shift;) {
		if (shift) {
			*iter = std::move(*(iter + shift));
		}

		auto* draw = *iter;

		if (cond(draw)) {
			_list.push_back(draw);
			++shift;
			continue;
		}

		++iter;
	}
	olist.resize(olist.size() - shift);

	SetDirty();
	if (olist.empty()) {
		other.SetClean();
	}
}

inline bool DrawableList::IsDirty() const {
	return _dirty;
}

inline void DrawableList::SetDirty() {
	_dirty = true;
}

inline void DrawableList::SetClean() {
	_dirty = false;
}

inline void DrawableList::Draw(Bitmap& dst) {
	Draw(dst, std::numeric_limits<Drawable::Z_t>::min(), std::numeric_limits<Drawable::Z_t>::max());
}

#endif
