#include "pool_allocator.h"
#include <iostream>
#include <cstdio>
#include <array>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

constexpr auto ws = sizeof(void*);
constexpr auto wa = alignof(void*);

struct alignas(wa / 2) A1P2 {};
struct alignas(wa) A1P {};
struct alignas(2 * wa) A2P {};
struct alignas(4 * wa) A4P {};

static_assert(PoolAllocatorBase::SlotAlign(alignof(A1P2)) == wa, "SlotAlign is broken");
static_assert(PoolAllocatorBase::SlotAlign(alignof(A1P)) == wa, "SlotAlign is broken");
static_assert(PoolAllocatorBase::SlotAlign(alignof(A2P)) == 2 * wa, "SlotAlign is broken");
static_assert(PoolAllocatorBase::SlotAlign(alignof(A4P)) == 4 * wa, "SlotAlign is broken");

static_assert(PoolAllocatorBase::SlotSize(sizeof(A1P2), alignof(A1P2)) == std::max(ws, wa), "SlotSize is broken");
static_assert(PoolAllocatorBase::SlotSize(sizeof(A1P), alignof(A1P)) == std::max(ws, wa), "SlotSize is broken");
static_assert(PoolAllocatorBase::SlotSize(sizeof(A2P), alignof(A2P)) == std::max(ws, wa * 2), "SlotSize is broken");
static_assert(PoolAllocatorBase::SlotSize(sizeof(A4P), alignof(A4P)) == std::max(ws, wa * 4), "SlotSize is broken");

struct SE {};
struct SP { int* p[1]; };
struct S2P { int* p[2]; };
struct S4P { int* p[4]; };

static_assert(PoolAllocatorBase::SlotSize(sizeof(SE), alignof(SE)) == std::max(ws, wa), "SlotSize is broken");
static_assert(PoolAllocatorBase::SlotSize(sizeof(SP), alignof(SP)) == std::max(ws, wa), "SlotSize is broken");
static_assert(PoolAllocatorBase::SlotSize(sizeof(S2P), alignof(S2P)) == std::max(ws * 2, wa), "SlotSize is broken");
static_assert(PoolAllocatorBase::SlotSize(sizeof(S4P), alignof(S4P)) == std::max(ws * 4, wa), "SlotSize is broken");

struct alignas(2 * wa) S1A2P { int* p[2]; };
struct alignas(2 * wa) S2A2P { int* p[2]; };
struct alignas(4 * wa) S2A4P { int* p[2]; };
struct alignas(2 * wa) S4A2P { int* p[4]; };
struct alignas(4 * wa) S4A4P { int* p[4]; };

static_assert(PoolAllocatorBase::SlotSize(sizeof(S1A2P), alignof(S1A2P)) == std::max(ws, 2 * wa), "SlotSize is broken");
static_assert(PoolAllocatorBase::SlotSize(sizeof(S2A2P), alignof(S2A2P)) == std::max(2 * ws, 2 * wa), "SlotSize is broken");
static_assert(PoolAllocatorBase::SlotSize(sizeof(S2A4P), alignof(S2A4P)) == std::max(2 * ws, 4 * wa), "SlotSize is broken");
static_assert(PoolAllocatorBase::SlotSize(sizeof(S4A2P), alignof(S4A2P)) == std::max(4 * ws, 2 * wa), "SlotSize is broken");
static_assert(PoolAllocatorBase::SlotSize(sizeof(S4A2P), alignof(S4A2P)) == std::max(4 * ws, 4 * wa), "SlotSize is broken");

static_assert(PoolAllocatorBase::PageHeaderSize(alignof(A1P2)) == wa, "PageHeaderSize is broken");
static_assert(PoolAllocatorBase::PageHeaderSize(alignof(A1P)) == wa, "PageHeaderSize is broken");
static_assert(PoolAllocatorBase::PageHeaderSize(alignof(A2P)) == 2 * wa, "PageHeaderSize is broken");
static_assert(PoolAllocatorBase::PageHeaderSize(alignof(A4P)) == 4 * wa, "PageHeaderSize is broken");

TEST_CASE("PageSize") {
}

TEST_CASE("Untyped") {
	PoolAllocatorUntyped p(5, 8, 8);
}

template <size_t N, size_t A = 1>
struct alignas(A * alignof(int32_t)) SI : public std::array<int32_t,N> {
	SI(int32_t value) { this->fill(value); }
};

template <size_t N, size_t A>
bool operator==(SI<N,A> l, int32_t r) {
	return l == SI<N,A>(r);
}

template <size_t N, size_t A>
bool operator==(int32_t l, SI<N,A> r) {
	return SI<N,A>(l) == r;
}

template <size_t N, size_t A>
std::ostream& operator<<(std::ostream& os, const SI<N,A>& s) {
	os << "SI<" << N << ", " << A << ">{";
	for (auto& v: s) {
		os << v << ", ";
	}
	os << "}";
	return os;
}



template <typename T>
void testAllocator(int nobjs_per_page, int nobjs) {
	PoolAllocator<T> alloc(nobjs_per_page);

	constexpr auto size = sizeof(T);
	constexpr auto align = alignof(T);

	std::cout << "SIZE: " << size <<  " ALIGN: " << alignof(T) << std::endl;

	const char* tname = typeid(T).name();

	REQUIRE_EQ(alloc.ObjectSize(), size);
	REQUIRE_EQ(alloc.ObjectAlign(), align);

	srand(42);

	std::vector<PoolUniquePtr<T>> vec;
	for (int i = 0; i < nobjs; ++i) {
		vec.push_back(PoolUniquePtr<T>(nullptr, &alloc));

		bool add = (rand() % 2);
		if (add) {
			auto ptr = alloc.MakeUnique(i);
			auto* p = ptr.get();
			INFO("Alignment type=" << tname << " ptr=" << p << " align=" << align);
			REQUIRE(IsAligned(p, align));
			vec.back() = std::move(ptr);
		} else {
			int index = (rand() % vec.size());
			vec[index].reset();
		}

		for (int j = 0; j < i; ++j) {
			if (vec[j] != nullptr) {
				INFO("Mem Corruption type=" << tname << " npp=" << nobjs_per_page << " nobjs=" << nobjs << " j=" << j << " i=" << i);
				REQUIRE_EQ(*vec[j], j);
			}
		}
	}
}

TEST_CASE("Typed") {
	testAllocator<SI<1>>(5, 500);
	testAllocator<SI<1>>(12, 127);
	testAllocator<SI<1>>(17, 83);

	testAllocator<SI<2>>(5, 200);
	testAllocator<SI<2>>(12, 301);
	testAllocator<SI<2>>(17, 83);

	testAllocator<SI<4>>(5, 37);
	testAllocator<SI<4>>(12, 152);
	testAllocator<SI<4>>(17, 83);

	testAllocator<SI<1, 2>>(5, 111);
	testAllocator<SI<2, 2>>(5, 112);
	testAllocator<SI<4, 2>>(5, 143);

	testAllocator<SI<1, 4>>(5, 99);
	testAllocator<SI<2, 4>>(5, 85);
	testAllocator<SI<4, 4>>(5, 82);

	testAllocator<SI<8, 8>>(5, 82);
	testAllocator<SI<16, 16>>(5, 82);
}

