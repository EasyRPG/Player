#include <cmath>
#include <benchmark/benchmark.h>
#include <rect.h>
#include <bitmap.h>
#include <pixel_format.h>
#include <transform.h>

static void BlitTest(benchmark::State& state, DynamicFormat fmt) {
	Bitmap::SetFormat(fmt);
	auto dest = Bitmap::Create(320, 240);
	auto src = Bitmap::Create(320, 240);
	auto rect = src->GetRect();
	for (auto _: state) {
		dest->Blit(0, 0, *src, rect, Opacity::Opaque());
	}
}

static void BM_BlitBGRA_a(benchmark::State& state) {
	BlitTest(state, format_B8G8R8A8_a().format());
}

BENCHMARK(BM_BlitBGRA_a);

static void BM_BlitRGBA_a(benchmark::State& state) {
	BlitTest(state, format_R8G8B8A8_a().format());
}

BENCHMARK(BM_BlitRGBA_a);

static void BM_BlitABGR_a(benchmark::State& state) {
	BlitTest(state, format_A8B8G8R8_a().format());
}

BENCHMARK(BM_BlitABGR_a);

static void BM_BlitARGB_a(benchmark::State& state) {
	BlitTest(state, format_A8R8G8B8_a().format());
}

BENCHMARK(BM_BlitARGB_a);

static void BM_BlitBGRA_n(benchmark::State& state) {
	BlitTest(state, format_B8G8R8A8_n().format());
}

BENCHMARK(BM_BlitBGRA_n);

static void BM_BlitRGBA_n(benchmark::State& state) {
	BlitTest(state, format_R8G8B8A8_n().format());
}

BENCHMARK(BM_BlitRGBA_n);

static void BM_BlitABGR_n(benchmark::State& state) {
	BlitTest(state, format_A8B8G8R8_n().format());
}

BENCHMARK(BM_BlitABGR_n);

static void BM_BlitARGB_n(benchmark::State& state) {
	BlitTest(state, format_A8R8G8B8_n().format());
}

BENCHMARK(BM_BlitARGB_n);

BENCHMARK_MAIN();
