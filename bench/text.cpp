#include <benchmark/benchmark.h>
#include <font.h>
#include <rect.h>
#include <bitmap.h>
#include <text.h>
#include <pixel_format.h>
#include <cache.h>

const std::string text = "Alex $A landed a critical hit on Slime $B!";
char32_t symbol = '\\';
constexpr int width = 240;
constexpr int height = 80;

static void BM_TextDrawStrSystem(benchmark::State& state) {
	Bitmap::SetFormat(format_R8G8B8A8_a().format());
	auto font = Font::Default();
	auto surface = Bitmap::Create(width, height);
	auto system = Cache::SysBlack();

	for (auto _: state) {
		Text::Draw(*surface, 0, 0, *font, *system, 0, text, Text::AlignLeft);
	}
}

BENCHMARK(BM_TextDrawStrSystem);

static void BM_TextDrawStrColor(benchmark::State& state) {
	Bitmap::SetFormat(format_R8G8B8A8_a().format());
	auto font = Font::Default();
	auto surface = Bitmap::Create(width, height);

	for (auto _: state) {
		Text::Draw(*surface, 0, 0, *font, Color(255,255,255,255), text);
	}
}

BENCHMARK(BM_TextDrawStrColor);

void DrawCharSystemWrap(benchmark::State& state, char32_t ch, bool is_exfont) {
	Bitmap::SetFormat(format_R8G8B8A8_a().format());
	auto font = Font::Default();
	auto surface = Bitmap::Create(width, height);
	auto system = Cache::SysBlack();

	for (auto _: state) {
		Text::Draw(*surface, 0, 0, *font, *system, 0, ch, is_exfont);
	}
}

static void BM_TextDrawCharSystem(benchmark::State& state) {
	DrawCharSystemWrap(state, 'X', false);
}

BENCHMARK(BM_TextDrawCharSystem);

static void BM_TextDrawCharSystemEx(benchmark::State& state) {
	DrawCharSystemWrap(state, 0, true);
}

BENCHMARK(BM_TextDrawCharSystemEx);

void DrawCharColorWrap(benchmark::State& state, char32_t ch, bool is_exfont) {
	Bitmap::SetFormat(format_R8G8B8A8_a().format());
	auto font = Font::Default();
	auto surface = Bitmap::Create(width, height);

	for (auto _: state) {
		Text::Draw(*surface, 0, 0, *font, Color(255,255,255,255), ch, is_exfont);
	}
}

static void BM_TextDrawCharColor(benchmark::State& state) {
	DrawCharColorWrap(state, 'X', false);
}

BENCHMARK(BM_TextDrawCharColor);

static void BM_TextDrawCharColorEx(benchmark::State& state) {
	DrawCharColorWrap(state, 0, true);
}

BENCHMARK(BM_TextDrawCharColorEx);

BENCHMARK_MAIN();
