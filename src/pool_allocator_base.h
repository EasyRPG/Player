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

#ifndef EP_POOL_ALLOCATOR_BASE_H
#define EP_POOL_ALLOCATOR_BASE_H

#include <cstddef>
#include <utility>
#include <memory>
#include <algorithm>
#include "compiler.h"
#include "align.h"

/** This is an implementation base class for PoolAllocatorUntyped and PoolAllocator<T>
 * @note Not meant to be used directly, as the same size and alignment must be passed to every method.
 * @note Destructor does not cleanup! Implementing child class must do this!
 */
class PoolAllocatorBase {
	public:
		using size_type = size_t;

		static constexpr size_type SlotAlign(size_type obj_align);
		static constexpr size_type SlotSize(size_type obj_size, size_type obj_align);
		static constexpr size_type PageAlign(size_type obj_align);
		static constexpr size_type PageHeaderSize(size_type obj_align);
		static constexpr size_type PageSize(size_type n_objs_per_page, size_type obj_size, size_type obj_align);

		PoolAllocatorBase(const PoolAllocatorBase&) = delete;
		PoolAllocatorBase& operator=(const PoolAllocatorBase&) = delete;
		PoolAllocatorBase(PoolAllocatorBase&&) = delete;
		PoolAllocatorBase& operator=(PoolAllocatorBase&&) = delete;

	protected:
		constexpr PoolAllocatorBase(size_type nobjs_per_page);

		// Note: Doesn't cleanup!
		~PoolAllocatorBase() {}

		void* Alloc(size_type obj_size, size_type obj_align);

		void Free(void* p, size_type obj_size, size_type obj_align) noexcept;

		void Reset(size_type obj_size, size_type obj_align) noexcept;
	private:
		union Slot {
			Slot* freenext;
		};
		struct Page {
			Page* next;
		};
	private:
		static Slot* FirstSlot(Page* page, size_type obj_align);
		static Slot* NextSlot(Slot* s, size_type n, size_type obj_size, size_type obj_align);
		void AllocNewPage(size_type obj_size, size_type obj_align);
	private:
		Page* _pagehead = nullptr;
		Slot* _freehead = nullptr;
		size_type _nobjs_per_page = 0;
};


constexpr PoolAllocatorBase::PoolAllocatorBase(size_type nobjs_per_page)
	: _nobjs_per_page(nobjs_per_page) { }

	constexpr PoolAllocatorBase::size_type PoolAllocatorBase::SlotAlign(size_type obj_align) {
		return std::max(static_cast<size_type>(alignof(Slot)), obj_align);
	}

constexpr PoolAllocatorBase::size_type PoolAllocatorBase::SlotSize(size_type obj_size, size_type obj_align) {
	return AlignUp(std::max(static_cast<size_type>(sizeof(Slot)), obj_size), SlotAlign(obj_align));
}

constexpr PoolAllocatorBase::size_type PoolAllocatorBase::PageAlign(size_type obj_align) {
	return std::max(static_cast<size_type>(alignof(Page)), SlotAlign(obj_align));
}

constexpr PoolAllocatorBase::size_type PoolAllocatorBase::PageHeaderSize(size_type obj_align) {
	return AlignUp(sizeof(Page), SlotAlign(obj_align));
}

constexpr PoolAllocatorBase::size_type PoolAllocatorBase::PageSize(size_type n_objs_per_page, size_type obj_size, size_type obj_align) {
	return PageHeaderSize(obj_align) + n_objs_per_page * SlotSize(obj_size, obj_align);
}

inline void* PoolAllocatorBase::Alloc(size_type obj_size, size_type obj_align) {
	if(EP_UNLIKELY(_freehead == nullptr)) {
		AllocNewPage(obj_size, obj_align);
	}

	auto* slot = _freehead;
	_freehead = slot->freenext;
	return reinterpret_cast<void*>(slot);
}

inline void PoolAllocatorBase::Free(void* obj, size_type obj_size, size_type obj_align) noexcept {
	auto* slot = reinterpret_cast<Slot*>(obj);
	slot->freenext = _freehead;
	_freehead = slot;
}

#endif
