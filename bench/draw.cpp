#include <benchmark/benchmark.h>
#include <font.h>
#include <rect.h>
#include <bitmap.h>
#include <sprite.h>
#include <graphics.h>
#include <drawable_list.h>

constexpr int num_sprites = 5000;

class TestSprite : public Drawable {
	public:
		TestSprite() : Drawable(TypeDefault, 0, true) {}
		void Draw(Bitmap&) override {}
};

static void BM_DrawSort(benchmark::State& state) {
	std::vector<std::unique_ptr<TestSprite>> sprites;
	for (int i = 0; i < num_sprites; ++i) {
		sprites.push_back(std::make_unique<TestSprite>());
	}
	DrawableList list;
	for (auto& s: sprites) {
		list.Append(s.get());
	}

	for (auto _: state) {
		list.Sort();
	}
}

BENCHMARK(BM_DrawSort);

static void BM_DrawSortLocality(benchmark::State& state) {
	std::array<TestSprite,num_sprites> sprites;
	DrawableList list;
	for (auto& s: sprites) {
		list.Append(&s);
	}

	for (auto _: state) {
		list.Sort();
	}
}

BENCHMARK(BM_DrawSortLocality);

BENCHMARK_MAIN();
