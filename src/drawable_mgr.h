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

#ifndef EP_DRAWABLE_MGR_H
#define EP_DRAWABLE_MGR_H

#include "drawable.h"
#include "drawable_list.h"
#include <cassert>

struct DrawableMgr {
	public:
		static DrawableList& GetLocalList();
		static DrawableList* GetLocalListPtr();

		static void SetLocalList(DrawableList* list);

		static void Register(Drawable* drawable);
		static void Remove(Drawable* drawable);
		static void OnUpdateZ(Drawable* drawable);
	private:
		static DrawableList* _local;
};


inline DrawableList& DrawableMgr::GetLocalList() {
	auto* local = GetLocalListPtr();
	assert(local != nullptr);
	return *local;
}

inline DrawableList* DrawableMgr::GetLocalListPtr() {
	return _local;
}

inline void DrawableMgr::OnUpdateZ(Drawable* /* drawable */) {
	GetLocalList().SetDirty();
}

#endif
