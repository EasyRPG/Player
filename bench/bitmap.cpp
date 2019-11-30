#include <benchmark/benchmark.h>
#include <rect.h>
#include <bitmap.h>
#include <pixel_format.h>

struct BitmapAccess : public Bitmap {
	static pixman_format_code_t find_format(const DynamicFormat& format) {
		return Bitmap::find_format(format);
	}
};

static void BM_FindFormatSingle(benchmark::State& state) {
	const auto fmt = format_R8G8B8A8_a().format();
	for (auto _: state) {
		BitmapAccess::find_format(fmt);
	}
}

BENCHMARK(BM_FindFormatSingle);

static void BM_FindFormat(benchmark::State& state) {
	for (auto _: state) {
		BitmapAccess::find_format(format_R8G8B8A8_a().format());
		BitmapAccess::find_format(format_B8G8R8A8_a().format());
		BitmapAccess::find_format(format_A8B8G8R8_n().format());
		BitmapAccess::find_format(format_B8G8R8A8_n().format());
	}
}

BENCHMARK(BM_FindFormat);

static void BM_Create(benchmark::State& state) {
	Bitmap::SetFormat(format_R8G8B8A8_a().format());
	for (auto _: state) {
		auto bm = Bitmap::Create(320, 240);
		(void)bm;
	}
}

BENCHMARK(BM_Create);

BENCHMARK_MAIN();


