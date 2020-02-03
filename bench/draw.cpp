#include <benchmark/benchmark.h>
#include <font.h>
#include <rect.h>
#include <bitmap.h>
#include <sprite.h>
#include <graphics.h>
#include <drawable_list.h>
#include <drawable_mgr.h>
#include <iostream>

constexpr int num_sprites = 5000;

class TestSprite : public Drawable {
	public:
		TestSprite() : Drawable(0) { DrawableMgr::Register(this); }
		void Draw(Bitmap&) override {}
};

static void BM_DrawSort(benchmark::State& state) {
	DrawableList list;
	DrawableMgr::SetLocalList(&list);

	std::vector<std::unique_ptr<TestSprite>> sprites;
	for (int i = 0; i < num_sprites; ++i) {
		sprites.push_back(std::make_unique<TestSprite>());
	}

	for (auto _: state) {
		list.Sort();
	}
}

BENCHMARK(BM_DrawSort);

static void BM_DrawSortLocality(benchmark::State& state) {
	DrawableList list;
	DrawableMgr::SetLocalList(&list);

	std::array<TestSprite,num_sprites> sprites;

	for (auto _: state) {
		list.Sort();
	}
}

BENCHMARK(BM_DrawSortLocality);

BENCHMARK_MAIN();
