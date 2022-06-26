#include <cassert>
#include <cstdlib>
#include "utils.h"
#include "drawable_list.h"
#include "drawable_mgr.h"
#include "bitmap.h"
#include "doctest.h"

TEST_SUITE_BEGIN("DrawableList");

namespace {

class TestSprite : public Drawable {
	public:
		TestSprite(Drawable::Z_t z = 0) : Drawable(z, Drawable::Flags::Global) {}
		void Draw(Bitmap&) override {}
};

class TestFrame : public Drawable {
	public:
		TestFrame(Drawable::Z_t z = 0) : Drawable(z, Drawable::Flags::Global | Drawable::Flags::Shared) {}
		void Draw(Bitmap&) override {}
};

}

TEST_CASE("Default") {
	DrawableList list;

	REQUIRE_EQ(list.IsDirty(), false);
	REQUIRE_EQ(list.begin(), list.end());
	REQUIRE_EQ(list.size(), 0L);
	REQUIRE(list.empty());
}

TEST_CASE("DirtyDraw") {
	Bitmap::SetFormat(format_R8G8B8A8_a().format());
	Bitmap bitmap(16, 16, false);

	DrawableList list;

	REQUIRE_EQ(list.IsDirty(), false);
	REQUIRE_EQ(list.IsSorted(), true);

	list.SetDirty();
	REQUIRE_EQ(list.IsDirty(), true);
	REQUIRE_EQ(list.IsSorted(), true);

	list.Draw(bitmap);
	REQUIRE_EQ(list.IsDirty(), false);
	REQUIRE_EQ(list.IsSorted(), true);

	list.Draw(bitmap);
	REQUIRE_EQ(list.IsDirty(), false);
	REQUIRE_EQ(list.IsSorted(), true);
}

template <typename F>
void testAppend(F&& f) {
	DrawableList default_list;
	DrawableMgr::SetLocalList(&default_list);

	DrawableList list;

	TestSprite s1(5);
	TestSprite s2(3);

	REQUIRE_EQ(list.size(), 0L);
	REQUIRE(list.empty());
	REQUIRE_EQ(list.begin(), list.end());
	REQUIRE_EQ(list.IsSorted(), true);
	REQUIRE_FALSE(list.IsDirty());

	list.Append(&s1);
	REQUIRE_EQ(list.size(), 1L);
	REQUIRE_FALSE(list.empty());
	REQUIRE_NE(list.begin(), list.end());
	REQUIRE_EQ(*list.begin(), &s1);
	REQUIRE_EQ(list.IsSorted(), true);
	REQUIRE_FALSE(list.IsDirty());

	list.Append(&s2);
	REQUIRE_EQ(list.size(), 2L);
	REQUIRE_FALSE(list.empty());
	REQUIRE_NE(list.begin(), list.end());
	REQUIRE_EQ(*list.begin(), &s1);
	REQUIRE_EQ(*(list.begin() + 1), &s2);
	REQUIRE_EQ(list.IsSorted(), false);
	REQUIRE(list.IsDirty());

	f(list);

	REQUIRE_EQ(list.size(), 2L);
	REQUIRE_FALSE(list.empty());
	REQUIRE_NE(list.begin(), list.end());
	REQUIRE_EQ(*list.begin(), &s2);
	REQUIRE_EQ(*(list.begin() + 1), &s1);
	REQUIRE_EQ(list.IsSorted(), true);
	REQUIRE_FALSE(list.IsDirty());
}

TEST_CASE("AppendSort") {
	testAppend([](auto& list) { list.Sort(); });
}

TEST_CASE("AppendDraw") {
	Bitmap::SetFormat(format_R8G8B8A8_a().format());
	Bitmap bitmap(16, 16, false);

	testAppend([&](auto& list) { list.Draw(bitmap); });
}

TEST_CASE("AppendUnSorted") {
	DrawableList default_list;
	DrawableMgr::SetLocalList(&default_list);

	DrawableList list;

	TestSprite s1(2);
	TestSprite s2(1);

	REQUIRE_EQ(list.IsSorted(), true);
	REQUIRE_FALSE(list.IsDirty());

	list.Append(&s1);
	REQUIRE_EQ(list.IsSorted(), true);
	REQUIRE_FALSE(list.IsDirty());

	list.Append(&s2);
	REQUIRE_EQ(list.IsSorted(), false);
	REQUIRE(list.IsDirty());

	list.Clear();
	REQUIRE(list.empty());
	REQUIRE_EQ(list.IsSorted(), true);
	REQUIRE_FALSE(list.IsDirty());
}

TEST_CASE("AppendSorted") {
	DrawableList default_list;
	DrawableMgr::SetLocalList(&default_list);

	DrawableList list;

	TestSprite s1(1);
	TestSprite s2(2);

	REQUIRE_EQ(list.IsSorted(), true);
	REQUIRE_FALSE(list.IsDirty());

	list.Append(&s1);
	REQUIRE_EQ(list.IsSorted(), true);
	REQUIRE_FALSE(list.IsDirty());

	list.Append(&s2);
	REQUIRE_EQ(list.IsSorted(), true);
	REQUIRE_FALSE(list.IsDirty());

	list.Clear();
	REQUIRE(list.empty());
	REQUIRE_EQ(list.IsSorted(), true);
	REQUIRE_FALSE(list.IsDirty());
}

