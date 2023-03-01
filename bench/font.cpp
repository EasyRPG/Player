#include <benchmark/benchmark.h>
#include <font.h>
#include <rect.h>
#include <bitmap.h>
#include <pixel_format.h>
#include <cache.h>

const std::string text = "Alex landed a critical hit on Slime!";
char32_t symbol = '\\';
constexpr int width = 240;
constexpr int height = 80;

static void BM_FontSizeStr(benchmark::State& state) {
	auto font = Font::Default();
	for (auto _: state) {
		auto rect = Text::GetSize(*font, text);
		(void)rect;
	}
}

BENCHMARK(BM_FontSizeStr);

static void BM_FontSizeChar(benchmark::State& state) {
	auto font = Font::Default();
	for (auto _: state) {
		auto rect = font->GetSize(symbol);
		(void)rect;
	}
}

BENCHMARK(BM_FontSizeChar);

static void BM_vRender(benchmark::State& state) {
	auto font = Font::Default();
	for (auto _: state) {
		auto bm = font->vRender(symbol);
		(void)bm;
	}
}

BENCHMARK(BM_vRender);

static void BM_Render(benchmark::State& state) {
	Bitmap::SetFormat(format_R8G8B8A8_a().format());
	auto surface = Bitmap::Create(width, height);
	auto system = Cache::SystemOrBlack();

	auto font = Font::Default();
	for (auto _: state) {
		font->Render(*surface, 0, 0, *system, 0, symbol);
	}
}

BENCHMARK(BM_Render);

BENCHMARK_MAIN();
