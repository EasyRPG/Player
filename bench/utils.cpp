#include <benchmark/benchmark.h>
#include <font.h>
#include <rect.h>
#include <bitmap.h>
#include <text.h>
#include <pixel_format.h>
#include <cache.h>

static void BM_ReplacePlaceholders(benchmark::State& state) {
	for (auto _: state) {
		Utils::ReplacePlaceholders("One night is %V %U", {'V', 'U'}, {"Rest", "Do not Rest"});
	}
}

BENCHMARK(BM_ReplacePlaceholders);

BENCHMARK_MAIN();