TEST_CASE("AppendSame") {
	DrawableList default_list;
	DrawableMgr::SetLocalList(&default_list);

	DrawableList list;

	TestSprite s1;
	TestSprite s2;

	REQUIRE_EQ(list.IsSorted(), true);
	REQUIRE_FALSE(list.IsDirty());

	list.Append(&s1);
	REQUIRE_EQ(list.IsSorted(), true);
	REQUIRE_FALSE(list.IsDirty());

	list.Append(&s2);
	REQUIRE_EQ(list.IsSorted(), true);
	REQUIRE_FALSE(list.IsDirty());

	list.Clear();
	REQUIRE(list.empty());
	REQUIRE_EQ(list.IsSorted(), true);
	REQUIRE_FALSE(list.IsDirty());
}

TEST_CASE("TakeSorted") {
	DrawableList default_list;
	DrawableMgr::SetLocalList(&default_list);

	DrawableList list;

	TestSprite s1(1);
	TestSprite s2(2);
	TestSprite s3(3);

	list.Append(&s1);
	list.Append(&s2);

	REQUIRE_EQ(list.size(), 2L);
	REQUIRE_FALSE(list.IsDirty());
	REQUIRE(list.IsSorted());

	REQUIRE_EQ(list.Take(&s3), nullptr);

	REQUIRE_EQ(list.size(), 2L);
	REQUIRE_FALSE(list.IsDirty());
	REQUIRE(list.IsSorted());

	REQUIRE_EQ(list.Take(&s1), &s1);

	REQUIRE_EQ(list.size(), 1L);
	REQUIRE_FALSE(list.IsDirty());
	REQUIRE(list.IsSorted());

	REQUIRE_EQ(list.Take(&s2), &s2);

	REQUIRE_EQ(list.size(), 0L);
	REQUIRE_FALSE(list.IsDirty());
	REQUIRE(list.IsSorted());
}

TEST_CASE("TakeNotSorted") {
	DrawableList default_list;
	DrawableMgr::SetLocalList(&default_list);

	DrawableList list;

	TestSprite s1(3);
	TestSprite s2(2);
	TestSprite s3(1);

	list.Append(&s1);
	list.Append(&s2);

	REQUIRE_EQ(list.size(), 2L);
	REQUIRE(list.IsDirty());
	REQUIRE_FALSE(list.IsSorted());

	REQUIRE_EQ(list.Take(&s3), nullptr);

	REQUIRE_EQ(list.size(), 2L);
	REQUIRE(list.IsDirty());
	REQUIRE_FALSE(list.IsSorted());

	REQUIRE_EQ(list.Take(&s1), &s1);

	REQUIRE_EQ(list.size(), 1L);
	REQUIRE(list.IsDirty());
	REQUIRE(list.IsSorted());

	REQUIRE_EQ(list.Take(&s2), &s2);

	REQUIRE_EQ(list.size(), 0L);
	REQUIRE(list.IsDirty());
	REQUIRE(list.IsSorted());
}

TEST_CASE("TakeFromAll") {
	DrawableList default_list;
	DrawableMgr::SetLocalList(&default_list);

	TestSprite s1;
	TestSprite s2;
	TestFrame f1;
	TestFrame f2;

	DrawableList list1;
	DrawableList list2;

	list1.Append(&s1);
	list1.Append(&f1);

	list2.Append(&s2);
	list2.Append(&f2);

	REQUIRE_EQ(list1.size(), 2L);
	REQUIRE_EQ(list2.size(), 2L);
	REQUIRE_FALSE(list1.IsDirty());
	REQUIRE_FALSE(list2.IsDirty());

	list1.TakeFrom(list2);

	REQUIRE_EQ(list1.size(), 4L);
	REQUIRE_EQ(list2.size(), 0L);
	REQUIRE(list1.IsDirty());
	REQUIRE_FALSE(list2.IsDirty());

	list2.TakeFrom(list1);

	REQUIRE_EQ(list1.size(), 0L);
	REQUIRE_EQ(list2.size(), 4L);
	REQUIRE_FALSE(list1.IsDirty());
	REQUIRE(list2.IsDirty());
}

TEST_CASE("TakeFromPred") {
	DrawableList default_list;
	DrawableMgr::SetLocalList(&default_list);

	TestSprite s1;
	TestSprite s2;
	TestFrame f1;
	TestFrame f2;

	DrawableList list1;
	DrawableList list2;

	list1.Append(&s1);
	list1.Append(&f1);

	list2.Append(&s2);
	list2.Append(&f2);

	REQUIRE_EQ(list1.size(), 2L);
	REQUIRE_EQ(list2.size(), 2L);
	REQUIRE_FALSE(list1.IsDirty());
	REQUIRE_FALSE(list2.IsDirty());

	list1.TakeFrom(list2, [](auto* d) { return !d->IsShared(); });

	REQUIRE_EQ(list1.size(), 3L);
	REQUIRE_EQ(list2.size(), 1L);
	REQUIRE(list1.IsDirty());
	REQUIRE_FALSE(list2.IsDirty());

	list2.TakeFrom(list1, [](auto* d) { return !d->IsShared(); });

	REQUIRE_EQ(list1.size(), 1L);
	REQUIRE_EQ(list2.size(), 3L);
	REQUIRE(list1.IsDirty());
	REQUIRE(list2.IsDirty());

	list2.TakeFrom(list1, [](auto* d) { return !d->IsShared(); });

	REQUIRE_EQ(list1.size(), 1L);
	REQUIRE_EQ(list2.size(), 3L);
	REQUIRE(list1.IsDirty());
	REQUIRE(list2.IsDirty());

	list2.TakeFrom(list1, [](auto* d) { return d->IsShared(); });

	REQUIRE_EQ(list1.size(), 0L);
	REQUIRE_EQ(list2.size(), 4L);
	REQUIRE_FALSE(list1.IsDirty());
	REQUIRE(list2.IsDirty());
}

TEST_SUITE_END();
