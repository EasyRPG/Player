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
#include "drawable_list.h"
#include "drawable_mgr.h"
#include <algorithm>
#include <cassert>

static bool DrawCmp(Drawable* l, Drawable* r) {
	return l->GetZ() < r->GetZ();
}

DrawableList::~DrawableList() {
	if (DrawableMgr::GetLocalListPtr() == this) {
		DrawableMgr::SetLocalList(nullptr);
	}
}

void DrawableList::Clear() {
	_list.clear();
	SetClean();
}

bool DrawableList::IsSorted() const {
	return std::is_sorted(_list.begin(), _list.end(), DrawCmp);
}

void DrawableList::Sort() {
	// stable sort to work around a flickering event sprite issue when
	// the map is scrolling (have same Z value)
	std::stable_sort(_list.begin(), _list.end(), DrawCmp);
	SetClean();
}

void DrawableList::Append(Drawable* ptr) {
	assert(ptr != nullptr);
	assert(_list.end() == std::find(_list.begin(), _list.end(), ptr));

	const bool ordered = _list.empty() || !DrawCmp(ptr, _list.back());

	_list.push_back(ptr);

	if (!ordered) {
		SetDirty();
	}
}

Drawable* DrawableList::Take(Drawable* ptr) {
	auto iter = std::find(_list.begin(), _list.end(), ptr);
	if (iter == _list.end()) {
		return nullptr;
	}

	auto ret = *iter;
	// FIXME: Can we remove this O(N) operation here?
	_list.erase(iter);
	return ret;

	// Removing doesn't change sorted order, so not dirty flag.
}

void DrawableList::TakeFrom(DrawableList& other) noexcept {
	if (&other == this) { return; }

	auto& olist = other._list;

	if (olist.empty()) {
		return;
	}

	_list.insert(_list.end(), olist.begin(), olist.end());
	olist.clear();

	SetDirty();
	other.SetClean();
}

void DrawableList::Draw(Bitmap& dst, Drawable::Z_t min_z, Drawable::Z_t max_z) {
	if (IsDirty()) {
		Sort();
	} else {
		assert(IsSorted());
	}

	for (auto* drawable : _list) {
		auto z = drawable->GetZ();
		if (z < min_z) {
			continue;
		}
		if (z > max_z) {
			break;
		}
		if (drawable->IsVisible()) {
			drawable->Draw(dst);
		}
	}
}

