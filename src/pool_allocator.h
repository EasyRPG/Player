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

#ifndef EP_POOL_ALLOCATOR_H
#define EP_POOL_ALLOCATOR_H

#include <cstddef>
#include <utility>
#include <memory>
#include "compiler.h"
#include "pool_allocator_base.h"
#include "scope_guard.h"

/** A paged pool based allocator for untyped data. Initialized at runtime with the size and alignment
 * requirements of the underlying objects. */
class PoolAllocatorUntyped : public PoolAllocatorBase {
	public:
		using size_type = PoolAllocatorBase::size_type;

		/**
		 * Create the pool with given arguments
		 *
		 * @param nobjs_per_page the number of objects we'll allocate in each page.
		 * @param obj_size The size of each object.
		 * @param obj_align The alignment requirements of each object.
		 */
		constexpr PoolAllocatorUntyped(size_type nobjs_per_page, size_type obj_size, size_type obj_align = alignof(std::max_align_t));

		/** Destroys the pool.  */
		~PoolAllocatorUntyped();

		/** Allocate an object with ObjectSize() size and ObjectAlign() alignment */
		void* Alloc() noexcept;

		/** 
		 * Free an object allocated by this pool.
		 *
		 * @param p pointer allocated by this pool.
		 */
		void Free(void* p) noexcept;

		/** @return size of each object allocated by this pool */
		size_type ObjectSize() const;

		/** @return alignment of each object allocated by this pool */
		size_type ObjectAlign() const;
	private:
		void Reset() noexcept;
	private:
		size_type _obj_size = 0;
		size_type _obj_align = 0;
};

template <typename T> class PoolAllocator;

/** The Deleter class for PoolUniquePtr */
template <typename T> class PoolAllocatorDeleter {
	public:
		PoolAllocatorDeleter(PoolAllocator<T>* alloc);

		void operator()(T* p) const;
	private:
		PoolAllocator<T>* _alloc = nullptr;
};

/** Wrapper around unique_ptr for PoolAllocator<T> */
template <typename T>
using PoolUniquePtr = std::unique_ptr<T, PoolAllocatorDeleter<T>>;

template <typename T>
class PoolAllocator : public PoolAllocatorBase {
	public:
		using size_type = PoolAllocatorBase::size_type;
		using UniquePtr = PoolUniquePtr<T>;

		/**
		 * Construct with given number of objects per page
		 *
		 * @param nobjs_per_page the number of objects will be storable on each page.
		 *
		 * @note For optimal performance, we want nobjs_per_page to match our maximum
		 * number of expected objects, so they all get allocated on a single page
		 */
		explicit constexpr PoolAllocator(size_t nobjs_per_page);

		/**
		 * Destroy the allocator
		 *
		 * @pre If all allocated objects haven't been freed, the result is undefined.
		 */
		~PoolAllocator();

		/**
		 * Allocate a single object of type T.
		 *
		 * @return pointer to *uninitialized* memory big enough to store a T.
		 * @post This pointer be must freed by being passed to FreeUninitialized().
		 */
		T* AllocUninitialized();

		/**
		 * Free a single object of type T which was allocated by this.
		 *
		 * @return pointer to uninitialized memory allocated by this.
		 * @note The destructor of T is not called!
		 */
		void FreeUninitialized(T* p) noexcept;

		/**
		 * Destroy and then free a single object of type T which was allocated by this.
		 *
		 * @return pointer to initialized memory allocated by this.
		 * @note The destructor of T is called!
		 */
		void Destroy(T* p) noexcept;

		/**
		 * Allocates and constructs a T by forwarding args and returns it.
		 *
		 * @param args... parameters to be forwarded to T constructor.
		 * @return a PoolUniquePtr<T> which will automatically call Destroy() when it goes out of scope.
		 */
		template <typename... Args>
			PoolUniquePtr<T> MakeUnique(Args&&... args);

		/** @return size of each object allocated by this pool */
		static constexpr size_type ObjectSize();

		/** @return alignment of each object allocated by this pool */
		static constexpr size_type ObjectAlign();
	private:
		void Reset() noexcept;
};

constexpr PoolAllocatorUntyped::PoolAllocatorUntyped(size_type nobjs_per_page, size_type obj_size, size_type obj_align)
	: PoolAllocatorBase(nobjs_per_page), _obj_size(obj_size), _obj_align(obj_align) {}

	inline PoolAllocatorUntyped::~PoolAllocatorUntyped() {
		Reset();
	}

inline void* PoolAllocatorUntyped::Alloc() noexcept {
	return PoolAllocatorBase::Alloc(ObjectSize(), ObjectAlign());
}

inline void PoolAllocatorUntyped::Free(void* p) noexcept {
	return PoolAllocatorBase::Free(p, ObjectSize(), ObjectAlign());
}

inline void PoolAllocatorUntyped::Reset() noexcept {
	return PoolAllocatorBase::Reset(ObjectSize(), ObjectAlign());
}

inline PoolAllocatorUntyped::size_type PoolAllocatorUntyped::ObjectSize() const {
	return _obj_size;
}

inline PoolAllocatorUntyped::size_type PoolAllocatorUntyped::ObjectAlign() const {
	return _obj_align;
}

	template <typename T>
constexpr PoolAllocator<T>::PoolAllocator(size_t nobjs_per_page)
	: PoolAllocatorBase(nobjs_per_page) {}

	template <typename T>
	PoolAllocator<T>::~PoolAllocator() {
		Reset();
	}

template <typename T>
T* PoolAllocator<T>::AllocUninitialized() {
	return static_cast<T*>(PoolAllocatorBase::Alloc(ObjectSize(), ObjectAlign()));
}

template <typename T>
void PoolAllocator<T>::FreeUninitialized(T* obj) noexcept {
	PoolAllocatorBase::Free(obj, ObjectSize(), ObjectAlign());
}

template <typename T>
void PoolAllocator<T>::Destroy(T* obj) noexcept {
	obj->~T();
	FreeUninitialized(obj);
}

template <typename T>
void PoolAllocator<T>::Reset() noexcept {
	PoolAllocatorBase::Reset(ObjectSize(), ObjectAlign());
}

template <typename T>
constexpr typename PoolAllocator<T>::size_type PoolAllocator<T>::ObjectSize() {
	return sizeof(T);
}

template <typename T>
constexpr typename PoolAllocator<T>::size_type PoolAllocator<T>::ObjectAlign() {
	return alignof(T);
}

template <typename T>
template <typename... Args>
inline PoolUniquePtr<T> PoolAllocator<T>::MakeUnique(Args&&... args) {
	auto* p = this->AllocUninitialized();

	auto sg = makeScopeGuard([&]() { this->FreeUninitialized(p); });
	auto* t = new (p) T(std::forward<Args>(args)...);
	sg.Dismiss();

	return PoolUniquePtr<T>(t, PoolAllocatorDeleter<T>(this));
}


	template <typename T>
PoolAllocatorDeleter<T>::PoolAllocatorDeleter(PoolAllocator<T>* alloc)
	: _alloc(alloc) {}

	template <typename T>
	void PoolAllocatorDeleter<T>::operator()(T* p) const {
		_alloc->Destroy(p);
	}

#endif
