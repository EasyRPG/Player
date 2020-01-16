#include "pool_allocator_base.h"
#include "align.h"
#include <algorithm>
#include <cassert>

PoolAllocatorBase::Slot* PoolAllocatorBase::FirstSlot(Page* page, size_type obj_align) {
	auto off = reinterpret_cast<uintptr_t>(page);
	off += PageHeaderSize(obj_align);
	return reinterpret_cast<Slot*>(off);
}

PoolAllocatorBase::Slot* PoolAllocatorBase::NextSlot(Slot* s, size_type n, size_type obj_size, size_type obj_align) {
	auto off = reinterpret_cast<uintptr_t>(s);
	off += SlotSize(obj_size, obj_align) * n;
	return reinterpret_cast<Slot*>(off);
}

void PoolAllocatorBase::AllocNewPage(size_type obj_size, size_type obj_align) {
	assert(_nobjs_per_page > 0);
	const auto page_size = PageSize(_nobjs_per_page, obj_size, obj_align);
	const auto page_align = PageAlign(obj_align);
#ifdef __cpp_aligned_new
	auto* raw_page = operator new(page_size, std::align_val_t(page_align));
#else
	auto* raw_page = operator new(page_size);
	(void)page_align;
#endif
	auto* page = reinterpret_cast<Page*>(raw_page);
	page->next = nullptr;

	auto* slots = FirstSlot(page, obj_align);
	auto* last_slot = NextSlot(slots, _nobjs_per_page -1, obj_size, obj_align);

	last_slot->freenext = _freehead;
	auto* prev = last_slot;
	for(size_t i = _nobjs_per_page-1; i > 0;) {
		--i;
		auto* slot = NextSlot(slots, i, obj_size, obj_align);
		slot->freenext = prev;
		prev = slot;
	}

	_freehead = prev;
	page->next = _pagehead;
	_pagehead = page;
}

void PoolAllocatorBase::Reset(size_type obj_size, size_type obj_align) noexcept {
	assert(_nobjs_per_page > 0);
	const auto page_size = PageSize(_nobjs_per_page, obj_size, obj_align);
	const auto page_align = PageAlign(obj_align);

	for(auto* page = _pagehead; page != nullptr;) {
		auto* nextpage = page->next;
#ifdef __cpp_aligned_new
		operator delete(page, page_size, std::align_val_t(page_align));
#elif defined(__cpp_sized_deallocation)
		operator delete(page, page_size);
#else
		operator delete(page);
#endif
		page = nextpage;
	}
	_pagehead = nullptr;
	_freehead = nullptr;
}

