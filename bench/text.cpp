#include <benchmark/benchmark.h>
#include <font.h>
#include <rect.h>
#include <bitmap.h>
#include <text.h>
#include <pixel_format.h>

const std::string text = "Alex landed a critical hit on Slime!";
char32_t symbol = '\\';
constexpr int width = 240;
constexpr int height = 80;

static void BM_TextDrawStr(benchmark::State& state) {
	Bitmap::SetFormat(format_R8G8B8A8_a().format());
	auto font = Font::Default();
	auto surface = Bitmap::Create(width, height);

	for (auto _: state) {
		Text::Draw(*surface, 0, 0, 0, font, text, Text::AlignLeft);
	}
}

BENCHMARK(BM_TextDrawStr);

static void BM_TextDrawStrRaw(benchmark::State& state) {
	Bitmap::SetFormat(format_R8G8B8A8_a().format());
	auto font = Font::Default();
	auto surface = Bitmap::Create(width, height);

	for (auto _: state) {
		Text::Draw(*surface, 0, 0, Color(255,255,255,255), font, text);
	}
}

BENCHMARK(BM_TextDrawStrRaw);



BENCHMARK_MAIN();
