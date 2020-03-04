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
#include "drawable_mgr.h"

DrawableList* DrawableMgr::_local = nullptr;

void DrawableMgr::SetLocalList(DrawableList* list) {
	if (list) {
		// If a Drawable that is was attached to list changed it's Z value when list
		// was not set as the current local list, the wrong list would have been set as dirty.
		// To prevent this
		// Always ensure local list gets sorted. When the list was used externally
		// we have no guarantee that changes to it's contents keep it sorted.
		list->SetDirty();
	}

	_local = list;
}

void DrawableMgr::Register(Drawable* drawable) {
	GetLocalList().Append(drawable);
}

void DrawableMgr::Remove(Drawable* drawable) {
	auto* list = GetLocalListPtr();
	// Global drawables can be singletons, which may get destroyed after all scenes due
	// static initialization order. Non-global drawables we assume are all gone before
	// all lists are destroyed.
	assert(list || drawable->IsGlobal());
	if (list) {
		list->Take(drawable);
	}
}

